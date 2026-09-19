#include "PG_DisaScriptExecutor.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <sstream>

#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <Disa/fileformats/PG_Script.h>
#include <Disa/fileformats/PG_TalkScript.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Script/PG_DisaActorAnimations.h>
#include <Script/PG_DisaContext.h>
#include <Disa/map/PG_DisaSpriteModel.h>

namespace {

/**
* @brief The name and argument layout of one op code, plus what running it does.
* @details The command function looks its own entry up again by op code, so every command that
* only has to announce itself shares one function instead of needing 80 of them.
*/
struct CommandInfo {
	const char* name = nullptr;
	/// Comma separated "name:width" per argument, a '-' in front of the width marks it signed.
	const char* arguments = "";
	/// Argument bytes, -1 when the parser works the size out of the script itself.
	int size = -1;
	PG_DisaScriptCommandPtr command = nullptr;
};

/// The if command carries its parsed condition and where to continue when it is false.
struct IfData {
	unsigned int jumpTarget = 0; /// Command index the lane continues at when the condition fails
	unsigned char count = 0; /// Number of compare operations
	/// Four operand triples of 7 byte with a combine byte between two of them.
	std::array<unsigned char, 32> conditions = {};
};

/// One menu entry, the branch code follows it inline until the next entry or the menu end.
struct MenuData {
	bool isStart = false;
	unsigned char entryCount = 0;
	unsigned int textIndex = 0; /// Offset of the zero terminated Shift-JIS text behind this struct
};

const std::array<CommandInfo, 256>& scriptCommands();
const std::array<CommandInfo, 256>& talkCommands();

//==== Reading raw arguments ====

unsigned int readUInt(const unsigned char* data, int size) {
	unsigned int out = 0;
	for (int i = 0; i < size; ++i) out |= static_cast<unsigned int>(data[i]) << (i * 8);
	return out;
}

int readInt(const unsigned char* data, int size) {
	const unsigned int raw = readUInt(data, size);
	if (size >= 4) return static_cast<int>(raw);
	const unsigned int signBit = 1u << (size * 8 - 1);
	return (raw & signBit) ? static_cast<int>(raw) - static_cast<int>(signBit << 1) : static_cast<int>(raw);
}

const CommandInfo& infoOf(const PG_DisaScriptLane& lane, unsigned char opCode) {
	return lane.type == PG_DisaScriptLane::Talk ? talkCommands()[opCode] : scriptCommands()[opCode];
}

const PG_DisaScriptCommand& currentCommandOf(const PG_DisaScriptLane& lane) {
	return lane.commands[lane.cursor];
}

//==== Logging ====

/// "[3|talk 2002023] dialog(...)", the lane is in front so parallel scripts stay readable.
void printLanePrefix(std::stringstream& stream, const PG_DisaScriptExecutor& executor, std::size_t laneIndex) {
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	stream << '[' << laneIndex << '|' << (lane.type == PG_DisaScriptLane::Talk ? "talk " : "script ")
		<< lane.scriptId.getName() << "] "; //getName, an identifier turns itself into a bool otherwise
}

/**
* @brief Writes the arguments the layout describes, anything the layout leaves over goes out as hex.
*/
void printArguments(std::stringstream& stream, const char* layout, const unsigned char* data, std::size_t size) {
	std::size_t offset = 0;
	bool first = true;
	for (const char* it = layout; it && *it;) {
		const char* colon = it;
		while (*colon && *colon != ':') ++colon;
		if (!*colon) break;

		const char* widthIt = colon + 1;
		const bool isSigned = *widthIt == '-';
		if (isSigned) ++widthIt;
		const int width = *widthIt - '0';
		if (width < 1 || width > 4 || offset + width > size) break;

		if (!first) stream << ", ";
		stream.write(it, colon - it);
		stream << '=';
		if (isSigned) stream << readInt(data + offset, width);
		else stream << readUInt(data + offset, width);

		first = false;
		offset += width;
		it = widthIt + 1;
		if (*it == ',') ++it;
	}

	for (; offset < size; ++offset) {
		if (!first) stream << ", ";
		stream << "0x" << std::hex << static_cast<int>(data[offset]) << std::dec;
		first = false;
	}
}

/// Announces the command the lane sits on, this is what every not yet implemented command does.
void printCommand(PG_DisaScriptExecutor& executor, std::size_t laneIndex, const void* data, const char* note = nullptr) {
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	const PG_DisaScriptCommand& command = currentCommandOf(lane);
	const CommandInfo& info = infoOf(lane, command.opCode);

	std::stringstream stream;
	printLanePrefix(stream, executor, laneIndex);
	stream << (info.name ? info.name : "unknown") << '(';
	printArguments(stream, info.arguments, static_cast<const unsigned char*>(data), command.dataSize);
	stream << ')';
	if (!info.name) stream << " //op code 0x" << std::hex << static_cast<int>(command.opCode) << std::dec;
	if (note) stream << " //" << note;
	OUT_DEBUG(stream.str());
}

//==== Commands both languages share ====

EDisaScriptResult cmdLog(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	return EDisaScriptResult::Done;
}

/// Logs and then does nothing, for the waits that hang on something the tool does not simulate.
EDisaScriptResult cmdLogSkippedWait(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data, "nothing to wait for yet, continues");
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdReturn(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	return EDisaScriptResult::Stop;
}

//==== SCRIPT.DAT ====

EDisaScriptResult cmdScriptError(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data, "invalid command, the lane stops");
	return EDisaScriptResult::Stop;
}

EDisaScriptResult cmdSleep(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize >= 2) {
		lane.memory.waitTicks = static_cast<float>(readUInt(static_cast<const unsigned char*>(data), 2));
	}
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdRepeatStart(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 4) return EDisaScriptResult::Done;

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	lane.memory.repeatCounter = std::max(0, readInt(bytes, 2));
	lane.memory.repeatMax = std::max(lane.memory.repeatCounter, readInt(bytes + 2, 2));
	lane.memory.repeatStart = lane.cursor + 1;
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdRepeatEnd(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (lane.memory.repeatCounter < lane.memory.repeatMax) {
		++lane.memory.repeatCounter;
		lane.cursor = lane.memory.repeatStart;
		return EDisaScriptResult::Jump;
	}
	lane.memory.repeatCounter = 0;
	lane.memory.repeatMax = 0;
	return EDisaScriptResult::Done;
}

/// Resolves one operand triple, a constant or a game state bit.
int readOperand(const PG_DisaScriptExecutor& executor, const unsigned char* operand) {
	const unsigned int value = readUInt(operand + 1, 2);
	if (operand[0] == 0) return readInt(operand + 1, 2);
	if (operand[0] != 1) return 0;
	/* Everything below 60000 is a game state bit, above it are queries into a world the tool has
	 * none of, so those read as zero. */
	if (value < 60000) return executor.context().getGameFlag(value) ? 1 : 0;
	return 0;
}

bool compare(int left, unsigned char operation, int right) {
	switch (operation) {
		case 0x0A: return left == right;
		case 0x0B: return left < right;
		case 0x0C: return left > right;
		case 0x0D: return left <= right;
		case 0x0E: return left >= right;
		case 0x0F: return left != right;
	}
	return false;
}

