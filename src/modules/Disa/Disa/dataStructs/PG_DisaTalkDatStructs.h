#pragma once

#include <array>
#include <cstdint>

#include <Util/Typedefs.h>

/*!
 * @brief One bit of the global game state, the way TALK.DAT and the game itself pack it.
 * @details The low three bits pick the bit inside a byte, the remaining thirteen are the index of
 * that byte in the game state array, which the game keeps at 0x00619330. So the whole 16 bit value
 * is a flat bit number, the same one the script commands SetFlag and If count in.
 */
PACK(struct PG_DisaGameStateFilter // 2 bytes
{
	std::uint16_t bitIndex : 3; //the bit of the byte that is tested
	std::uint16_t index : 13;   //the byte of the game state array

	/// The whole 16 bit as one number, which is the flat bit the script commands count in.
	inline std::uint16_t getValue() const noexcept { return static_cast<std::uint16_t>(index << 3 | bitIndex); }
	inline void setValue(std::uint16_t value) noexcept { bitIndex = value & 7; index = value >> 3; }

	inline std::uint16_t getIndex() const noexcept { return index; }
	inline std::uint8_t getMask() const noexcept { return static_cast<std::uint8_t>(1 << bitIndex); }

	/// True while the bit is not set, which is what a negative filter of a script asks for.
	inline bool isClear(const unsigned char* gameState) const {
		return (gameState[getIndex()] & getMask()) == 0;
	}
	/// True while the bit is set, which is what a positive filter asks for.
	inline bool isSet(const unsigned char* gameState) const { return !isClear(gameState); }

	inline void set(unsigned char* gameState) const {
		gameState[getIndex()] = gameState[getIndex()] | getMask();
	}
	inline void clear(unsigned char* gameState) const {
		gameState[getIndex()] = gameState[getIndex()] & ~getMask();
	}

	inline bool isEmpty() const { return bitIndex == 0 && index == 0; }

	bool operator==(const PG_DisaGameStateFilter& other) const = default;
});
static_assert(sizeof(PG_DisaGameStateFilter) == 2, "A game state filter is 2 byte!");

/*!
 * @brief One entry of the TALK.DAT file table, the game picks a script by matching against it.
 * @details The game searches for a script by ID, or, when there is none, by dungeon and search
 * flag narrowed down by the game state filters: every positive one has to be a set game state bit
 * and every negative one a clear bit, which is what sorts the story variants of a scene. Only the
 * first gameStateFilterPositiveSize / ..NegativeSize entries of each list are looked at.
 *
 * The game holds the loaded table as an array of these, so this is both the file layout and the
 * runtime one. @see PG_TalkScript @see PG_DisaScriptManager.h
 */
PACK(struct PG_DisaTalkDatScriptEntry // 32 bytes
{
	std::uint32_t startAddress; //the header is not counted, so the first script starts at 0
	std::uint32_t scriptId; //can be 0 == null
	std::uint16_t dungeonId;
	std::uint16_t searchFlag; //unknown, mostly 0-2 and 102-111
	std::uint16_t gameStateFilterPositiveSize; //0-4
	std::uint16_t gameStateFilterNegativeSize; //0-4
	std::array<PG_DisaGameStateFilter, 4> gameStateFilterPositive;
	std::array<PG_DisaGameStateFilter, 4> gameStateFilterNegative;

	/// Compares every member, the filters past the two sizes included
	bool operator==(const PG_DisaTalkDatScriptEntry& other) const = default;
});
static_assert(sizeof(PG_DisaTalkDatScriptEntry) == 32, "A TALK.DAT file table entry is 32 byte!");
