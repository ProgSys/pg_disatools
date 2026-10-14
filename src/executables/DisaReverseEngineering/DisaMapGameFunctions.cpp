#include "DisaMapGameFunctions.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <ranges>
#include <span>

#include <glm/glm.hpp>

#include <FunctionPatcher.h>
#include <DisaFunctions.h>
#include <DisaMapFunctions.h>
#include <DisaCharacterFunctions.h>
#include <DisaMapChunk.h>
#include <Disa/fileformats/PG_MPD.h>

/*
* Port of FUN_0045c0f0. Globals keep their address in the name (DAT_xxxxxxxx_name), parts of the original function
* are named after the address of their first instruction (LAB_xxxxxxxx_name) and callees after their own address
* (FUN_xxxxxxxx_name), so all of them can be found again in Ghidra, the disassembly and memory dumps.
*/
static_assert(offsetof(PG_DisaPartyCharacter, currentHP) == 0x5c4);
static_assert(offsetof(PG_DisaPartyCharacter, activeStats) == 0x5cc);
static_assert(offsetof(PG_DisaPartyCharacter, jumpingPower) == 0x67b);
static_assert(offsetof(PG_DisaPartyCharacter, movement) == 0x67d);
static_assert(offsetof(PG_DisaPartyCharacter, unknownData6) == 0x684);
static_assert(offsetof(PG_DisaPartyCharacter, field_0x68b) == 0x68a); //misnamed, it is at 0x68a
static_assert(offsetof(PG_DisaPartyCharacter, field_0x6ac) == 0x6ac);
static_assert(offsetof(mpdfileTile, geo_color) == 0x78);
static_assert(offsetof(mpdfileTile, geo_mark) == 0x79);

namespace {

    // ---- Globals ----

    /// DAT_00889618, the battle state of layer 0, layer 1 (versus mode) follows at 0x0088961C.
    std::array<int32_t, 2>& DAT_00889618_battleStates() { return FP::getData<std::array<int32_t, 2>>(0x00889618); }
    int32_t& DAT_00889618_battleState() { return DAT_00889618_battleStates()[0]; }
    /// DAT_00889620, 16 pushed states per layer.
    std::array<std::array<int32_t, 16>, 2>& DAT_00889620_battleStateStacks() {
        return FP::getData<std::array<std::array<int32_t, 16>, 2>>(0x00889620);
    }
    /// DAT_008896a0, the stack depth per layer.
    std::array<int32_t, 2>& DAT_008896a0_battleStateDepths() { return FP::getData<std::array<int32_t, 2>>(0x008896a0); }

    /// DAT_008895ac, the unit under the cursor, -1 none. Found again each frame.
    int32_t& DAT_008895ac_hoveredUnit() { return FP::getData<int32_t>(0x008895ac); }
    /// DAT_008895b0, the unit the states work on. Never cleared.
    int32_t& DAT_008895b0_selectedUnit() { return FP::getData<int32_t>(0x008895b0); }
    /// DAT_008895ec, the entry of the command menu.
    int32_t& DAT_008895ec_menuCursor() { return FP::getData<int32_t>(0x008895ec); }

    /// DAT_0065db3a, 0 story, 1 Item World, 2 and 4 not traced, 3 bonus floors, 5 versus.
    uint8_t& DAT_0065db3a_gameMode() { return FP::getData<uint8_t>(0x0065db3a); }
    /// DAT_006b7578, nonzero while the AI or an event moves units, the states wait for it.
    int32_t& DAT_006b7578_busyCount() { return FP::getData<int32_t>(0x006b7578); }
    /// DAT_0067726b, set when a click or key was handled this frame, stops the mouse camera pan.
    uint8_t& DAT_0067726b_isInputConsumed() { return FP::getData<uint8_t>(0x0067726b); }
    /// DAT_0067726a, set while the mouse drags the facing of a unit.
    uint8_t& DAT_0067726a_isTurningUnit() { return FP::getData<uint8_t>(0x0067726a); }

    /// DAT_00778d98, the keys of this frame, 2 is pressed.
    DisaKeyStates& DAT_00778d98_keyStates() { return FP::getData<DisaKeyStates>(0x00778d98); }
    /// DAT_005d7c40
    DisaSettings* DAT_005d7c40_settings() { return FP::getData<DisaSettings*>(0x005d7c40); }

    /// PTR_00730980, the battle camera.
    DisaCameraInfo& DAT_00730980_camera() { return FP::getData<DisaCameraInfo>(0x00730980); }
    /**
    * @brief DAT_00730ba0, the map cursor of layer 0, layer 1 uses 0x00778A10.
    * Only the sprite actor part is valid, it ends before DisaMapUnit::character.
    */
    DisaMapUnit& DAT_00730ba0_cursor() { return FP::getData<DisaMapUnit>(0x00730ba0); }

    /// DAT_006e80f0 + 0x1740, scratch transform for effects.
    DisaTransform* getEffectTransform() {
        return reinterpret_cast<DisaTransform*>(FP::getData<uint8_t*>(0x006e80f0) + 0x1740);
    }

    /// DAT_006e6f58, the system strings, offset is in bytes like in the original.
    const uint8_t* getSystemString(int offset) {
        return *reinterpret_cast<const uint8_t* const*>(FP::getData<uint8_t*>(0x006e6f58) + offset);
    }

    // ---- Small game functions ----

    /// Like the original, which multiplies with the float 1 / 12 at double precision and truncates.
    int toTile(float position) {
        return static_cast<int>(position * double(1.f / 12.f));
    }

    /**
    * @brief Pushes the current state of the layer and switches to the given one.
    * Original function: FUN_00446d10
    */
    void FUN_00446d10_pushBattleState(int32_t state, int8_t layer) {
        int32_t& depth = DAT_008896a0_battleStateDepths()[layer];
        DAT_00889620_battleStateStacks()[layer][depth] = DAT_00889618_battleStates()[layer];
        DAT_00889618_battleStates()[layer] = state;
        ++depth;
    }

    /**
    * @brief Returns to the last pushed state of the layer, nothing when the stack is empty.
    * Original function: FUN_00446d50
    * @details The original reads 0x0088961C + (layer * 16 + depth) * 4, which is the stack entry at depth - 1.
    */
    void FUN_00446d50_popBattleState(int8_t layer) {
        int32_t& depth = DAT_008896a0_battleStateDepths()[layer];
        if (depth == 0) return;
        DAT_00889618_battleStates()[layer] = DAT_00889620_battleStateStacks()[layer][depth - 1];
        --depth;
    }

    /**
    * @brief Replaces the state of the layer and empties its stack.
    * Original function: FUN_00444740
    */
    void FUN_00444740_setBattleState(int32_t state, int8_t layer) {
        DAT_008896a0_battleStateDepths()[layer] = 0;
        DAT_00889618_battleStates()[layer] = state;
    }

    /**
    * @brief Counts the flying projectiles, the states wait until none are left.
    * Original function: FUN_00444760
    * @details 128 entries of 0xf0 bytes from 0x00730F10, the int16 at 0x9c is the active flag.
    */
    int FUN_00444760_countProjectiles() {
        const uint8_t* projectiles = &FP::getData<uint8_t>(0x00730f10);
        int count = 0;
        for (int i = 0; i < 128; ++i) {
            count += *reinterpret_cast<const int16_t*>(projectiles + i * 0xf0 + 0x9c) != 0;
        }
        return count;
    }

    /**
    * @brief The versus fog of war value of the tile at a world position, 0xff when the layer cannot see it.
    * Original function: FUN_00444990, returns 0 outside of versus fog.
    * @details Like the original the tiles are not range checked, the index wraps through int8.
    */
    uint8_t FUN_00444990_getFog(int8_t layer, float x, float z) {
        if ((layer != 0 && layer != 1) || DAT_0065db3a_gameMode() != 5 || FP::getData<uint8_t>(0x006e57b3) == 0) return 0;
        const int8_t tileX = static_cast<int8_t>(toTile(x));
        const int8_t tileZ = static_cast<int8_t>(toTile(z));
        //[layer][x][z]
        return (&FP::getData<uint8_t>(0x005d7cd0))[tileZ + (tileX + layer * 0x40) * 0x40];
    }

    /**
    * @brief True when the layer waits for input, so the versus turn can be handed over.
    * Original function: FUN_00444c70
    */
    bool FUN_00444c70_isLayerIdle(int8_t layer) {
        const DisaMapUnit& cursor = layer == 1 ? FP::getData<DisaMapUnit>(0x00778a10) : DAT_00730ba0_cursor();
        const int32_t state = DAT_00889618_battleStates()[layer];
        const bool isBusy = DAT_006b7578_busyCount() != 0;
        //the original compares moveFramesLeft with the busy count, which is 0 by then
        const auto isMoving = [&]() { return isBusy || cursor.moveFramesLeft != 0 || FUN_00444760_countProjectiles() != 0; };
        if ((state == 0 || state == 1 || state == 0x17) && isMoving()) return false;
        constexpr std::array<int32_t, 13> busyStates = { 0x15, 0x28, 0x87, 200, 0x96, 0xa0, 0x3e9, 0x4b0, 0x4b1, 0x4b2, 11000, 0x2af9, 0x3ea };
        return std::ranges::find(busyStates, state) == busyStates.end();
    }