EDisaScriptResult cmdIf(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	IfData ifData; //the lane keeps its data as plain bytes, so it is copied out rather than pointed at
	memcpy(&ifData, data, sizeof(IfData));

	bool results[4] = { false, false, false, false };
	const unsigned char* condition = ifData.conditions.data();
	for (int i = 0; i < ifData.count && i < 4; ++i) {
		results[i] = compare(readOperand(executor, condition), condition[3], readOperand(executor, condition + 4));
		condition += 8; //the seventh byte ends the operation, the eighth combines it with the next
	}

	/* One condition decides alone, two are combined by the byte between them, and the game lets
	 * anything above two through no matter what it compared. */
	bool passed = true;
	if (ifData.count == 1) {
		passed = results[0];
	} else if (ifData.count == 2) {
		const unsigned char combine = ifData.conditions[7];
		passed = combine == 21 ? (!results[0] || results[1]) : (results[0] && results[1]);
	}

	std::stringstream note;
	note << (passed ? "true" : "false, jumps to command ") ;
	if (!passed) note << ifData.jumpTarget;
	printCommand(executor, laneIndex, data, note.str().c_str());

	if (passed) return EDisaScriptResult::Done;

	PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	lane.cursor = ifData.jumpTarget;
	return EDisaScriptResult::Jump;
}

EDisaScriptResult cmdSetFlag(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 7) return EDisaScriptResult::Done;

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	if (bytes[3] != 1) return EDisaScriptResult::Done; //only mode 1 writes, the game ignores the rest
	executor.context().setGameFlag(readUInt(bytes + 1, 2), readOperand(executor, bytes + 4) != 0);
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdRunScript(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 3) return EDisaScriptResult::Done;

	executor.runScriptById(PG_DisaIdentifier::disaIdentifierFromIds(readUInt(static_cast<const unsigned char*>(data), 3)), static_cast<int>(laneIndex));
	return EDisaScriptResult::Done;
}

/**
* @brief SetDungeon and LoadDungeonGeometry, both make a dungeon the one the scripts run in.
* @details The game looks the ID up in its dungeon table and keeps the one it has when nothing
* matches, which is how 0xFFFF ends up meaning "stay where you are".
*/
EDisaScriptResult cmdSetDungeon(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 2) {
		printCommand(executor, laneIndex, data);
		return EDisaScriptResult::Done;
	}

	const unsigned int dungeonId = readUInt(static_cast<const unsigned char*>(data), 2);
	if (dungeonId == 0xFFFF) {
		std::stringstream note;
		note << "no such dungeon, keeps current dungeon";
		printCommand(executor, laneIndex, data, note.str().c_str());
		return EDisaScriptResult::Done;
	}
	if (auto* context = executor.getParent<PG_DisaContext>()) {
		context->setDungeonId(dungeonId);
	}
	printCommand(executor, laneIndex, data);
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdLoadTalkScript(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 7) {
		printCommand(executor, laneIndex, data);
		return EDisaScriptResult::Done;
	}

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	unsigned int id = readUInt(bytes, 3);
	unsigned int dungeonId = readUInt(bytes + 3, 2);
	unsigned int flag = readUInt(bytes + 5, 2);
	//0xFFFF is not a dungeon, it stands for the one that is loaded right now
	const bool currentDungeon = dungeonId == 0xFFFF;
	if (currentDungeon) dungeonId = executor.context().getDungeonId();

	std::stringstream stream;
	stream << id << "::" << dungeonId << "::" << flag;
	const bool loaded = executor.callTalkScript(PG_DisaIdentifier(stream.str(), "disa"), static_cast<int>(laneIndex));

	std::stringstream note;
	if (currentDungeon) note << "dungeon 0xFFFF is the loaded one, " << dungeonId << ". ";
	note << (loaded ? "loaded" : "nothing loaded");
	printCommand(executor, laneIndex, data, note.str().c_str());

	//the game reports the result in a game state bit, so a following if can branch on it
	executor.context().setGameFlag(5, !loaded);
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdWaitForUser(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data, "no input yet, waits a moment instead");
	executor.getLane(laneIndex).memory.waitTicks = executor.getConfirmTicks();
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdWaitForFlag(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float deltaTicks, const void* data) {
	PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	const unsigned int flag = currentCommandOf(lane).dataSize >= 2 ? readUInt(static_cast<const unsigned char*>(data), 2) : 0;

	if (executor.context().getGameFlag(flag)) {
		printCommand(executor, laneIndex, data, "flag is set");
		lane.memory.waitedTicks = 0.f;
		return EDisaScriptResult::Done;
	}

	//nothing in the tool sets the flags the game would, so the wait gives up instead of parking the lane for good
	lane.memory.waitedTicks += deltaTicks;
	if (lane.memory.waitedTicks < executor.getWaitTimeoutTicks()) return EDisaScriptResult::Running;

	printCommand(executor, laneIndex, data, "flag never set, gives up waiting");
	lane.memory.waitedTicks = 0.f;
	return EDisaScriptResult::Done;
}

/// The text of a command is not zero terminated inside the lane, its length is the argument size.
std::string readText(const PG_DisaScriptLane& lane, const void* data) {
	const std::size_t size = currentCommandOf(lane).dataSize;
	if (!data || !size) return {};
	return shiftJisToString(std::string(static_cast<const char*>(data), size));
}

EDisaScriptResult cmdText(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	std::stringstream stream;
	printLanePrefix(stream, executor, laneIndex);
	stream << "Text(\"" << readText(executor.getLane(laneIndex), data) << "\")";
	OUT_DEBUG(stream.str());
	return EDisaScriptResult::Done;
}

//==== Actors ====

/**
* @brief Turns the facing a command carries into an angle, nothing when it says to keep the current one.
* @details The game adds the quarter turn to the world rotation the script runs under, and for the
* cutscene scripts that rotation sits one quarter turn back. The model counts its facing the way the
* map does, where zero looks east, so facing 1 is the one that lands on zero.
*/
std::optional<float> facingOf(int facing) {
	constexpr int keepFacing = 10;
	if (facing == keepFacing || facing < 0 || facing > 3) return std::nullopt;
	return static_cast<float>(facing) * 90.f;
}

/// Writes the facing of a command onto an actor, if it carries one.
void applyFacing(PG_DisaSpriteModel* actor, const std::optional<float>& facing) {
	if (facing) actor->setRotation(*facing);
}

/// The chain's view of the map's actors, which owns slots 64 to 95 and passes the rest through.
PG_DisaActorScriptViewModel* slotsOf(PG_DisaScriptExecutor& executor, std::size_t laneIndex) {
	return executor.getLane(laneIndex).slots.get();
}

/// Reads the slot of a command and hands back every actor it stands for.
std::vector<PG_DisaSpriteModel*> actorsOf(PG_DisaScriptExecutor& executor, std::size_t laneIndex, int slot) {
	auto out = executor.context().resolveSlots(slot, slotsOf(executor, laneIndex));
	if (out.empty()) {
		OUT_WARNING("Slot " << slot << " holds no actor, the command does nothing. "
			"A script slot fills up on its first SetActor or SetActorSprite.");
	}
	return out;
}

/**
* @brief The actor of a slot, made when the slot is still empty.
* @details Both SetActor and SetActorSprite run the same allocation in the game: a script slot
* that holds nothing yet gets an actor right there. A fresh one stands at the map origin, which
* is inside the floor, so it is dropped onto the ground before anything sees it.
*/
PG_DisaSpriteModel* spawnIntoSlot(PG_DisaScriptExecutor& executor, std::size_t laneIndex, int slot) {
	PG_DisaContext& context = executor.context();
	PG_DisaActorScriptViewModel* scriptSlots = slotsOf(executor, laneIndex);

	const bool existed = context.resolveSlot(slot, scriptSlots) != nullptr;
	PG_DisaSpriteModel* actor = context.takeSlot(slot, scriptSlots);
	if (!actor) {
		OUT_WARNING("Slot " << slot << " can hold no actor, nothing is spawned!");
		return nullptr;
	}

	if (!existed) context.snapToGround(actor);
	return actor;
}

EDisaScriptResult cmdSetActor(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 6) return EDisaScriptResult::Done;

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	const int slot = bytes[0];
	const int characterId = readInt(bytes + 1, 2);
	const int level = static_cast<int>(readUInt(bytes + 3, 2));
	const int allegiance = bytes[5];

	PG_DisaSpriteModel* actor = spawnIntoSlot(executor, laneIndex, slot);
	if (!actor) return EDisaScriptResult::Done;

	/* 10000 and 20000 on top of the ID pull a real party or story character in, the sprite is the
	 * same one either way and the stats behind it are not simulated. */
	actor->setCharacterId(static_cast<unsigned short>(std::abs(characterId) % 10000));
	actor->setCharacterLevel(static_cast<unsigned short>(level));
	actor->setCharacterAlliance(static_cast<unsigned char>(allegiance));
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdSetActorSprite(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 4) return EDisaScriptResult::Done;

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	const int characterId = readInt(bytes + 2, 2);

	//10000 and up is not a sprite, it names another slot to take the look of
	constexpr int copyFromSlot = 10000;
	unsigned short sprite = static_cast<unsigned short>(std::abs(characterId) % copyFromSlot);
	if (std::abs(characterId) >= copyFromSlot) {
		const int sourceSlot = std::abs(characterId) - copyFromSlot;
		auto* source = executor.context().resolveSlot(sourceSlot, slotsOf(executor, laneIndex));
		if (!source) {
			OUT_WARNING("Slot " << sourceSlot << " holds no actor to take the look of!");
			return EDisaScriptResult::Done;
		}
		sprite = source->getCharacterId();
	}

	const int slot = static_cast<int>(readUInt(bytes, 2));
	/* The game allocates the actor here too, so a script that only swaps the look of an empty
	 * slot still ends up with one standing there. Sprite 0 is the one that clears a slot rather
	 * than filling it. */
	if (sprite > 0 && PG_DisaActorListModel::isItemSlot(slot)) {
		if (auto* actor = spawnIntoSlot(executor, laneIndex, slot)) actor->setCharacterId(sprite);
		return EDisaScriptResult::Done;
	}

	for (auto* actor : actorsOf(executor, laneIndex, slot)) {
		actor->setCharacterId(sprite);
	}
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdSetActorPos(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 13) return EDisaScriptResult::Done;

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	const int slot = static_cast<int>(readUInt(bytes, 2));
	const int parent = static_cast<int>(readUInt(bytes + 2, 2));
	const glm::vec3 offset(readInt(bytes + 4, 2), readInt(bytes + 6, 2), readInt(bytes + 8, 2));
	const float duration = static_cast<float>(readUInt(bytes + 10, 2));
	const auto mode = static_cast<PG_DisaActorAnimations::EMoveMode>(bytes[12]);

	PG_DisaContext& context = executor.context();
	const glm::vec3 target = context.getSlotOrigin(parent, slotsOf(executor, laneIndex)) + PG_DisaContext::toTileUnits(offset);
	for (auto* actor : actorsOf(executor, laneIndex, slot)) {
		PG_DisaActorAnimations::move(context, actor, target, duration, mode);
	}
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdSetActorPose(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 4) return EDisaScriptResult::Done;

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	//a pose is a sheet number spelled out, held until something else changes it, and never busy
	const int pose = readInt(bytes + 2, 2);
	for (auto* actor : actorsOf(executor, laneIndex, static_cast<int>(readUInt(bytes, 2)))) {
		PG_DisaActorAnimations::pose(actor, pose);
	}
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdSetActorAnimation(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 5) return EDisaScriptResult::Done;

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	const int animationSet = readInt(bytes + 2, 2);
	const std::optional<float> facing = facingOf(bytes[4]);

	PG_DisaContext& context = executor.context();
	for (auto* actor : actorsOf(executor, laneIndex, static_cast<int>(readUInt(bytes, 2)))) {
		applyFacing(actor, facing);
		//the game puts the actor into combat numbering here, the SetAnimationModeSet after it decides
		actor->setMode(PG_DisaSpriteModel::Mode_Combat);
		PG_DisaActorAnimations::play(context, actor, animationSet, 1);
	}
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdSetActorPoseDir(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 5) return EDisaScriptResult::Done;

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	/* The game reads the side view off the camera once and bakes it into the pose, so the actor
	 * keeps that sheet however it turns afterwards. Only the side front view is taken here. */
	const int pose = static_cast<int>(readUInt(bytes + 2, 2)) * 10 + 1;
	const std::optional<float> facing = facingOf(bytes[4]);

	for (auto* actor : actorsOf(executor, laneIndex, static_cast<int>(readUInt(bytes, 2)))) {
		applyFacing(actor, facing);
		PG_DisaActorAnimations::pose(actor, pose);
	}
	return EDisaScriptResult::Done;
}

/**
* @brief Picks which numbering and how many side views the actor's animations come from.
* @details Mode 0 leaves the sheet to the pose the script named, 1 is the battle numbering with two
* side views, 2 the cutscene one with five, which is why EMode counts the same way. Every cutscene
* actor is put into 2 right after each SetActorAnimation, which sets it back to 1 on its own.
*/
EDisaScriptResult cmdSetAnimationModeSet(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 3) return EDisaScriptResult::Done;

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	const int mode = bytes[2];
	if (mode > PG_DisaSpriteModel::Mode_Cutscene) return EDisaScriptResult::Done;

	for (auto* actor : actorsOf(executor, laneIndex, static_cast<int>(readUInt(bytes, 2)))) {
		actor->setMode(static_cast<PG_DisaSpriteModel::EMode>(mode));
	}
	return EDisaScriptResult::Done;
}

/**
* @brief Turns an actor, in one of three ways the duration alone decides between.
* @details A duration of zero sets the facing outright, one below 10000 adds the angle over that
* many ticks, and 10000 and up is not a duration at all: it says the angle is one to turn to and
* carries the length on top of it.
*/
EDisaScriptResult cmdSetActorRotation(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 5) {
		printCommand(executor, laneIndex, data);
		return EDisaScriptResult::Done;
	}

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	const float duration = static_cast<float>(readUInt(bytes + 1, 2));
	const float degree = static_cast<float>(readInt(bytes + 3, 2));
	constexpr float turnToAngle = 10000.f;

	const bool byAngle = duration > 0.f && duration < turnToAngle;
	const float ticks = duration >= turnToAngle ? duration - turnToAngle : duration;
	printCommand(executor, laneIndex, data, byAngle ? "rotate by" : ticks > 0.f ? "rotate to" : "set");

	PG_DisaContext& context = executor.context();
	for (auto* actor : actorsOf(executor, laneIndex, bytes[0])) {
		if (byAngle) PG_DisaActorAnimations::rotateBy(context, actor, degree, ticks);
		else PG_DisaActorAnimations::rotateTo(context, actor, degree, ticks);
	}
	return EDisaScriptResult::Done;
}