    /**
    * @brief Plays a system sound at the sound effect volume.
    * Original function: FUN_0046aab0
    */
    void FUN_0046aab0_playSound(int id) {
        FP::callFastFarA<void>(fPlaySound, id, FP::getData<int>(0x00677274), 0, id, 0);
    }

    /**
    * @brief A bit of the game state, the bits the scripts check and set.
    * Original function: FUN_0046de80
    */
    bool FUN_0046de80_isGameFlagSet(int bit) {
        return ((&FP::getData<uint8_t>(0x00619330))[bit >> 3] & (1 << (bit & 7))) != 0;
    }

    /**
    * @brief Clears a bit of the game state.
    * Original function: FUN_0046dec0
    */
    void FUN_0046dec0_clearGameFlag(int bit) {
        (&FP::getData<uint8_t>(0x00619330))[bit >> 3] &= ~(1 << (bit & 7));
    }

    /// Character byte 0x306, the turns the character still sleeps.
    int8_t getSleepTurns(const DisaPartyCharacterInfo* character) {
        return reinterpret_cast<const int8_t*>(character)[0x306];
    }

    /**
    * @brief 0 can act, 1 asleep, 2 defeated. A unit without character can act.
    * Original function: FUN_0046f640, Ghidra types it as bool.
    */
    int FUN_0046f640_getCondition(const DisaPartyCharacterInfo* character) {
        if (!character) return 0;
        if (static_cast<int32_t>(character->currentHP) < 1) return 2;
        return getSleepTurns(character) > 0 ? 1 : 0;
    }

    /// True with the mouse in use and the game window focused, the check in front of every mouse helper.
    bool isMouseUsable() {
        const uint8_t* window = FP::getData<uint8_t*>(0x005d7c70);
        return DAT_005d7c40_settings()->isMouseActive && window[0x305438] == 0 && window[0x30543a] != 0;
    }

    /**
    * @brief The left button is held longer than 10 frames.
    * Original function: FUN_0050ff80
    */
    bool FUN_0050ff80_isLeftMouseLongPress() {
        return isMouseUsable() && FP::getData<uint8_t>(0x00778dca) != 0;
    }

    /**
    * @brief The right button is held longer than 10 frames.
    * Original function: FUN_0050ffb0
    */
    bool FUN_0050ffb0_isRightMouseLongPress() {
        return isMouseUsable() && FP::getData<uint8_t>(0x00778dd4) != 0;
    }

    /**
    * @brief The left button is down.
    * Original function: FUN_0050f910
    */
    bool FUN_0050f910_isLeftMouseDown() {
        return isMouseUsable() && (FP::getData<uint32_t>(0x00701c50) & 0x4000000) != 0;
    }

    /**
    * @brief True when the versus turn timer ran out.
    * Original function: FUN_00445590
    */
    bool FUN_00445590_isVersusTimeUp() {
        if (DAT_0065db3a_gameMode() != 5 || FP::getData<int16_t>(0x006e57b0) == 0) return false;
        return FP::getData<int32_t>(0x006d4ab8) == 0;
    }

    /// Pad button 0x8000 is held, DAT_0065db47 flips its meaning. Shifts Next and Previous Page to the camera.
    bool isPadShiftReleased() {
        return (FP::getData<uint32_t>(0x00778d64) & 0x8000) != static_cast<uint32_t>(FP::getData<int8_t>(0x0065db47) * 0x8000);
    }

    /// Stops the glide of the cursor, the states do it before they take over the cursor.
    void stopCursor(DisaMapUnit& cursor) {
        cursor.moveFramesLeft = 0;
        cursor.velocity.z = 0.f;
        cursor.velocity.x = 0.f;
    }

    // ---- Game functions ----

    /// Original function: FUN_00444810, rebuilds the versus fog of war of the layer and the map overlay.
    void FUN_00444810_updateFog(int8_t layer) { FP::callStdFastFarA<void>(0x00444810, layer); }
    /// Original function: FUN_004fd640, records the input of the frame for the versus replay.
    void FUN_004fd640_recordVersusInput() { FP::callFarA<void>(0x004fd640); }
    /// Original function: FUN_00466970, the deployment and the battle start, 1 once done.
    int FUN_00466970_updateBattleStart(int mode) { return FP::callStdFastFarA<int>(0x00466970, mode); }
    /// Original function: FUN_0044b480, the info windows of the hovered tile and unit.
    void FUN_0044b480_drawHoverInfo() { FP::callFarA<void>(0x0044b480); }
    /// Original function: FUN_0043dba0, the key bound to the action was pressed, with its modifier key when it has one.
    bool FUN_0043dba0_isActionPressed(int action) { return FP::callStdFarA<int>(0x0043dba0, action) != 0; }
    /// Original function: FUN_0050f7c0, a left click that did not move the mouse.
    bool FUN_0050f7c0_isLeftMouseClicked() { return FP::callStdFastFarA<bool>(fIsLeftMouseBtnClicked, false); }
    /// Original function: FUN_0050f950, a right click or the mouse back button.
    bool FUN_0050f950_isRightMouseClicked() { return FP::callStdFastFarA<bool>(fIsExitPressed, false); }
    /// Original function: FUN_0050fe60, forgets the pending mouse clicks.
    void FUN_0050fe60_clearMouseClicks() { FP::callFarA<void>(0x0050fe60); }
    /// Original function: FUN_004d9dc0, rotates and zooms the camera by mouse drag, wheel and the page keys.
    void FUN_004d9dc0_updateCameraControls(DisaCameraInfo& camera) { FP::callStdFastFarA<void>(0x004d9dc0, &camera); }
    /// Original function: FUN_00449780, moves the cursor to the next unit of the layer that can still act.
    void FUN_00449780_focusNextUnit(int hovered, int8_t layer) { FP::callStdFastFarA<int>(0x00449780, hovered, layer); }
    /// Original function: FUN_00449af0, moves the cursor to the next player unit.
    void FUN_00449af0_focusNextPlayerUnit(int hovered) { FP::callStdFastFarA<int>(0x00449af0, hovered); }
    /// Original function: FUN_00449c70, moves the cursor to the Item World gate.
    void FUN_00449c70_focusGate(int hovered) { FP::callStdFastFarA<int>(0x00449c70, hovered); }
    /// Original function: FUN_00449db0, moves the cursor to the base panel.
    void FUN_00449db0_focusBasePanel(int hovered) { FP::callStdFastFarA<int>(0x00449db0, hovered); }
    /// Original function: FUN_004d0db0, the facing the pad asks for, -1 none. Recorded is the versus replay input.
    int FUN_004d0db0_getPadFacing(DisaCameraInfo& camera, bool isRecorded) {
        return FP::callFastFarA<int>(0x004d0db0, &camera, 0, int(isRecorded));
    }
    /// Original function: FUN_004d1c90, the facing from the unit to the mouse, -1 none. The first stack slot is unused.
    int FUN_004d1c90_getMouseFacing(DisaCameraInfo& camera, DisaMapUnit& cursor, DisaMapUnit& unit, bool isRecorded) {
        return FP::callFastFarA<int>(0x004d1c90, &camera, 0, 0, &cursor, &unit, int(isRecorded));
    }
    /// Original function: FUN_004cf1d0, turns the unit to a facing from 0 to 3, nothing for -1.
    void FUN_004cf1d0_turnUnit(DisaMapUnit& unit, int facing) { FP::callStdFastFarA<void>(0x004cf1d0, &unit, facing); }
    /// Original function: FUN_004485d0, undoes the last Defend, lift or move of the unit, 0 when there is nothing.
    int FUN_004485d0_undoUnit(int index, int8_t layer) { return FP::callStdFastFarA<int>(0x004485d0, index, layer); }
    /// Original function: FUN_004486e0, lists the party members that can leave the base panel into DAT_008896b8.
    void FUN_004486e0_buildDeployList(int8_t layer) { FP::callStdFastFarA<void>(0x004486e0, layer); }
    /// Original function: FUN_0046a650, the shortcut that ends the deployment and starts the battle. Sound is only passed on.
    void FUN_0046a650_startBattle(int sound) { FP::callStdFastFarA<void>(0x0046a650, sound); }
    /// Original function: FUN_0046a710, the End Turn shortcut.
    void FUN_0046a710_endTurn() { FP::callFarA<void>(0x0046a710); }
    /// Original function: FUN_004da8d0, counts the living units per alliance into DAT_006b757c. Bit 0 skips geo symbols.
    void FUN_004da8d0_countUnits(uint32_t flags) { FP::callStdFastFarA<void>(0x004da8d0, flags); }
    /// Original function: FUN_004d3450, places a party member in the first free unit, returns its index or -1.
    int FUN_004d3450_spawnUnit(int slot, int16_t mode, float x, float y, float z, bool isFlag) {
        return FP::callFastFarA<int>(0x004d3450, slot, int(mode), x, y, z, int(isFlag));
    }
    /**
    * @brief Original function: FUN_004d21f0, floods the move range of a unit.
    * The port FUN_004d21f0_buildMoveRange is not hooked in main.cpp yet, so this calls the original like the game does.
    */
    void FUN_004d21f0_buildMoveRangeOriginal(DisaMapUnit& unit, uint32_t flags, int8_t layer) {
        FP::callFastFarA<void>(0x004d21f0, &unit, flags, int(layer));
    }