/**
* @brief Tints the sprite of an actor, over a duration.
* @details Every channel is a percent where 100 leaves it as it is. The sprite carries no alpha of
* its own yet, so an actor faded out to nothing is simply not drawn.
*/
EDisaScriptResult cmdSetMultRGBA(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 12) return EDisaScriptResult::Done;

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	constexpr float unchanged = 100.f;
	const glm::vec4 color(readInt(bytes + 2, 2) / unchanged, readInt(bytes + 4, 2) / unchanged,
		readInt(bytes + 6, 2) / unchanged, readInt(bytes + 8, 2) / unchanged);
	const float duration = static_cast<float>(readUInt(bytes + 10, 2));

	PG_DisaContext& context = executor.context();
	for (auto* actor : actorsOf(executor, laneIndex, static_cast<int>(readUInt(bytes, 2)))) {
		PG_DisaActorAnimations::tint(context, actor, color, duration);
	}
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdSetActorGravity(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 4) return EDisaScriptResult::Done;

	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	const int percent = readInt(bytes + 2, 2);

	for (auto* actor : actorsOf(executor, laneIndex, static_cast<int>(readUInt(bytes, 2)))) {
		//a negative value is not a gravity, it asks for the engine's own
		actor->setGravity(percent < 0 ? PG_DisaSpriteModel::defaultGravity
			: static_cast<float>(percent) / 100.f / PG_DisaContext::unitsPerTile);
	}
	return EDisaScriptResult::Done;
}

/**
* @brief Lets an animation that stopped at a marker run on, and waits until it gets there.
* @details Not every animation loops. Some park at a marker and wait to be let go, which is how a
* script times a stand up or a draw against its dialog. The game re-runs this command until the
* animation has parked, lets it go and then moves straight on, it does not wait for the rest of
* the animation to play out.
*/
EDisaScriptResult cmdContinueActorAnimation(PG_DisaScriptExecutor& executor, std::size_t laneIndex,
	float, const void* data) {
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 2) {
		printCommand(executor, laneIndex, data);
		return EDisaScriptResult::Done;
	}
	static bool visted = false; //@todo needs to be not a static!!! But attched to the command
	const int slot = static_cast<int>(readUInt(static_cast<const unsigned char*>(data), 2));
	auto actors = actorsOf(executor, laneIndex, slot);

	if (!visted) {
		//should only trigger one time
		for (auto* actor : actors) {
			actor->nextAnimationSegment();
		}
		visted = true;
	}

	PG_DisaContext& context = executor.context();

	for (auto* actor : actors) {
		if (context.hasAnimation(actor)) return EDisaScriptResult::Running;
	}

	visted = false;
	printCommand(executor, laneIndex, data, "nothing left to wait for");
	return EDisaScriptResult::Done;
}

//==== TALK.DAT ====

EDisaScriptResult cmdDialog(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	std::stringstream stream;
	printLanePrefix(stream, executor, laneIndex);
	stream << "dialog(\"" << readText(executor.getLane(laneIndex), data) << "\")";
	OUT_DEBUG(stream.str());
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdConfirm(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data, "no input yet, waits a moment instead");
	executor.getLane(laneIndex).memory.waitTicks = executor.getConfirmTicks();
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdConfirmAuto(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize >= 2) {
		lane.memory.waitTicks = static_cast<float>(readUInt(static_cast<const unsigned char*>(data), 2));
	}
	return EDisaScriptResult::Done;
}

/// talk `set` and `clear`, the flag is a bit index in the low three bits and a byte index above it.
EDisaScriptResult cmdTalkFlag(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	const PG_DisaScriptCommand& command = currentCommandOf(lane);
	if (command.dataSize < 2) return EDisaScriptResult::Done;

	/* The argument splits into a byte index and a bit inside it, which put back together is the
	 * plain bit index the script SetFlag and an If already use. */
	executor.context().setGameFlag(readUInt(static_cast<const unsigned char*>(data), 2), command.opCode == 0x09);
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdTalkScript(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 3) {
		printCommand(executor, laneIndex, data);
		return EDisaScriptResult::Done;
	}

	const unsigned int scriptId = readUInt(static_cast<const unsigned char*>(data), 3);
	/* Zero starts nothing, it is only there to make the talk script stop until the scripts it
	 * started are done. It needs no handling of its own: the lane is held back for as long as a
	 * script runs either way, so reaching this command at all means there is nothing left to
	 * wait for. */
	if (scriptId == 0) {
		printCommand(executor, laneIndex, data, "every started script finished");
		return EDisaScriptResult::Done;
	}

	printCommand(executor, laneIndex, data);
	executor.runScriptById(PG_DisaIdentifier::disaIdentifierFromIds(scriptId), static_cast<int>(laneIndex));
	return EDisaScriptResult::Done;
}

EDisaScriptResult cmdTalkCall(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	const PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (currentCommandOf(lane).dataSize < 3) return EDisaScriptResult::Done;

	const unsigned int scriptId = readUInt(static_cast<const unsigned char*>(data), 3);
	if (!executor.callTalkScript(laneIndex, scriptId)) return EDisaScriptResult::Stop;
	return EDisaScriptResult::Jump;
}

/// The entry text sits right behind the struct, both live in the lane's plain byte data.
MenuData readMenuData(const void* data) {
	MenuData out;
	memcpy(&out, data, sizeof(MenuData));
	return out;
}

EDisaScriptResult cmdMenu(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	const MenuData menu = readMenuData(data);
	const char* text = static_cast<const char*>(data) + menu.textIndex;

	PG_DisaScriptLane& lane = executor.getLane(laneIndex);
	if (!menu.isStart) {
		/* Reached by falling out of the branch above, so the chosen entry is done. The game
		 * leaves the menu at its end marker, sub menus need their own talk script so a plain
		 * search forward finds the right one. */
		for (std::size_t i = lane.cursor + 1; i < lane.commands.size(); ++i) {
			if (lane.commands[i].opCode != 0xFD) continue;
			printCommand(executor, laneIndex, data, "end of the chosen branch");
			lane.cursor = i + 1;
			return EDisaScriptResult::Jump;
		}
		printCommand(executor, laneIndex, data, "end of the chosen branch, no menuEnd follows");
		return EDisaScriptResult::Stop;
	}

	std::stringstream stream;
	printLanePrefix(stream, executor, laneIndex);
	stream << "menuStart(" << static_cast<int>(menu.entryCount) << ") //no menu yet, takes entry 0";
	OUT_DEBUG(stream.str());

	int entry = 0;
	std::stringstream entries;
	printLanePrefix(entries, executor, laneIndex);
	entries << "  [" << entry << "] \"" << shiftJisToString(text) << '"';
	OUT_DEBUG(entries.str());

	for (std::size_t i = lane.cursor + 1; i < lane.commands.size() && entry + 1 < menu.entryCount; ++i) {
		if (lane.commands[i].opCode == 0xFD) break;
		if (lane.commands[i].opCode != 0x14) continue;

		const void* entryData = lane.getData(lane.commands[i]);
		const MenuData other = readMenuData(entryData);
		std::stringstream line;
		printLanePrefix(line, executor, laneIndex);
		line << "  [" << ++entry << "] \"" << shiftJisToString(static_cast<const char*>(entryData) + other.textIndex) << '"';
		OUT_DEBUG(line.str());
	}

	return EDisaScriptResult::Done; //entry 0's branch follows right here
}

EDisaScriptResult cmdRestart(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float, const void* data) {
	printCommand(executor, laneIndex, data);
	/* Not a jump back to the top: the game runs the original search again, so a flag a menu just
	 * set can bring up a different script than the one that is running. */
	if (!executor.restartTalkScript(laneIndex)) return EDisaScriptResult::Stop;
	return EDisaScriptResult::Jump;
}

//==== Command tables ====

const std::array<CommandInfo, 256>& scriptCommands() {
	static const std::array<CommandInfo, 256> commands = [] {
		std::array<CommandInfo, 256> out = {};
		auto set = [&out](unsigned char opCode, const char* name, const char* arguments = "",
			PG_DisaScriptCommandPtr command = &cmdLog, int size = -1) {
			out[opCode] = CommandInfo{ name, arguments, size, command };
		};

		set(0x00, "Error", "", &cmdScriptError, 0); //the game consumes no body, it parks on this command
		set(0x01, "Sleep", "ticks:2", &cmdSleep);
		set(0x02, "Return", "", &cmdReturn);
		set(0x03, "RepeatStart", "start:-2,end:-2", &cmdRepeatStart);
		set(0x04, "RepeatEnd", "", &cmdRepeatEnd);
		set(0x05, "LockUser");
		set(0x06, "UnlockUser");
		set(0x07, "If", "", &cmdIf);
		set(0x08, "SetFlag", "flagType:1,flag:2,mode:1,valueType:1,value:2", &cmdSetFlag);
		set(0x09, "Nop");
		set(0x0A, "SetFade", "opacity:-1,duration:2");
		set(0x0B, "SleepSkippable", "ticks:2", &cmdSleep);
		set(0x0C, "ShakeScreen", "unused:1,strength:1,duration:2");
		set(0x0E, "SetFadeColor", "r:1,g:1,b:1");
		set(0x11, "LoadTalkScript", "scriptId:3,dungeonId:2,flag:2", &cmdLoadTalkScript);
		set(0x12, "RunScript", "scriptId:3", &cmdRunScript);
		set(0x15, "SetDungeon", "dungeonId:2", &cmdSetDungeon);
		set(0x16, "EnterDungeon", "mode:-2");
		set(0x17, "LoadDungeonGeometry", "dungeonId:2", &cmdSetDungeon);
		set(0x18, "WaitForMapLoad", "", &cmdLogSkippedWait);
		set(0x1B, "SetScreenColor", "duration:2");
		set(0x1C, "SetActorRawPos", "slot:2,x:-2,y:-2,depth:3");
		set(0x23, "ClearActorData", "slot:2");
		set(0x24, "SetRenderGroup", "group:2,duration:2,r:1,g:1,b:1,a:1,rotX:-2,rotY:-2,rotZ:-2");
		set(0x27, "SetCursorPos", "parent:2,x:-2,z:-2,duration:2");
		set(0x28, "SetCameraMode", "mode:1");
		set(0x29, "SetCameraZoom", "duration:2,distance:-2");
		set(0x2A, "SpinCamera", "duration:2,rotX:-2,rotY:-2,rotZ:-2");
		set(0x2B, "SetCameraView", "duration:2,value:2");
		set(0x2C, "SetCameraPos", "parent:2,x:-2,y:-2,z:-2,duration:2");
		set(0x2D, "SetActorGravity", "slot:2,gravity:-2", &cmdSetActorGravity);
		set(0x2E, "SetCameraRotation", "duration:2,rotX:-2,rotY:-2,rotZ:-2");
		set(0x32, "Text", "", &cmdText);
		set(0x33, "ClearText");
		set(0x34, "WaitForUser", "", &cmdWaitForUser);
		set(0x37, "ShowText", "mode:1,startY:1");
		set(0x38, "ResetTextScroll");
		set(0x39, "LoadPortrait", "slot:1,imageId:2");
		set(0x3A, "ShowPortrait", "slot:1", &cmdLogSkippedWait);
		set(0x46, "SetUnitField", "characterId:2,mode:1,value:-2");
		set(0x4E, "SpawnPartyMember", "slot:1,characterId:2");
		set(0x4F, "AddPartyMember", "characterId:-2,level:2");
		set(0x50, "AddMoney", "amount:-4");
		set(0x51, "GiveItem", "itemId:2,level:-2");
		set(0x53, "RequestTalk", "index:2");
		set(0x54, "WaitForFlag", "flag:2", &cmdWaitForFlag);
		set(0x55, "WaitForPortrait", "", &cmdLogSkippedWait);
		set(0x5A, "LoadData", "loader:1,entry:1");
		set(0x5C, "SetTextColor", "r:2,g:2,b:2,a:2,duration:2");
		set(0x61, "RunScriptOnActor", "scriptId:3,slot:2", &cmdRunScript);
		set(0x64, "MoveActor", "slot:2,parent:2,x:-2,y:-2,z:-2,duration:2,mode:1", &cmdSetActorPos);
		set(0x65, "SetActorPos", "slot:2,parent:2,x:-2,y:-2,z:-2,duration:2,mode:1", &cmdSetActorPos);
		set(0x66, "SetActorPose", "slot:2,pose:-2", &cmdSetActorPose);
		set(0x67, "SetActorAnimation", "slot:2,animation:2,facing:1", &cmdSetActorAnimation);
		set(0x68, "ContinueActorAnimation", "slot:2", &cmdContinueActorAnimation);
		set(0x69, "SetActorGroup", "slot:2,group:1");
		set(0x6A, "SetActorPoseDir", "slot:2,poseBase:2,facing:1", &cmdSetActorPoseDir);
		set(0x6B, "SetAnimationModeSet", "slot:2,mode:1", &cmdSetAnimationModeSet);
		set(0x6D, "SetActor", "slot:1,characterId:-2,level:2,allegiance:1", &cmdSetActor);
		set(0x6E, "SetActorSprite", "slot:2,characterId:-2", &cmdSetActorSprite);
		set(0x6F, "PlayEffect", "effectId:2,parent:2,x:-2,y:-2,z:-2,scale:2,speed:2,group:2");
		set(0x70, "SetMapTile", "parent:1,x:-2,z:-2,type:1,value:-2");
		set(0x71, "PlayAimedEffect", "effectId:2,source:2,x:-2,y:-2,z:-2,scale:2,speed:2,group:2");
		set(0x72, "PlayEffectEx", "effectId:2,parent:2,x:-2,y:-2,z:-2,scale:2,speed:2,group:2,extraA:2,extraB:2");
		set(0x73, "setActorRotation", "slot:1,duration:2,rotation:-2", &cmdSetActorRotation);
		set(0x74, "PlayAimedEffectEx", "effectId:2,source:2,x:-2,y:-2,z:-2,scale:2,speed:2,group:1,extraA:2,extraB:2");
		set(0x77, "OrbitActor", "slot:2,duration:2,center:2,x:-2,z:-2,y:-2,angle:-2");
		set(0x78, "DoAction", "slot:1,action:1");
		set(0x79, "SetActorFlag", "slot:2,value:1");
		set(0x7A, "SetMultRGBA", "slot:2,r:2,g:2,b:2,a:2,duration:2", &cmdSetMultRGBA);
		set(0x7B, "SetActorPalette", "slot:2,palette:1,variant:1");
		set(0x82, "LoadStream", "streamId:2");
		set(0x83, "PlayStream", "streamId:2,volume:1");
		set(0x87, "PlayVoice", "voiceId:2,channel:1");
		set(0x88, "StopVoice", "characterId:2,value:1");
		set(0x8C, "PlaySound", "soundId:2,volume:1,frequency:2");
		set(0x8D, "FadeSoundVolume", "target:1,duration:2");
		set(0x91, "PlayBGM", "bgmId:1,volume:1");
		set(0x92, "FadeBGM", "volume:1,duration:2");
		set(0x93, "PlayBGMDirect", "bgmId:2");
		set(0x94, "PlayBGMWait", "bgmId:2,volume:2");
		set(0x95, "PlayBGMAt", "bgmId:2,offsetMs:3");
		set(0x98, "SetUnitScript", "unit:2,scriptId:3");
		set(0xC8, "RunFunction", "function:2,value:3");
		set(0xC9, "SetSceneMode", "mode:2");
		set(0xCA, "SetActorFlag2", "slot:2,value:1");
		set(0xCB, "SetActorDepth", "slot:2,depth:-4");
		set(0xCC, "PlayEffectId", "handle:2,effectId:2,parent:2,x:-2,y:-2,z:-2,scale:2,speed:2,group:2");
		set(0xCD, "PlayAimedEffectId", "handle:2,effectId:2,source:2,x:-2,y:-2,z:-2,scale:2,speed:2,group:2");
		set(0xCE, "PlayEffectExId", "handle:2,effectId:2,parent:2,x:-2,y:-2,z:-2,scale:2,speed:2,group:2,extraA:2,extraB:2");
		set(0xCF, "PlayAimedEffectExId", "handle:2,effectId:2,source:2,x:-2,y:-2,z:-2,scale:2,speed:2,group:1,extraA:2,extraB:2");
		/* The stored body size of SetEffectParam says 6, the game reads 3. Trusting the size byte
		 * loses the command stream right there. */
		set(0xD0, "SetEffectParam", "handle:2,value:1", &cmdLog, 3);
		set(0xD1, "SetEffectValue", "handle:2,value:-4");
		return out;
	}();
	return commands;
}