    /**
    * @brief Original function: FUN_004d1e50, traces the path of a unit to a tile, 0 on success.
    * The port FUN_004d1e50_buildPath is not hooked in main.cpp yet, so this calls the original like the game does.
    */
    int FUN_004d1e50_buildPathOriginal(DisaMapUnit& unit, int x, int z, int8_t layer) {
        return FP::callFastFarA<int>(0x004d1e50, &unit, x, z, int(layer));
    }
    /// Original function: FUN_004a1eb0, queues the blue tiles of the move range up to the given distance.
    void FUN_004a1eb0_drawMoveRange(int movement) { FP::callStdFastFarA<void>(0x004a1eb0, movement); }
    /// Original function: FUN_00447f00, the geo panel info of the cursor tile.
    void FUN_00447f00_drawTileInfo() { FP::callFarA<void>(0x00447f00); }
    /// Original function: FUN_0046a930, queues the name plate of a unit at height y.
    void FUN_0046a930_drawNamePlate(int y, DisaMapUnit& unit, int mode) {
        //ecx and the third stack slot are never read
        FP::callFastFarA<void>(0x0046a930, 0, y, &unit, mode, 0);
    }
    /**
    * @brief Original function: FUN_004b03c0, the status window of a character.
    * Mode 10 is the full window, 0x68 the small one of a hovered unit. The first stack slot is never read.
    */
    void FUN_004b03c0_drawStatusWindow(int x, DisaPartyCharacterInfo* character, int mode) {
        FP::callVariadicThisFarA<void>(0x004b03c0, x, 0, character, character, mode, -1);
    }
    /// Original function: FUN_0047f2d0, the equip screen of the character, not 0 once it closes.
    int FUN_0047f2d0_updateEquipScreen(int8_t mode, DisaPartyCharacterInfo* character) {
        return FP::callStdFastFarA<int>(0x0047f2d0, mode, character);
    }
    /// Original function: FUN_004da100, updates the status icons of the unit.
    void FUN_004da100_updateStatusIcons(DisaMapUnit& unit) { FP::callStdFastFarA<int>(0x004da100, &unit); }
    /// Original function: FUN_004d2960, marks the geo panels with the No Entry effect.
    void FUN_004d2960_updateGeoPanels() { FP::callFarA<void>(0x004d2960); }
    /// Original function: FUN_00447bf0, a unit of another alliance on the target tile, 0xff a base panel, -1 none.
    int FUN_00447bf0_findBlocker(int index, bool isAtPosition) {
        return FP::callStdFastFarA<int>(0x00447bf0, index, int(isAtPosition));
    }
    /// Original function: FUN_004475c0, moves the unit one step along its path, 1 once it arrived.
    int FUN_004475c0_updateWalk(int index) { return FP::callStdFastFarA<int>(0x004475c0, index); }
    /// Original function: FUN_004783d0, starts an effect sprite at the position of the transform.
    void FUN_004783d0_spawnEffect(int sprite, int animation, DisaTransform* transform) {
        FP::callFastFarA<void>(0x004783d0, sprite, animation, transform);
    }
    /// Original function: FUN_004e3440, converts a system string for the new text style.
    void FUN_004e3440_convertString(void* destination, const void* source) {
        FP::callStdFastFarA<void>(0x004e3440, destination, source);
    }
    /// Original function: FUN_004e42c0, converts a system string for the new text style, formats it on the way.
    void FUN_004e42c0_convertString(void* destination, const void* source) {
        FP::callCdeclFarA<void>(0x004e42c0, destination, source);
    }
    /// Original function: FUN_004dd950, writes a format string with a name into buffer, position receives where.
    void FUN_004dd950_formatName(const void* format, void* buffer, const char* name, int* position, int style) {
        FP::callFastFarA<void>(0x004dd950, format, 0, buffer, name, position, style);
    }

    /// Names that FUN_004dd950 placed into a message line, the message draws them in their own text style.
    struct MessageNames {
        const char* const* names = nullptr;
        const int* positions = nullptr;
        const int* styles = nullptr;
        int count = 0;
    };

    /**
    * @brief Original function: FUN_00477550, a message window shown for a number of frames.
    * x and y -1 center it, color points to a palette entry like 0x005AD538.
    */
    void FUN_00477550_showMessage(int x, int y, int width, int lineCount, int frames, const void* color, const void* line1,
        const void* line2 = nullptr, const MessageNames& names = {}) {
        FP::callFastFarA<void>(0x00477550, x, y, width, lineCount, frames, color, line1, line2, names.names, names.positions, 0,
            names.styles, names.count, 0);
    }

    /// The old text style, 0 or 1, takes the system strings as they are. The others convert them first.
    bool isOldTextStyle() {
        const uint32_t style = DAT_005d7c40_settings()->field_0x30;
        return style == 1 || style == 0;
    }

    /// Shows a one line system string, converted for the new text style.
    void showSystemMessage(int offset, int frames, const void* color) {
        if (isOldTextStyle()) {
            FUN_00477550_showMessage(-1, 0x3a, -1, 1, frames, color, getSystemString(offset));
            return;
        }
        std::array<uint8_t, 0x400> line{};
        FUN_004e42c0_convertString(line.data(), getSystemString(offset));
        FUN_00477550_showMessage(-1, 0x3a, -1, 1, frames, color, line.data());
    }

    /**
    * @brief The text style of the character's name.
    * Original function: FUN_0046dda0
    */
    uint32_t FUN_0046dda0_getNameStyle(const DisaPartyCharacterInfo* character) {
        return character->field_0x6b0 != 0 ? character->field_0x6ac : DAT_005d7c40_settings()->field_0x30;
    }

    /**
    * @brief The tile of the world grid, nullptr outside or for a hole.
    * Original function: FUN_004a51b0
    */
    const mpdfileTile* FUN_004a51b0_getWorldTile(int x, int z) {
        if (x < 0 || z < 0 || x >= 64 || z >= 64) return nullptr;
        return (&FP::getData<const mpdfileTile*>(0x00673260))[z * 64 + x];
    }

    /**
    * @brief Stores whether the unit stands on a geo panel and its color.
    * Original function: FUN_004d2ad0
    */
    void FUN_004d2ad0_updateGeoPanel(DisaMapUnit& unit) {
        if (!unit.isActive) return;
        const mpdfileTile* tile = FUN_004a51b0_getWorldTile(toTile(unit.position.x), toTile(unit.position.z));
        if (tile && tile->geo_mark != 0) {
            unit.isOnGeoPanel = 1;
            unit.geoPanelColor = tile->geo_color + 1;
            return;
        }
        //one int32 write in the original
        unit.isOnGeoPanel = 0;
        unit.geoPanelColor = 0;
    }