const std::array<CommandInfo, 256>& talkCommands() {
	static const std::array<CommandInfo, 256> commands = [] {
		std::array<CommandInfo, 256> out = {};
		auto set = [&out](unsigned char opCode, const char* name, const char* arguments, int size,
			PG_DisaScriptCommandPtr command = &cmdLog) {
			out[opCode] = CommandInfo{ name, arguments, size, command };
		};

		set(0x01, "dialog", "", -1, &cmdDialog);
		set(0x02, "confirm", "", 0, &cmdConfirm);
		set(0x03, "confirmClose", "", 0, &cmdConfirm);
		set(0x04, "charLeft", "slot:-2,animation:2,modifier:1", 5);
		set(0x05, "portrait", "portraitId:-2", 2);
		set(0x06, "voice", "voiceId:2", 2);
		set(0x07, "speechBubble", "type:1,arrow:1", 2);
		set(0x08, "nameLabel", "nameId:-2,position:1", 3);
		set(0x09, "set", "flag:2", 2, &cmdTalkFlag);
		set(0x0A, "clear", "flag:2", 2, &cmdTalkFlag);
		set(0x0B, "script", "scriptId:3", 3, &cmdTalkScript);
		set(0x0C, "call", "talkScriptId:3", 3, &cmdTalkCall);
		set(0x0D, "scriptCustom", "", -1);
		set(0x0E, "charMid", "slot:-2,animation:2,modifier:1", 5);
		set(0x0F, "callCustom", "", -1);
		set(0x14, "menuEntry", "", -1, &cmdMenu);
		set(0x18, "charRight", "slot:-2,animation:2,modifier:1", 5);
		set(0x64, "loadImg", "value:3", 3);
		set(0x65, "confirmAuto", "delay:2,delayAlt:2,keepOpen:1", 5, &cmdConfirmAuto);
		set(0xC9, "showBox", "show:1", 1);
		set(0xFA, "nop", "skipped:1", 1);
		set(0xFD, "menuEnd", "", 0, &cmdReturn);
		set(0xFE, "restart", "", 0, &cmdRestart);
		set(0xFF, "return", "", 0, &cmdReturn);
		return out;
	}();
	return commands;
}

//==== Parsing ====

/// Length of a zero terminated string starting at offset, the terminator is not counted.
std::size_t stringLength(const std::vector<char>& code, std::size_t offset) {
	std::size_t end = offset;
	while (end < code.size() && code[end] != 0) ++end;
	return end - offset;
}

/**
* @brief Turns SCRIPT.DAT bytes into commands.
* @details Op code, body size, body. The size byte is a hint, the game advances by a size it knows
* per command, which only ever differs for SetEffectParam. An `if` carries its block inline, so its
* code is parsed like any other and the command only remembers where the block ends.
*/
void parseScript(const std::vector<char>& code, PG_DisaScriptLane& lane) {
	const auto& commands = scriptCommands();
	/* Open if blocks, the byte the block ends at and the command whose jump target it fills. */
	std::vector<std::pair<std::size_t, std::size_t>> openBlocks;

	auto closeBlocks = [&lane, &openBlocks](std::size_t offset) {
		while (!openBlocks.empty() && offset >= openBlocks.back().first) {
			const PG_DisaScriptCommand& command = lane.commands[openBlocks.back().second];
			const unsigned int target = static_cast<unsigned int>(lane.commands.size());
			memcpy(lane.data.data() + command.dataIndex + offsetof(IfData, jumpTarget), &target, sizeof(target));
			openBlocks.pop_back();
		}
	};

	std::size_t offset = 0;
	while (offset + 1 < code.size()) {
		closeBlocks(offset);

		const unsigned char opCode = static_cast<unsigned char>(code[offset]);
		const unsigned char sizeByte = static_cast<unsigned char>(code[offset + 1]);
		const CommandInfo& info = commands[opCode];

		if (opCode == 0x02 && openBlocks.empty()) {
			/* A return outside of any if block ends the script, the game never looks at what comes
			 * after it. Stopping here keeps the bytes of the next script out of the lane. */
			lane.add(opCode, info.command, nullptr, 0);
			break;
		}

		if (opCode == 0x07) {
			if (offset + 3 >= code.size()) break;
			const std::size_t blockSize = sizeByte + static_cast<std::size_t>(static_cast<unsigned char>(code[offset + 2])) * 256 + 1;
			const std::size_t blockEnd = offset + 2 + blockSize;

			IfData ifData;
			ifData.count = static_cast<unsigned char>(code[offset + 3]);
			const std::size_t conditionSize = ifData.count ? ifData.count * 7u + ifData.count - 1u : 0u;
			if (conditionSize > ifData.conditions.size() || offset + 4 + conditionSize > code.size()) {
				OUT_WARNING("Script \"" << lane.name << "\" has an if with " << static_cast<int>(ifData.count) << " conditions, it stops here.");
				break;
			}
			memcpy(ifData.conditions.data(), code.data() + offset + 4, conditionSize);

			openBlocks.emplace_back(blockEnd, lane.commands.size());
			lane.add(opCode, info.command ? info.command : &cmdLog, &ifData, sizeof(IfData));
			offset += 4 + conditionSize;
			continue;
		}

		const std::size_t bodySize = info.size >= 0 ? static_cast<std::size_t>(info.size) : sizeByte;
		if (offset + 2 + bodySize > code.size()) {
			OUT_WARNING("Script \"" << lane.name << "\" ends inside a command, it reaches into the script that follows it.");
			break;
		}

		lane.add(opCode, info.command ? info.command : &cmdLog, code.data() + offset + 2, bodySize);
		offset += 2 + bodySize;
	}

	closeBlocks(code.size()); //an if whose block reaches the end of the script still needs its target
}

/**
* @brief Turns TALK.DAT bytes into commands.
* @details There is no size byte here, every op code has a length the table knows. Strings and the
* menu entries are the two that have to be measured while reading.
*/
void parseTalkScript(const std::vector<char>& code, PG_DisaScriptLane& lane) {
	const auto& commands = talkCommands();

	std::size_t offset = 0;
	while (offset < code.size()) {
		const unsigned char opCode = static_cast<unsigned char>(code[offset]);
		const CommandInfo& info = commands[opCode];
		++offset;

		if (!info.name) {
			OUT_WARNING("Talk script \"" << lane.name << "\" has the unknown op code 0x" << std::hex << static_cast<int>(opCode) << std::dec << ", it stops here.");
			break;
		}

		if (opCode == 0x14) {
			/* A menu start carries the number of entries and is followed by a 0x01 marker, a
			 * further entry is only that marker. Both then hold the text of their entry. */
			if (offset >= code.size()) break;
			MenuData menu;
			menu.entryCount = static_cast<unsigned char>(code[offset++]);
			menu.isStart = menu.entryCount != 1;
			if (menu.isStart && offset < code.size() && code[offset] == 1) ++offset;

			const std::size_t length = stringLength(code, offset);
			menu.textIndex = sizeof(MenuData);

			std::vector<char> entry(sizeof(MenuData) + length + 1, 0);
			memcpy(entry.data(), &menu, sizeof(MenuData));
			memcpy(entry.data() + sizeof(MenuData), code.data() + offset, length);

			lane.add(opCode, info.command, entry.data(), entry.size());
			offset += length + 1;
			continue;
		}

		if (info.size < 0) { //dialog and the custom script names, a zero terminated string
			const std::size_t length = stringLength(code, offset);
			lane.add(opCode, info.command, code.data() + offset, length);
			offset += length + 1;
			continue;
		}

		if (offset + info.size > code.size()) {
			OUT_WARNING("Talk script \"" << lane.name << "\" ends inside a command.");
			break;
		}
		const std::size_t bodySize = static_cast<std::size_t>(info.size);
		lane.add(opCode, info.command, bodySize ? code.data() + offset : nullptr, bodySize);
		offset += bodySize;
	}
}

bool readEntry(const PG_DisaFileEntry& entry, std::vector<char>& code) {
	auto reader = entry.createReader();
	if (!reader || !reader->isOpen()) return FAILURE;
	reader->read(code, reader->size());
	return SUCCESS;
}

} //namespace

void PG_DisaScriptExecutor::setContainers(std::shared_ptr<PG_Script> scripts, std::shared_ptr<PG_TalkScript> talkScripts) {
	m_scripts = std::move(scripts);
	m_talkScripts = std::move(talkScripts);
}

bool PG_DisaScriptExecutor::runScriptById(const PG_DisaIdentifier& scriptId, int parentLane) {
	PG_DisaFileEntry entry = findScript(scriptId);
	if (!entry) {
		OUT_WARNING("No script with the ID " << scriptId << " in SCRIPT.DAT!");
		return FAILURE;
	}
	return addLane(entry, PG_DisaScriptLane::Script, scriptId, parentLane) >= 0;
}

bool PG_DisaScriptExecutor::runTalkScriptById(const PG_DisaIdentifier& identifier, int parentLane) {
	/* The game keeps one talk script at a time and refuses the search outright while one runs. */
	if (hasRunningTalkLane()) {
		OUT_WARNING("A talk script is already running, the search for " << identifier << " is refused.");
		return FAILURE;
	}

	PG_DisaFileEntry entry = findTalkScript(identifier);
	if (!entry) {
		OUT_WARNING("No talk script with the ID " << identifier << " in TALK.DAT!");
		return FAILURE;
	}

	const int index = addLane(entry, PG_DisaScriptLane::Talk, identifier, parentLane);
	if (index < 0) return FAILURE;
	m_lanes[index].search = identifier;
	return SUCCESS;
}

int PG_DisaScriptExecutor::addLane(const PG_DisaFileEntry& entry, PG_DisaScriptLane::EType type, const PG_DisaIdentifier& scriptId, int parentLane) {
	std::vector<char> code;
	if (!readEntry(entry, code)) {
		OUT_ERROR("Failed to read \"" << entry.getName() << "\"!");
		return -1;
	}

	PG_DisaScriptLane lane;
	lane.type = type;
	lane.scriptId = scriptId;
	lane.parent = parentLane;
	lane.name = entry.getName();

	//a lane started by another shares its slots, one the user started opens a chain of its own
	if (parentLane >= 0 && parentLane < static_cast<int>(m_lanes.size())) lane.slots = m_lanes[parentLane].slots;
	if (!lane.slots) {
		auto* map = context().getMapActors();
		if (map) lane.slots.reset(map->createScriptViewModel());
	}
	if (type == PG_DisaScriptLane::Talk) parseTalkScript(code, lane);
	else parseScript(code, lane);

	if (lane.commands.empty()) {
		OUT_WARNING("\"" << lane.name << "\" holds no command!");
		return -1;
	}

	m_lanes.push_back(std::move(lane));
	const int index = static_cast<int>(m_lanes.size()) - 1;
	OUT_DEBUG("Lane " << index << " runs \"" << m_lanes[index].name << "\" with " << m_lanes[index].commands.size()
		<< " commands" << (parentLane >= 0 ? ", started by lane " + std::to_string(parentLane) : std::string()) << '.');
	return index;
}

PG_DisaFileEntry PG_DisaScriptExecutor::findScript(const PG_DisaIdentifier& identifier) const {
	if (!m_scripts) return {};
	const auto scriptId = identifier.toNumber();

	for (const PG_DisaFileEntry& entry : *m_scripts) {
		if (PG_Script::getScriptId(entry) == scriptId) return entry;
	}
	return {};
}

PG_DisaFileEntry PG_DisaScriptExecutor::findTalkScript(const PG_DisaIdentifier& identifier) const {
	/* A search of nothing but wildcards would hand back the very first entry of the archive, which
	* is never what the caller means. */
	if (!m_talkScripts || !identifier.isValid()) return {};

	auto ids = identifier.getIds();
	if (ids.size() < 1) return {};
	const unsigned int scriptId = ids.front();
	const unsigned int dungeonId = ids.size() >= 2 ? ids[1] : 0;
	const unsigned int searchFlag = ids.size() >= 3 ? ids[2] : 0;

	for (const PG_DisaFileEntry& entry : *m_talkScripts) {
		const talkScriptFileInfoHeader info = PG_TalkScript::getScriptInfo(entry);
		if (scriptId && info.scriptId != scriptId) continue;
		if (dungeonId && info.dungeonId != dungeonId) continue;
		if (searchFlag && info.searchFlag != searchFlag) continue;

		/* On top of the search every positive filter has to be a set game state bit and every
		 * negative one a clear bit, which is what sorts the story variants of a scene. */
		const std::size_t positives = std::min<std::size_t>(info.gameStateFilterPositiveSize, info.gameStateFilterPositive.size());
		const std::size_t negatives = std::min<std::size_t>(info.gameStateFilterNegativeSize, info.gameStateFilterNegative.size());
		const auto isSet = [c = &context()](std::uint16_t bit) { return c->getGameFlag(bit); };

		if (!std::all_of(info.gameStateFilterPositive.begin(), info.gameStateFilterPositive.begin() + positives, isSet)) continue;
		if (std::any_of(info.gameStateFilterNegative.begin(), info.gameStateFilterNegative.begin() + negatives, isSet)) continue;
		return entry;
	}
	return {};
}