    /**
    * @brief Refreshes the geo panels and the status icons of every unit.
    * Original function: FUN_004da320
    */
    void FUN_004da320_refreshUnits() {
        FUN_004d2960_updateGeoPanels();
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            FUN_004da100_updateStatusIcons(*DisaMapUnit::get(i));
        }
    }

    /**
    * @brief Starts the HP bar of the unit from the current HP towards newHp.
    * Original function: FUN_0046b170_setHPBarTo
    * @param frames 0 shows newHp at once.
    */
    void FUN_0046b170_setHpBarTo(DisaMapUnit& unit, int newHp, int frames) {
        const DisaPartyCharacterInfo* character = unit.character;
        unit.hpBarMax = static_cast<int32_t>(character->activeStats.hp);
        unit.hpBarFrom = static_cast<int32_t>(character->currentHP);
        unit.hpBarTo = newHp;
        unit.hpBarFrame = 0;
        unit.hpBarFrames = static_cast<int16_t>(frames);
        unit.hpBarColor = unit.alliance;
        if (frames == 0) {
            unit.hpBarFrame = 1;
            unit.hpBarFrames = 1;
        }
    }

    /**
    * @brief Moves the unit to the center of its tile and stops it.
    * Original function: FUN_0046cf10_snapToTile, which truncates the negated tile so the center is found from below.
    */
    void FUN_0046cf10_snapToTile(DisaMapUnit& unit) {
        constexpr double negativeTile = double(-1.f / 12.f);
        unit.position.x = float(6 - static_cast<int>(unit.position.x * negativeTile) * 12);
        const int z = static_cast<int>(unit.position.z * negativeTile);
        unit.moveFramesLeft = 0;
        unit.velocity.z = 0.f;
        unit.velocity.x = 0.f;
        unit.position.z = float(6 - z * 12);
    }

    /// The tile of the first base panel.
    glm::ivec2 getBasePanelTile() {
        return glm::ivec2(toTile(FP::getData<float>(fTeleportPannelX)), toTile(FP::getData<float>(fTeleportPannelZ)));
    }

    /// Glides the cursor back onto the unit, the states do it when they are cancelled.
    void moveCursorTo(const DisaMapUnit& unit, float frames) {
        DAT_00730ba0_cursor().FUN_0046cab0_translateTo(unit.position.x, unit.restPosition.y, unit.position.z, frames);
    }

    // ---- The frame ----

    /// Locals of FUN_0045c0f0 that live through the whole frame, the stack offsets are the ones after the prologue.
    struct BattleFrame {
        bool isLayerIdle = false;           ///< [esp+0x77] local_1219, FUN_00444c70(0) in versus mode
        int lineHeight = 10;                ///< [esp+0x4c] local_1244, 10 or 13 with the new text style
        int targetCount = 0;                ///< [esp+0x10] local_1280, DAT_00889a20 capped at 8
        glm::ivec2 cursorTile = { 0, 0 };   ///< [esp+0xc] local_1284 and [esp+0x38] local_1258
        /// [esp+0x2c] local_1264 and [esp+0x30] local_1260, the tile of the last unit the hover search looked at.
        glm::ivec2 hoverCheckedTile = { -1, -1 };
        bool isHandled = false;             ///< [esp+0x76] local_121a
        bool isMenuOpened = false;          ///< [esp+0x7f] local_1211
    };

    /**
    * @brief Pans the camera, finds the hovered unit and prepares the locals of the frame.
    * Part of FUN_0045c0f0, from 0x0045C0F0 to the dispatch at 0x0045C6E6.
    */
    void LAB_0045c0f0_updateCameraAndHover(BattleFrame& frame) {
        int32_t& state = DAT_00889618_battleState();
        DisaCameraInfo& camera = DAT_00730980_camera();
        DisaMapUnit& cursor = DAT_00730ba0_cursor();

        if (DAT_0065db3a_gameMode() == 5) {
            frame.isLayerIdle = FUN_00444c70_isLayerIdle(0);
            FUN_004fd640_recordVersusInput();
        }
        cursor.velocity.x = 0.f;
        cursor.velocity.z = 0.f;
        FP::getData<int32_t>(0x0088c634) = 0;
        //double buffered by the frame counter
        FP::getData<uint8_t*>(0x0088c638) = &FP::getData<uint8_t>(0x0088c640) + (FP::getData<uint32_t>(0x006e62bc) & 1) * 0xf1e0;

        //the mouse drags the camera in the states that wait for a tile
        const DisaSettings* settings = DAT_005d7c40_settings();
        const uint8_t* window = FP::getData<uint8_t*>(0x005d7c70);
        constexpr std::array<int32_t, 7> mouseStates = { 0, 0x3e, 0x1e, 0x82, 0x83, 0x14, 0x16 };
        const bool isMousePan = settings->isMouseActive && window[0x305438] == 0 && window[0x30543a] != 0
            && FP::getData<uint8_t>(0x00778dca) != 0 && FP::getData<int32_t>(0x0071fd34) == 0 && DAT_0067726a_isTurningUnit() == 0
            && std::ranges::find(mouseStates, state) != mouseStates.end();

        float& dragAnchorX = FP::getData<float>(0x00701f54);
        float& dragAnchorZ = FP::getData<float>(0x00701f5c);
        if (isMousePan) {
            const glm::ivec2 mouse = FP::getData<glm::ivec2>(0x00701c80);
            int32_t& lastMouseX = FP::getData<int32_t>(0x008be594);
            int32_t& lastMouseY = FP::getData<int32_t>(0x008be590);
            if (mouse.x != lastMouseX || mouse.y != lastMouseY) {
                glm::vec3 world;
                FUN_004d1070_screenToWorld(&world, mouse.x, mouse.y, camera.eyeOffset.y, false);
                if (dragAnchorX == -1.f && dragAnchorZ == -1.f) {
                    dragAnchorX = world.x;
                    dragAnchorZ = world.z;
                }
                glm::vec2 target(camera.eyeOffset.x + dragAnchorX - world.x, camera.eyeOffset.z + dragAnchorZ - world.z);
                //every bound moves the target 6 inwards of the min side, also on the max side like the original
                const int minZ = FP::getData<int32_t>(0x00664774) * 12;
                const int minX = FP::getData<int32_t>(0x00664770) * 12;
                const int maxZ = FP::getData<int32_t>(0x0066476c) * 12;
                const int maxX = FP::getData<int32_t>(0x00664768) * 12;
                if (target.y <= float(minZ)) target.y = float(minZ + 6);
                if (target.x <= float(minX)) target.x = float(minX + 6);
                if (float(maxZ) < target.y) target.y = float(maxZ + 6);
                if (float(maxX) < target.x) target.x = float(maxX + 6);
                camera.move(glm::vec3(target.x, camera.eyeOffset.y, target.y), 10.f);
                lastMouseX = mouse.x;
                lastMouseY = mouse.y;
            }
        }
        else {
            const int32_t followedUnit = FP::getData<int32_t>(0x006772c4); //0 the cursor, else the unit index + 1, -1 none
            const bool isFollowing = DAT_0067726b_isInputConsumed() != 0 || (FP::getData<int32_t>(0x0071fd34) != 0 && state != 99)
                || state == 200 || state == 0xaa || state == 0x96 || state == 0xa0;
            if (isFollowing) {
                //the further the target, the slower, in tiles
                const float distanceX = std::abs(float(cursor.position.x - camera.eyeOffsetTarget.x) / 12.f);
                const float distanceZ = std::abs(float(cursor.position.z - camera.eyeOffsetTarget.z) / 12.f);
                int frames = static_cast<int>(distanceX > distanceZ ? distanceX : distanceZ);
                if (frames == 0) frames = 1;
                if (state != 500 && followedUnit >= 0) {
                    const float speed = float(frames * 5);
                    if (followedUnit == 0) camera.move(glm::vec3(cursor.position.x, cursor.restPosition.y, cursor.position.z), speed);
                    else camera.move(DisaMapUnit::get(followedUnit - 1)->position, speed);
                }
            }
            dragAnchorZ = -1.f;
            dragAnchorX = -1.f;
        }
        camera.processTick();

        //the first active unit on the cursor tile that is not held, so the bottom of a tower
        frame.cursorTile = glm::ivec2(toTile(cursor.position.x), toTile(cursor.position.z));
        DAT_008895ac_hoveredUnit() = -1;
        if (FUN_00444990_getFog(0, cursor.position.x, cursor.position.z) != 0xff) {
            for (int i = 0; i < DisaMapUnit::count; ++i) {
                const DisaMapUnit& unit = *DisaMapUnit::get(i);
                if (!unit.isActive || unit.heldByIndex >= 0) continue;
                frame.hoverCheckedTile = glm::ivec2(toTile(unit.position.x), toTile(unit.position.z));
                if (frame.hoverCheckedTile == frame.cursorTile) {
                    DAT_008895ac_hoveredUnit() = i;
                    break;
                }
            }
        }

        DisaKeyStates& keys = DAT_00778d98_keyStates();
        if (keys.isKeyPressed(DisaKeyStates::GeoEffectOnOff)) FP::getData<uint32_t>(0x00664778) ^= 1;

        //requested by other functions, 1 and 2 refresh one layer, 3 both
        uint8_t& fogRefresh = FP::getData<uint8_t>(0x005d7c80);
        if (fogRefresh == 1) FUN_00444810_updateFog(0);
        else if (fogRefresh == 2) FUN_00444810_updateFog(1);
        else if (fogRefresh == 3) {
            FUN_00444810_updateFog(0);
            FUN_00444810_updateFog(1);
        }
        fogRefresh = 0;

        frame.lineHeight = FP::getData<uint8_t>(0x005d7c4e) != 0 || settings->field_0x22 != 0 ? 13 : 10;
        frame.targetCount = std::min<int>(FP::getData<int16_t>(0x00889a20), 8);
    }

    // ---- States of layer 0 up to 200, the jump table at 0x0046642C ----

    /**
    * @brief The player turn ran out, counts the units, puts a waiting party member out of the base panel and hands
    * over to the enemy phase (200) or to the end of the battle (0x2af9).
    * Part of FUN_0045c0f0 at 0x0045CE5D, state 0 jumps here when FUN_00445590 returns true.
    */
    void LAB_0045ce5d_endPlayerPhase() {
        for (const uint32_t address : { 0x00889588u, 0x0088953cu, 0x00889580u, 0x005d7c8cu, 0x00889904u, 0x00889584u }) {
            FP::getData<int32_t>(address) = 0;
        }
        FUN_004da8d0_countUnits(1);
        if (FP::getData<int16_t>(0x006b757c) == 0) { //no player unit left on the map
            FUN_004486e0_buildDeployList(0);
            if (FP::getData<int16_t>(0x008896b8) > 0) {
                const float x = FP::getData<float>(fTeleportPannelX) + 6.f;
                const float z = FP::getData<float>(fTeleportPannelZ) + 6.f;
                const int height = static_cast<int>(FUN_004d2be0_getGroundHeight(nullptr, 0, x, z));
                const int index = FUN_004d3450_spawnUnit(FP::getData<int16_t>(0x00889700), 0, x, float(height), z, false);
                DisaMapUnit& unit = *DisaMapUnit::get(index);
                unit.restPosition.y = unit.position.y;
                unit.isFromBasePanel = true;
                --FP::getData<int8_t>(0x0065db39); //party members left in the base panel
                FUN_00444810_updateFog(0);
            }
        }
        std::array<int32_t, 2>& states = DAT_00889618_battleStates();
        const bool hasEnemies = FP::getData<int16_t>(0x006b7580) != 0 || FP::getData<int16_t>(0x006b7582) != 0;
        if (FP::getData<uint8_t>(0x006b790f) == 0 && hasEnemies) states = { 200, 0x3ea };
        else states = { 0x2af9, 11000 };
        DAT_008896a0_battleStateDepths() = { 0, 0 };
        FP::getData<uint8_t>(0x006d4ade) = 1;
    }

    /**
    * @brief Turns the hovered player unit by pad or by a long press of the left button.
    * @return True when the unit was turned, which stops the cursor update of the frame.
    */
    bool turnHoveredUnit(DisaMapUnit& unit, bool isNextPagePressed) {
        DisaCameraInfo& camera = DAT_00730980_camera();
        DisaMapUnit& cursor = DAT_00730ba0_cursor();
        const bool isPadShiftHeld = (FP::getData<uint16_t>(0x00778d64) >> 15) != 0;
        if (!isNextPagePressed && isPadShiftHeld) {
            if (unit.alliance != 0) return false;
            stopCursor(cursor);
            FUN_004cf1d0_turnUnit(unit, FUN_004d0db0_getPadFacing(camera, false));
            return true;
        }
        if (!FUN_0050ff80_isLeftMouseLongPress()) {
            DAT_0067726a_isTurningUnit() = 0;
            return false;
        }
        if (unit.alliance != 0) return false;
        FUN_0050fe60_clearMouseClicks();
        stopCursor(cursor);
        FUN_004cf1d0_turnUnit(unit, FUN_004d1c90_getMouseFacing(camera, cursor, unit, false));
        DAT_0067726a_isTurningUnit() = 1;
        return true;
    }

    /**
    * @brief Cancel on an own unit undoes its last action or move. On the base panel the move is forced to be
    * undoable, so the unit goes back into the panel.
    */
    void undoHoveredUnit(DisaMapUnit& unit) {
        if (unit.alliance != 0) {
            FUN_0046aab0_playSound(4);
            return;
        }
        const glm::ivec2 tile(toTile(unit.position.x), toTile(unit.position.z));
        const glm::ivec2 basePanel(toTile(FP::getData<float>(fTeleportPannelX)), toTile(FP::getData<float>(fTeleportPannelZ)));
        bool isUndone;
        if (tile == basePanel && FP::getData<uint8_t>(0x0065db43) != 0) {
            const uint8_t moveState = unit.moveState;
            const bool isFromBasePanel = unit.isFromBasePanel;
            unit.moveState = 1;
            unit.isFromBasePanel = true;
            isUndone = FUN_004485d0_undoUnit(DAT_008895ac_hoveredUnit(), 0) != 0;
            if (!isUndone) {
                unit.moveState = moveState;
                unit.isFromBasePanel = isFromBasePanel;
            }
        }
        else {
            isUndone = FUN_004485d0_undoUnit(DAT_008895ac_hoveredUnit(), 0) != 0;
        }
        FUN_0046aab0_playSound(isUndone ? 7 : 4);
        FUN_00444810_updateFog(0);
        FUN_00444810_updateFog(1);
        DAT_0067726b_isInputConsumed() = 1;
    }

    /**
    * @brief Input on the hovered unit: turn, select, the equip shortcut and undo.
    * @return True when the input took over the cursor this frame.
    */
    bool updateHoveredUnit(BattleFrame& frame, int hovered, bool isNextPagePressed) {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        DisaMapUnit& cursor = DAT_00730ba0_cursor();
        DisaMapUnit& unit = *DisaMapUnit::get(hovered);
        const auto isClickOnUnit = [&frame](bool isClicked) {
            return isClicked && frame.cursorTile == frame.hoverCheckedTile && !FUN_0046de80_isGameFlagSet(0xd48);
        };
        bool isHandled = turnHoveredUnit(unit, isNextPagePressed);

        //select, a held or defeated unit refuses
        if (keys.isKeyPressed(DisaKeyStates::Select) || isClickOnUnit(FUN_0050f7c0_isLeftMouseClicked())) {
            DAT_0067726b_isInputConsumed() = 1;
            keys.clear();
            isHandled = true;
            stopCursor(cursor);
            if (unit.heldByIndex < 0 && FUN_0046f640_getCondition(unit.character) != 2) {
                FUN_0046aab0_playSound(1);
                DAT_008895b0_selectedUnit() = DAT_008895ac_hoveredUnit();
                if (unit.alliance == 0) {
                    DAT_008895ec_menuCursor() = 0;
                    FUN_00446d10_pushBattleState(0x0a, 0); //command menu
                }
                else {
                    FUN_004d21f0_buildMoveRangeOriginal(*DisaMapUnit::get(DAT_008895ac_hoveredUnit()), 0, 0);
                    FUN_00446d10_pushBattleState(0x16, 0); //inspect
                }
            }
            else {
                FUN_0046aab0_playSound(4);
            }
        }

        //straight into the equip screen
        if (FUN_0043dba0_isActionPressed(0x12) && unit.heldByIndex < 0 && FUN_0046f640_getCondition(unit.character) != 2) {
            if (unit.alliance == 0) {
                DAT_008895b0_selectedUnit() = hovered;
                FUN_00446d10_pushBattleState(0x0a, 0);
                FUN_0046aab0_playSound(1);
                if (DAT_0065db3a_gameMode() != 5 && unit.actionState == 0 && getSleepTurns(unit.character) == 0) {
                    FUN_00446d10_pushBattleState(0x12, 0);
                }
            }
            else {
                FUN_0046aab0_playSound(4);
            }
        }

        if (keys.isKeyPressed(DisaKeyStates::Cancel) || isClickOnUnit(FUN_0050f950_isRightMouseClicked())) {
            FUN_0050fe60_clearMouseClicks();
            keys.clear();
            isHandled = true;
            stopCursor(cursor);
            undoHoveredUnit(unit);
        }
        return isHandled;
    }

    /**
    * @brief State 0, the free cursor of the player turn: shortcuts, select, undo, deploy and the battle menu.
    * Part of FUN_0045c0f0 at 0x0045C71C.
    */
    void LAB_0045c71c_updateFreeCursor(BattleFrame& frame) {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        DisaCameraInfo& camera = DAT_00730980_camera();
        DisaMapUnit& cursor = DAT_00730ba0_cursor();

        if (DAT_006b7578_busyCount() != 0 || cursor.moveFramesLeft != 0 || FUN_00444760_countProjectiles() != 0) {
            FUN_0044b480_drawHoverInfo();
            return;
        }
        if (FUN_00445590_isVersusTimeUp()) {
            LAB_0045ce5d_endPlayerPhase();
            FUN_0044b480_drawHoverInfo();
            return;
        }
        FUN_004d9dc0_updateCameraControls(camera);
        //both overwrite stack slots of the frame locals, lineHeight and targetCount are not used by state 0
        const bool isRightLongPress = FUN_0050ffb0_isRightMouseLongPress();
        const bool isNextPagePressed = keys.isKeyPressed(DisaKeyStates::NextPage);

        //the cursor jumps, the frame ends without the hover info
        if (isNextPagePressed && isPadShiftReleased()) {
            keys.clear();
            FUN_00449780_focusNextUnit(DAT_008895ac_hoveredUnit(), 0);
            DAT_0067726b_isInputConsumed() = 1;
            return;
        }
        if (FUN_0043dba0_isActionPressed(0x0f)) {
            keys.clear();
            FUN_00449af0_focusNextPlayerUnit(DAT_008895ac_hoveredUnit());
            DAT_0067726b_isInputConsumed() = 1;
            return;
        }
        if (FUN_0043dba0_isActionPressed(0x10)) {
            keys.clear();
            FUN_00449c70_focusGate(DAT_008895ac_hoveredUnit());
            DAT_0067726b_isInputConsumed() = 1;
            return;
        }
        if (FUN_0043dba0_isActionPressed(0x11)) {
            keys.clear();
            FUN_00449db0_focusBasePanel(DAT_008895ac_hoveredUnit());
            DAT_0067726b_isInputConsumed() = 1;
            return;
        }
        if (FUN_0043dba0_isActionPressed(0x13)) {
            if (DAT_0065db3a_gameMode() == 4 || DAT_0065db3a_gameMode() == 5) {
                FUN_0046aab0_playSound(4);
            }
            else {
                FUN_0046aab0_playSound(1);
                FUN_00446d10_pushBattleState(0x5a, 0);
            }
        }

        const int hovered = DAT_008895ac_hoveredUnit();
        bool isHandled = false;
        if (hovered >= 0) {
            isHandled = updateHoveredUnit(frame, hovered, isNextPagePressed);
        }
        else if (FUN_0043dba0_isActionPressed(0x11)) {
            //never true, 0x11 already returned above
            FUN_0046aab0_playSound(4);
        }

        //confirm on the base panel lists the party members that can come out
        const glm::ivec2 tile(toTile(cursor.position.x), toTile(cursor.position.z));
        const glm::ivec2 basePanel(toTile(FP::getData<float>(fTeleportPannelX)), toTile(FP::getData<float>(fTeleportPannelZ)));
        const bool isOnBasePanel = tile == basePanel;
        if (isOnBasePanel && FP::getData<uint8_t>(0x0065db43) != 0 && (keys.isKeyPressed(DisaKeyStates::Select)
            || (FUN_0050f7c0_isLeftMouseClicked() && DAT_008895ac_hoveredUnit() == -1 && !FUN_0046de80_isGameFlagSet(0xd48)))) {
            DAT_0067726b_isInputConsumed() = 1;
            FUN_0050fe60_clearMouseClicks();
            keys.clear();
            stopCursor(cursor);
            isHandled = true;
            FUN_004486e0_buildDeployList(0);
            if (FP::getData<int16_t>(0x008896b8) > 0) {
                FUN_0046aab0_playSound(1);
                FUN_00446d10_pushBattleState(0x32, 0);
            }
            else {
                FUN_0046aab0_playSound(4);
            }
        }

        //FUN_0043dba0 leaves the action in ecx, FUN_0046a650 passes it on to the sound
        if (FUN_0043dba0_isActionPressed(0x0d)) FUN_0046a650_startBattle(0x0d);
        if (FUN_0043dba0_isActionPressed(0x0e)) FUN_0046a710_endTurn();

        //the battle menu, a right click on the base panel does not open it
        if (keys.isKeyPressed(DisaKeyStates::Menu) || (!isRightLongPress && FUN_0050f950_isRightMouseClicked() && !isOnBasePanel
            && !FUN_0046de80_isGameFlagSet(0xd48))) {
            FUN_0050fe60_clearMouseClicks();
            keys.clear();
            stopCursor(cursor);
            FUN_0046aab0_playSound(3);
            DAT_008895ec_menuCursor() = 0;
            FUN_00446d10_pushBattleState(100, 0);
        }
        else if (!isHandled) {
            FUN_004d1500_updateMapCursor(&camera, &cursor, 0, 0);
        }

        if (FUN_0050f7c0_isLeftMouseClicked() && !FUN_0050f910_isLeftMouseDown() && !FUN_0046de80_isGameFlagSet(0xd48)) {
            DAT_0067726b_isInputConsumed() = 1;
        }
        FUN_0044b480_drawHoverInfo();
    }

    /**
    * @brief State 1, only the cursor moves. Versus mode waits here for the other player.
    * Part of FUN_0045c0f0 at 0x0045CFE6.
    */
    void LAB_0045cfe6_updateCursorOnly() {
        DisaMapUnit& cursor = DAT_00730ba0_cursor();
        if (DAT_006b7578_busyCount() == 0 && cursor.moveFramesLeft == 0 && FUN_00444760_countProjectiles() == 0) {
            DisaCameraInfo& camera = DAT_00730980_camera();
            DisaKeyStates& keys = DAT_00778d98_keyStates();
            FUN_004d9dc0_updateCameraControls(camera);
            if (keys.isKeyPressed(DisaKeyStates::NextPage) && isPadShiftReleased()) {
                keys.clear();
                FUN_00449780_focusNextUnit(DAT_008895ac_hoveredUnit(), 0);
            }
            FUN_004d1500_updateMapCursor(&camera, &cursor, 0, 0);
        }
        FUN_0044b480_drawHoverInfo();
    }

    /**
    * @brief State 0x12, the equip screen of the selected unit. Takes over the new weapon and stats when it closes.
    * Part of FUN_0045c0f0 at 0x0045D07E.
    */
    void LAB_0045d07e_updateEquipScreen() {
        DisaMapUnit& unit = *DisaMapUnit::get(DAT_008895b0_selectedUnit());
        //cl 1, the upper bytes of ecx still hold the -1 of the dispatch, the function only reads cl
        if (FUN_0047f2d0_updateEquipScreen(1, unit.character) == 0) return;
        DisaPartyCharacterInfo* character = unit.character;
        unit.weaponType = static_cast<int8_t>(character->unknownData6[5]); //character byte 0x689
        unit.weaponSprite = static_cast<int8_t>(character->field_0x68b);   //character byte 0x68a
        unit.movement = character->movement;
        unit.jumpPower = static_cast<int8_t>(character->jumpingPower);
        FUN_0046b170_setHpBarTo(unit, character->currentHP, 0);
        FUN_004da100_updateStatusIcons(unit);
        FUN_00446d50_popBattleState(0);
    }

    /**
    * @brief State 0x13, the status window of the selected unit.
    * Part of FUN_0045c0f0 at 0x0045D125.
    */
    void LAB_0045d125_updateStatusScreen() {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        if (keys.isKeyPressed(DisaKeyStates::Cancel) || (FUN_0050f950_isRightMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            keys.clear();
            FUN_0046aab0_playSound(7);
            FUN_00446d50_popBattleState(0);
        }
        const int selected = DAT_008895b0_selectedUnit();
        if (selected >= 0) FUN_004b03c0_drawStatusWindow(0x20, DisaMapUnit::get(selected)->character, 10);
    }

    /// The small status window of the hovered unit, or the geo panel info of the tile without one.
    void drawHoveredStatus() {
        const int hovered = DAT_008895ac_hoveredUnit();
        if (hovered >= 0) FUN_004b03c0_drawStatusWindow(0x20, DisaMapUnit::get(hovered)->character, 0x68);
        else FUN_00447f00_drawTileInfo();
    }

    /**
    * @brief Starts the move to the cursor tile. Distance 0 on another tile is the jump out of the base panel.
    * @param distance Steps from the range map, 0xff not reachable.
    */
    void moveToCursor(DisaMapUnit& unit, glm::ivec2 target, int distance) {
        unit.moveOriginFacing = unit.facing;
        unit.moveOrigin = unit.position;
        unit.pathLength = static_cast<int16_t>(distance);
        if (distance != 0) {
            FUN_0046aab0_playSound(1);
            if (FUN_004d1e50_buildPathOriginal(unit, target.x, target.y, 0) != 0) return;
            FP::getData<int32_t>(0x006772c4) = DAT_008895b0_selectedUnit() + 1; //the camera follows the unit
            FUN_00446d10_pushBattleState(0x15, 0);
            unit.showHpBar = 0;
            unit.pathStep = 1;
        }
        else {
            unit.position.x = float(target.x * 12 + 6);
            unit.position.z = float(target.y * 12 + 6);
            unit.position.y = FUN_004d2be0_getGroundHeight(nullptr, 0, unit.position.x, unit.position.z);
            //versus fog, a hidden unit stands on the tile, the move fails and ends the turn of the unit
            if (DAT_0065db3a_gameMode() == 5 && FP::getData<uint8_t>(0x006e57b3) != 0
                && FUN_00447bf0_findBlocker(DAT_008895b0_selectedUnit(), true) != -1) {
                FUN_0046aab0_playSound(4);
                unit.position.x = unit.moveOrigin.x;
                unit.position.z = unit.moveOrigin.z;
                unit.position.y = unit.moveOrigin.y;
                unit.moveState = 2;
                unit.actionState = 2;
                showSystemMessage(0x960, 0x5a, &FP::getData<uint8_t>(0x005ae3ec));
                FUN_00446d10_pushBattleState(0, 0);
                return;
            }
            FUN_0046aab0_playSound(1);
            FUN_00446d10_pushBattleState(0x15, 0);
        }
        if (FP::getData<int32_t>(0x005d9cd8) == 0 && unit.moveState == 0) unit.moveState = 1;
    }

    /**
    * @brief State 0x14, choose the tile to move the selected unit to.
    * Part of FUN_0045c0f0 at 0x0045D193.
    */
    void LAB_0045d193_updateMoveTarget() {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        DisaCameraInfo& camera = DAT_00730980_camera();
        DisaMapUnit& cursor = DAT_00730ba0_cursor();
        FUN_004d9dc0_updateCameraControls(camera);
        FUN_004d1500_updateMapCursor(&camera, &cursor, 0, 0);

        DisaMapUnit& unit = *DisaMapUnit::get(DAT_008895b0_selectedUnit());
        const int movement = unit.movement;
        FUN_004a1eb0_drawMoveRange(movement);
        const glm::ivec2 unitTile(toTile(unit.position.x), toTile(unit.position.z));
        const glm::ivec2 target(toTile(cursor.position.x), toTile(cursor.position.z));
        //a unit that acted cannot go back into the base panel
        const bool isPanelBlocked = FP::getData<uint8_t>(0x0065db43) != 0 && target == getBasePanelTile() && unit.actionState != 0;
        //no range check, the cursor never leaves the map
        const int distance = (&FP::getData<uint8_t>(0x00664f10))[target.y * 64 + target.x];

        if (keys.isKeyPressed(DisaKeyStates::Select) || (FUN_0050f7c0_isLeftMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            DAT_0067726b_isInputConsumed() = 1;
            keys.clear();
            stopCursor(cursor);
            if (distance > movement || target == unitTile || isPanelBlocked) FUN_0046aab0_playSound(4);
            else moveToCursor(unit, target, distance);
        }

        if (keys.isKeyPressed(DisaKeyStates::Cancel) || (FUN_0050f950_isRightMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            FUN_0050fe60_clearMouseClicks();
            keys.clear();
            stopCursor(cursor);
            FUN_0046aab0_playSound(7);
            FUN_00446d50_popBattleState(0);
            moveCursorTo(unit, 15.f);
        }
        if (DAT_008895ac_hoveredUnit() >= 0) FUN_0046a930_drawNamePlate(0xa9, *DisaMapUnit::get(DAT_008895ac_hoveredUnit()), 0);
        FUN_00447f00_drawTileInfo();
    }

    /// The unit stepped onto the Item World gate, shows the floor message and leaves the floor.
    void enterGate(const DisaMapUnit& unit) {
        //every 10th floor shows no message
        if (FP::getData<uint16_t>(0x0065db2c) % 10 != 9) {
            std::array<uint8_t, 0x100> line1{};
            const char* name = unit.character->charName.data();
            const int style = static_cast<int>(FUN_0046dda0_getNameStyle(unit.character));
            int namePosition = 0;
            const MessageNames names{ &name, &namePosition, &style, 1 };
            const void* color = &FP::getData<uint8_t>(0x005ad538);
            if (isOldTextStyle()) {
                FUN_004dd950_formatName(getSystemString(0x8b8), line1.data(), name, &namePosition, style);
                FUN_00477550_showMessage(-1, 0x3a, -1, 2, 0x96, color, line1.data(), getSystemString(0x8bc), names);
            }
            else {
                std::array<uint8_t, 0x540> format{};
                std::array<uint8_t, 0x400> line2{};
                FUN_004e3440_convertString(format.data(), getSystemString(0x8b8));
                FUN_004dd950_formatName(format.data(), line1.data(), name, &namePosition,
                    static_cast<int>(FUN_0046dda0_getNameStyle(unit.character)));
                FUN_004e42c0_convertString(line2.data(), getSystemString(0x8bc));
                FUN_00477550_showMessage(-1, 0x3a, -1, 2, 0x96, color, line1.data(), line2.data(), names);
            }
        }
        FUN_0046aab0_playSound(0x1f);
        DAT_008896a0_battleStateDepths()[0] = 0;
        DAT_00889618_battleState() = 0x3e9;
    }

    /**
    * @brief State 0x15, the selected unit walks its path. Then the base panel takes it back, the gate ends the
    * Item World floor and an exit tile ends the map.
    * Part of FUN_0045c0f0 at 0x0045D602.
    */
    void LAB_0045d602_updateWalk() {
        const int selected = DAT_008895b0_selectedUnit();
        const int walkResult = FUN_004475c0_updateWalk(selected);
        DisaMapUnit& unit = *DisaMapUnit::get(selected);

        //versus fog, the unit ran into a hidden unit and stops on the last tile
        if (DAT_0065db3a_gameMode() == 5 && FP::getData<uint8_t>(0x006e57b3) != 0 && unit.movement < 99
            && FUN_00447bf0_findBlocker(selected, false) != -1) {
            FUN_0046cf10_snapToTile(unit);
            moveCursorTo(unit, 0.f);
            unit.moveState = 2;
            unit.actionState = 2;
            unit.pathLength = 0;
            unit.pathX[0] = static_cast<int16_t>(toTile(unit.position.x));
            unit.pathZ[0] = static_cast<int16_t>(toTile(unit.position.z));
            DisaTransform* transform = getEffectTransform();
            transform->position.x = unit.position.x;
            transform->position.y = float(double(unit.position.y) - 12.0);
            transform->position.z = unit.position.z;
            FUN_004783d0_spawnEffect(0x139d, 0x1389, transform);
        }
        else if (walkResult != 1) {
            return;
        }
        if (unit.moveFramesLeft != 0) return;

        unit.showHpBar = 1;
        unit.pose = 0;
        FUN_004d2ad0_updateGeoPanel(unit);
        FUN_004da320_refreshUnits();
        DAT_008896a0_battleStateDepths()[0] = 0;
        DAT_00889618_battleState() = 0;
        FP::getData<int32_t>(0x006772c4) = 0; //the camera follows the cursor again

        const glm::ivec2 tile(toTile(unit.position.x), toTile(unit.position.z));
        if (getBasePanelTile() == tile && FP::getData<uint8_t>(0x0065db43) != 0) {
            //back into the base panel, the party member keeps its states for the next time it comes out
            reinterpret_cast<uint8_t*>(unit.character)[0x69c] = unit.actionState;
            reinterpret_cast<uint8_t*>(unit.character)[0x69d] = unit.moveState;
            unit.isActive = false;
            ++FP::getData<int8_t>(0x0065db39);
        }
        const glm::ivec2 gateTile(toTile(FP::getData<float>(0x0065dae0)), toTile(FP::getData<float>(0x0065dae4)));
        if (unit.alliance == 0 && gateTile == tile && FP::getData<uint8_t>(0x0065db41) != 0) enterGate(unit);

        //an exit tile of the first chunk ends the map
        const auto& eventTiles = DisaMapChunk::get(0)->eventTiles;
        for (const mpdfileChunkEventTile& eventTile : eventTiles) {
            const bool isOnTile = static_cast<uint8_t>(eventTile.x) == tile.x && static_cast<uint8_t>(eventTile.z) == tile.y;
            if (eventTile.index != 0 && isOnTile && unit.alliance == 0 && eventTile.index == 2) {
                FUN_00446e20(eventTile.index, tile.y);
            }
        }
        FUN_00444810_updateFog(0);
    }

    /**
    * @brief State 0x16, looks at a unit the player does not control, with its move range.
    * Part of FUN_0045c0f0 at 0x0045DA48.
    */
    void LAB_0045da48_updateInspect() {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        DisaCameraInfo& camera = DAT_00730980_camera();
        DisaMapUnit& cursor = DAT_00730ba0_cursor();
        FUN_004d9dc0_updateCameraControls(camera);
        FUN_004d1500_updateMapCursor(&camera, &cursor, 0, 0);

        const DisaMapUnit& unit = *DisaMapUnit::get(DAT_008895b0_selectedUnit());
        //geo symbols, characters 3200 to 3209 and models have no move range
        if (unit.isGeoSymbol == 0 && unit.characterId / 10 != 320 && unit.modelType == 0) FUN_004a1eb0_drawMoveRange(unit.movement);
        if (keys.isKeyPressed(DisaKeyStates::Cancel) || (FUN_0050f950_isRightMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            DAT_0067726b_isInputConsumed() = 1;
            keys.clear();
            stopCursor(cursor);
            FUN_0046aab0_playSound(7);
            FUN_00446d50_popBattleState(0);
            moveCursorTo(unit, 15.f);
        }
        drawHoveredStatus();
    }

    /**
    * @brief State 0x17, watches the unit DAT_008895b4 with its move range until it moves, falls or Cancel.
    * Part of FUN_0045c0f0 at 0x0045DB64.
    */
    void LAB_0045db64_updateWatchUnit() {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        DisaCameraInfo& camera = DAT_00730980_camera();
        DisaMapUnit& cursor = DAT_00730ba0_cursor();
        const bool isIdle = DAT_006b7578_busyCount() == 0 && cursor.moveFramesLeft == 0 && FUN_00444760_countProjectiles() == 0;
        if (isIdle) {
            FUN_004d9dc0_updateCameraControls(camera);
            FUN_004d1500_updateMapCursor(&camera, &cursor, 0, 0);
        }

        const DisaMapUnit& unit = *DisaMapUnit::get(FP::getData<int32_t>(0x008895b4));
        const glm::vec2 watchedPosition = FP::getData<glm::vec2>(0x008895e4); //x and z
        const bool isStill = unit.isActive && FUN_0046f640_getCondition(unit.character) != 2 && unit.heldByIndex < 0
            && unit.position.x == watchedPosition.x && unit.position.z == watchedPosition.y;
        if (!isStill) {
            stopCursor(cursor);
            FUN_00446d50_popBattleState(0);
            moveCursorTo(unit, 15.f);
        }
        else {
            if (unit.isGeoSymbol == 0 && unit.characterId / 10 != 320 && unit.modelType == 0) FUN_004a1eb0_drawMoveRange(unit.movement);
            if (isIdle && keys.isKeyPressed(DisaKeyStates::Cancel)) {
                keys.clear();
                stopCursor(cursor);
                FUN_0046aab0_playSound(7);
                FUN_00446d50_popBattleState(0);
                moveCursorTo(unit, 15.f);
            }
        }
        drawHoveredStatus();
    }

    /// State 0x0A, the command menu. Part of FUN_0045c0f0 at 0x0045D06E.
    void LAB_0045d06e_updateCommandMenu() {
        FP::callStdFastFarA<void>(0x004502d0, 0); //FUN_004502d0(layer)
    }

    /// State 0x28, the battle start. Part of FUN_0045c0f0 at 0x0045E546.
    void LAB_0045e546_updateBattleStart() {
        if (FUN_00466970_updateBattleStart(0) != 1) return;
        DAT_00889618_battleStates() = { 0xa0, 0xa0 };
        DAT_008896a0_battleStateDepths() = { 0, 0 };
    }

    /// State 0x3C, choose the special. Part of FUN_0045c0f0 at 0x0045E57D.
    void LAB_0045e57d_updateSpecialList() {
        FP::callStdFastFarA<void>(0x0044a3b0, 0); //FUN_0044a3b0(layer)
    }

    /// State 0x3D. Part of FUN_0045c0f0 at 0x0045E58D.
    void LAB_0045e58d_updateState3d() {
        FP::callStdFastFarA<void>(0x0044b150, 0); //FUN_0044b150(layer)
    }

    /// State 0x46, the item list. Part of FUN_0045c0f0 at 0x004602D8.
    void LAB_004602d8_updateItemList() {
        FP::callFarA<void>(0x0044e030);
    }

    /// State 0x4C, the versus item list. Part of FUN_0045c0f0 at 0x00461AD0.
    void LAB_00461ad0_updateVersusItemList() {
        FP::callStdFastFarA<void>(0x0044ec50, 0); //FUN_0044ec50(layer)
    }

    /// State 0x4D. Part of FUN_0045c0f0 at 0x00461AE0.
    void LAB_00461ae0_updateState4d() {
        FP::callStdFastFarA<void>(0x0044eec0, 0); //FUN_0044eec0(layer)
    }

    /// State 0x8C. Part of FUN_0045c0f0 at 0x00463C6E.
    void LAB_00463c6e_updateState8c() {
        if (FP::callFarA<int>(0x00448ab0) == 0) return;
        DAT_00889618_battleState() = 0;
        DAT_008896a0_battleStateDepths()[0] = 0;
    }

    /// State 0x96, the start of the enemy phase. Part of FUN_0045c0f0 at 0x00463C8E.
    void LAB_00463c8e_updateEnemyPhaseStart() {
        if (FP::callFarA<int>(0x00446090) == 0) return;
        DAT_00889618_battleStates() = { 0xa0, 0xa0 };
        DAT_008896a0_battleStateDepths() = { 0, 0 };
    }

    /// State 0xA0, the turn change. Part of FUN_0045c0f0 at 0x00463CC2.
    void LAB_00463cc2_updateTurnChange() {
        if (FP::callFarA<int>(0x004455c0) != 0) {
            DAT_008896a0_battleStateDepths()[0] = 0;
            //versus mode continues with the second player
            DAT_00889618_battleState() = FP::getData<uint8_t>(0x006d4ab2) != 0 ? 1 : 0;
        }
        FUN_0044b480_drawHoverInfo();
    }

    /// State 0x5A. Part of FUN_0045c0f0 at 0x00463D00.
    void LAB_00463d00_updateState5a() {
        //ecx still holds the -1 of the dispatch, the function never reads it
        FP::callStdFastFarA<void>(0x00453110, -1);
    }

    /// State 0x65. Part of FUN_0045c0f0 at 0x00463D0E.
    void LAB_00463d0e_updateState65() {
        if (FP::callFarA<int>(0x00480630) != 0) FUN_00446d50_popBattleState(0);
    }

    /// State 0xAA. Part of FUN_0045c0f0 at 0x00463D2B.
    void LAB_00463d2b_updateStateAa() {
        FP::callStdFastFarA<void>(0x00453620, 0); //FUN_00453620(layer)
    }

    /// State 200. Part of FUN_0045c0f0 at 0x00463D3B.
    void LAB_00463d3b_updateState200() {
        FP::callFarA<void>(0x0044cb50);
    }

    /// State 0x32, the base panel deployment list. Part of FUN_0045c0f0 at 0x00463D49.
    void LAB_00463d49_updateDeployList() {
        FP::callStdFastFarA<void>(0x00452990, 0); //FUN_00452990(layer)
    }

    /// State 0x64, the battle menu. Part of FUN_0045c0f0 at 0x00463D59.
    void LAB_00463d59_updateBattleMenu() {
        FP::callStdFastFarA<void>(0x00451d70, 0); //FUN_00451d70(layer)
    }

    /// State 0x63. Part of FUN_0045c0f0 at 0x00463D69.
    void LAB_00463d69_updateState63() {
        const int result = FP::callStdFastFarA<int>(0x004b4900, 0); //FUN_004b4900(layer)
        if (result == 1) {
            FUN_00446d50_popBattleState(0); //jumps to 0x0045D119 in state 0x12
        }
        else if (result == 2) {
            FP::callStdFastFarA<void>(0x00444a80, 2); //FUN_00444a80
            FUN_00477550_showMessage(-1, 0x3a, -1, 1, 0x5a, &FP::getData<uint8_t>(0x005ad538), getSystemString(0x358));
        }
    }

    /// State 300, the deployment before the battle. Part of FUN_0045c0f0 at 0x00463DC4.
    void LAB_00463dc4_updateDeployment() {
        if (FUN_00466970_updateBattleStart(0) != 1) return;
        DAT_008896a0_battleStateDepths()[0] = 0;
        DAT_00889618_battleState() = 0;
        //the deployed units leave the base panel
        const int16_t count = FP::getData<int16_t>(0x00889a24);
        const int16_t* deployed = &FP::getData<int16_t>(0x0088a3b0);
        for (const int16_t index : std::span(deployed, std::max<int16_t>(count, 0))) {
            DisaMapUnit::get(index)->isActive = false;
        }
    }

    /// The states above 300. Part of FUN_0045c0f0 at 0x00463E1F.
    void LAB_00463e1f_updateHighState(BattleFrame& frame) {
        //TODO port
    }

    /// Runs the state of layer 0. Part of FUN_0045c0f0 at 0x0045C6E6.
    void LAB_0045c6e6_updateBattleState(BattleFrame& frame) {
        const int32_t state = DAT_00889618_battleState();
        if (state > 300) {
            LAB_00463e1f_updateHighState(frame);
            return;
        }
        switch (state) {
        case 0x00: LAB_0045c71c_updateFreeCursor(frame); break;
        case 0x01: LAB_0045cfe6_updateCursorOnly(); break;
        case 0x12: LAB_0045d07e_updateEquipScreen(); break;
        case 0x13: LAB_0045d125_updateStatusScreen(); break;
        case 0x14: LAB_0045d193_updateMoveTarget(); break;
        case 0x15: LAB_0045d602_updateWalk(); break;
        case 0x16: LAB_0045da48_updateInspect(); break;
        case 0x17: LAB_0045db64_updateWatchUnit(); break;
        case 0x0a: LAB_0045d06e_updateCommandMenu(); break;
        case 0x28: LAB_0045e546_updateBattleStart(); break;
        case 0x3c: LAB_0045e57d_updateSpecialList(); break;
        case 0x3d: LAB_0045e58d_updateState3d(); break;
        case 0x46: LAB_004602d8_updateItemList(); break;
        case 0x4c: LAB_00461ad0_updateVersusItemList(); break;
        case 0x4d: LAB_00461ae0_updateState4d(); break;
        case 0x8c: LAB_00463c6e_updateState8c(); break;
        case 0x96: LAB_00463c8e_updateEnemyPhaseStart(); break;
        case 0xa0: LAB_00463cc2_updateTurnChange(); break;
        case 0x5a: LAB_00463d00_updateState5a(); break;
        case 0x65: LAB_00463d0e_updateState65(); break;
        case 0xaa: LAB_00463d2b_updateStateAa(); break;
        case 200: LAB_00463d3b_updateState200(); break;
        case 0x32: LAB_00463d49_updateDeployList(); break;
        case 0x64: LAB_00463d59_updateBattleMenu(); break;
        case 0x63: LAB_00463d69_updateState63(); break;
        case 300: LAB_00463dc4_updateDeployment(); break;
        default: break; //0x00463C65
        }
    }

    /**
    * @brief Draws the map, the cursors, the units and the projectiles.
    * Part of FUN_0045c0f0, from LAB_00465035 to the end.
    */
    void LAB_00465035_drawBattleMap(BattleFrame& frame) {
        //TODO port
    }
}

void FUN_0045c0f0_updateBattleMap() {
    BattleFrame frame;
    LAB_0045c0f0_updateCameraAndHover(frame);
    LAB_0045c6e6_updateBattleState(frame);
    LAB_00465035_drawBattleMap(frame);
}