bool PG_DisaScriptExecutor::isBlockingScriptRunning() const {
	/* The game leaves the scripts of this range out of the counter the talk interpreter checks,
	 * so those keep running next to the dialog instead of holding it back. */
	constexpr unsigned int firstFreeRunning = 1000001;
	constexpr unsigned int lastFreeRunning = 1999999;

	for (const PG_DisaScriptLane& lane : m_lanes) {
		if (lane.finished || lane.type != PG_DisaScriptLane::Script) continue;
		const unsigned int scriptId = lane.scriptId.getNameAsNumber();
		if (scriptId < firstFreeRunning || scriptId > lastFreeRunning) return true;
	}
	return false;
}

bool PG_DisaScriptExecutor::hasRunningTalkLane() const {
	for (const PG_DisaScriptLane& lane : m_lanes) {
		if (!lane.finished && lane.type == PG_DisaScriptLane::Talk) return true;
	}
	return false;
}

bool PG_DisaScriptExecutor::replaceLane(std::size_t laneIndex, const PG_DisaFileEntry& entry, const PG_DisaIdentifier& identifier) {
	std::vector<char> code;
	if (!readEntry(entry, code)) return FAILURE;

	/* The game has no call stack, the new script takes the running one over, so the lane keeps its
	 * place and everything it was running is dropped. */
	PG_DisaScriptLane& lane = m_lanes[laneIndex];
	lane.commands.clear();
	lane.data.clear();
	lane.memory = PG_DisaScriptLaneMemory();
	lane.cursor = 0;
	lane.scriptId = identifier;
	lane.search = identifier;
	lane.name = entry.getName();
	parseTalkScript(code, lane);

	OUT_DEBUG("Lane " << laneIndex << " continues in \"" << lane.name << "\" with " << lane.commands.size() << " commands.");
	return !lane.commands.empty();
}

bool PG_DisaScriptExecutor::callTalkScript(std::size_t laneIndex, unsigned int scriptId) {
	/* A call only knows an ID, the dungeon and the flag stay wildcards. */
	PG_DisaIdentifier identifier = PG_DisaIdentifier::disaIdentifierFromIds(scriptId);
	PG_DisaFileEntry entry = findTalkScript(identifier);
	if (!entry) {
		OUT_WARNING("No talk script with the ID " << scriptId << " in TALK.DAT!");
		return FAILURE;
	}
	return replaceLane(laneIndex, entry, identifier);
}

bool PG_DisaScriptExecutor::restartTalkScript(std::size_t laneIndex) {
	const PG_DisaIdentifier search = m_lanes[laneIndex].search;
	PG_DisaFileEntry entry = findTalkScript(search);
	if (!entry) {
		OUT_WARNING("Nothing matches " << search  << " any more, the lane ends.");
		return FAILURE;
	}
	return replaceLane(laneIndex, entry, search);
}

bool PG_DisaScriptExecutor::hasRunningChildren(std::size_t laneIndex) const {
	for (const PG_DisaScriptLane& lane : m_lanes) {
		if (!lane.finished && lane.parent == static_cast<int>(laneIndex)) return true;
	}
	return false;
}



PG_DisaContext& PG_DisaScriptExecutor::context() {
	if (auto* context = getParent<PG_DisaContext>()) return *context;
	static PG_DisaContext local;
	return local;
}

const PG_DisaContext& PG_DisaScriptExecutor::context() const {
	return const_cast<PG_DisaScriptExecutor*>(this)->context();
}

bool PG_DisaScriptExecutor::isRunning() const {
	for (const PG_DisaScriptLane& lane : m_lanes) {
		if (!lane.finished) return true;
	}
	return false;
}

void PG_DisaScriptExecutor::stopAll() {
	m_lanes.clear();
}

void PG_DisaScriptExecutor::tick(double deltaTimeSeconds) {
	if (m_lanes.empty()) return;

	const float deltaTicks = static_cast<float>(deltaTimeSeconds) * ticksPerSecond;
	/* A lane a command starts is appended and ticked in the same pass, which is what the game does
	 * too, so the size is read fresh every round. */
	for (std::size_t i = 0; i < m_lanes.size(); ++i) {
		runLane(i, deltaTicks);
	}

	if (isRunning()) return;

	OUT_DEBUG("Every lane finished.");
	m_lanes.clear();
	emit allLanesFinished();
}

void PG_DisaScriptExecutor::runLane(std::size_t laneIndex, float deltaTicks) {
	if (m_lanes[laneIndex].finished) return;

	/* The game runs its talk interpreter only while no script is running, so a talk script never
	 * walks ahead of the cutscene it started. Its waits do not tick down meanwhile either, the
	 * whole thing is simply not updated. */
	const bool isTalk = m_lanes[laneIndex].type == PG_DisaScriptLane::Talk;
	if (isTalk && isBlockingScriptRunning()) return;

	if (m_lanes[laneIndex].memory.waitTicks > 0.f) {
		m_lanes[laneIndex].memory.waitTicks -= deltaTicks;
		if (m_lanes[laneIndex].memory.waitTicks > 0.f) return;
		m_lanes[laneIndex].memory.waitTicks = 0.f;
	}

	/* A script with a loop that never waits would spin here forever, so the run is capped and the
	 * lane dropped rather than the window. */
	constexpr int maxCommandsPerTick = 100000;
	for (int run = 0; run < maxCommandsPerTick; ++run) {
		if (m_lanes[laneIndex].isAtEnd()) {
			OUT_DEBUG("Lane " << laneIndex << " ran out of commands without a return.");
			m_lanes[laneIndex].finished = true;
			return;
		}

		const PG_DisaScriptCommand command = m_lanes[laneIndex].commands[m_lanes[laneIndex].cursor];
		const void* data = m_lanes[laneIndex].getData(command);
		const EDisaScriptResult result = (*command.command)(*this, laneIndex, deltaTicks, data);

		switch (result) {
			case EDisaScriptResult::Done: ++m_lanes[laneIndex].cursor; break;
			case EDisaScriptResult::Jump: break; //the command placed the cursor itself
			case EDisaScriptResult::Running: return; //same command again next tick
			case EDisaScriptResult::Stop:
				m_lanes[laneIndex].finished = true;
				return;
		}

		if (m_lanes[laneIndex].memory.waitTicks > 0.f) return;
		//a script this lane just started takes over, the rest of the talk script waits for it
		if (isTalk && isBlockingScriptRunning()) return;
	}

	OUT_WARNING("Lane " << laneIndex << " ran " << maxCommandsPerTick << " commands in one tick, it looks stuck and is dropped.");
	m_lanes[laneIndex].finished = true;
}
