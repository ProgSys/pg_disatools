#include "DisaMapGameFunctions.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>

#include <glm/glm.hpp>

#include <FunctionPatcher.h>
#include <DisaFunctions.h>
#include <DisaMapFunctions.h>
#include <DisaCharacterFunctions.h>
#include <DisaMapChunk.h>
#include <DisaMath.h>
#include <DisaRendering.h>
#include <DisaScriptFunctions.h>
#include <DisaScriptManager.h>
#include <Disa/fileformats/PG_MPD.h>

/*
* Port of FUN_0045c0f0. Globals keep their address in the name (DAT_xxxxxxxx_name), parts of the original function
* are named after the address of their first instruction (LAB_xxxxxxxx_name) and callees after their own address
* (FUN_xxxxxxxx_name), so all of them can be found again in Ghidra, the disassembly and memory dumps.
*/
static_assert(offsetof(PG_DisaPartyCharacter, currentHP) == 0x5c4);
static_assert(offsetof(PG_DisaPartyCharacter, currentSP) == 0x5c8);
static_assert(offsetof(PG_DisaPartyCharacter, activeStats) == 0x5cc);
static_assert(offsetof(PG_DisaPartyCharacter, activeStats.intelligence) == 0x5dc);
static_assert(offsetof(PG_DisaPartyCharacter, skillId) == 0x4a4);
static_assert(offsetof(PG_DisaPartyCharacter, characterId) == 0x664);
static_assert(offsetof(PG_DisaPartyCharacter, level) == 0x660);
static_assert(offsetof(PG_DisaPartyCharacter, skillLevel) == 0x564);
static_assert(offsetof(mpdfileTile, mobility) == 0x77);
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

    /// DAT_0088951c, the row of the cursor in the visible part of a list.
    int32_t& DAT_0088951c_listCursor() { return FP::getData<int32_t>(0x0088951c); }
    /// DAT_00889520, the first visible row of a list.
    int32_t& DAT_00889520_listScroll() { return FP::getData<int32_t>(0x00889520); }
    /// DAT_00889a20, the number of attack targets the command menu found.
    int16_t& DAT_00889a20_attackTargetCount() { return FP::getData<int16_t>(0x00889a20); }
    /// DAT_008899a0, the unit indices of the attack targets, enemies first.
    int16_t* DAT_008899a0_attackTargets() { return &FP::getData<int16_t>(0x008899a0); }
    /// DAT_005d9cd0, the number of queued actions in DAT_005db920, run when the turn is executed.
    int32_t& DAT_005d9cd0_actionCount() { return FP::getData<int32_t>(0x005d9cd0); }
    /// DAT_005db920, the queued actions of 0x388 bytes.
    uint8_t* DAT_005db920_getAction(int index) { return &FP::getData<uint8_t>(0x005db920) + index * 0x388; }
    /// DAT_00701cd0, the marker over the chosen target, a sprite actor like the cursor.
    DisaMapUnit& DAT_00701cd0_targetMarker() { return FP::getData<DisaMapUnit>(0x00701cd0); }
    /// DAT_00701f3b, shows the target marker.
    uint8_t& DAT_00701f3b_isTargetMarkerShown() { return FP::getData<uint8_t>(0x00701f3b); }

    /// DAT_0065db3a, 0 and 2 leave to the castle, 1 a dungeon that gets harder, 3 Item World, 4 leaves to scene 0x33, 5 versus.
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
    /**
    * @brief DAT_00778a10, the map cursor of layer 1 in versus mode.
    * Only the sprite actor part is valid.
    */
    DisaMapUnit& DAT_00778a10_cursorB() { return FP::getData<DisaMapUnit>(0x00778a10); }
    /// DAT_006d4ab2, versus mode: the layer whose turn it is.
    uint8_t& DAT_006d4ab2_turnLayer() { return FP::getData<uint8_t>(0x006d4ab2); }

    /**
    * @brief A flying projectile or explosion. FUN_00476d90 spawns it, FUN_0046ac00 draws it.
    * The first 0x60 bytes are a sprite actor (sprite 1000).
    * Size: 0xf0
    */
    struct DisaProjectile {
        uint8_t field0x00[0x60];   // 0x00
        glm::vec3 position;        // 0x60
        uint8_t field0x6c[0x18];   // 0x6c
        float hitRadiusSquared;    // 0x84
        glm::vec3 target;          // 0x88 - the aim point, state 0x50 spreads it around the hovered unit
        int32_t power;             // 0x94 - type 10 deals it as it is
        int32_t field0x98;         // 0x98 - -1 on spawn
        int16_t isActive;          // 0x9c
        uint8_t field0x9e[2];      // 0x9e
        int16_t owner;             // 0xa0 - unit index, or 200 + alliance, which spares that alliance
        int16_t direction;         // 0xa2
        int16_t type;              // 0xa4 - 10 explosion, 2 bursts on a hit
        int16_t delay;             // 0xa6 - frames before it can hit
        int16_t field0xa8;         // 0xa8 - cleared on a hit
        int16_t lifetime;          // 0xaa - frames, 0 flies until it leaves the map
        uint8_t field0xac[0x44];   // 0xac
    };
    static_assert(sizeof(DisaProjectile) == 0xf0);
    static_assert(offsetof(DisaProjectile, isActive) == 0x9c);
    static_assert(offsetof(DisaProjectile, lifetime) == 0xaa);

    /// DAT_00730f10, the 128 projectiles.
    std::span<DisaProjectile, 128> DAT_00730f10_projectiles() {
        return std::span<DisaProjectile, 128>(&FP::getData<DisaProjectile>(0x00730f10), 128);
    }


    /// DAT_006e80f0 + 0x1740, scratch transform for effects.
    DisaTransform* getEffectTransform() {
        return reinterpret_cast<DisaTransform*>(FP::getData<uint8_t*>(0x006e80f0) + 0x1740);
    }

    /// DAT_006e80f0 + 0x1770, scratch character the forecasts are computed into.
    DisaPartyCharacterInfo* getScratchCharacter() {
        return reinterpret_cast<DisaPartyCharacterInfo*>(FP::getData<uint8_t*>(0x006e80f0) + 0x1770);
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
    */
    int FUN_00444760_countProjectiles() {
        return static_cast<int>(std::ranges::count_if(DAT_00730f10_projectiles(), [](const DisaProjectile& projectile) {
            return projectile.isActive != 0;
        }));
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
        const DisaMapUnit& cursor = layer == 1 ? DAT_00778a10_cursorB() : DAT_00730ba0_cursor();
        const int32_t state = DAT_00889618_battleStates()[layer];
        const bool isBusy = DAT_006b7578_busyCount() != 0;
        //the original compares moveFramesLeft with the busy count, which is 0 by then
        const auto isMoving = [&]() { return isBusy || cursor.moveFramesLeft != 0 || FUN_00444760_countProjectiles() != 0; };
        if ((state == 0 || state == 1 || state == 0x17) && isMoving()) return false;
        constexpr std::array<int32_t, 13> busyStates = {
            0x15, 0x28, 0x87, 200, 0x96, 0xa0, 0x3e9, 0x4b0, 0x4b1, 0x4b2, 11000, 0x2af9, 0x3ea };
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
    void FUN_00449780_focusNextUnit(int hovered, int8_t layer) { FP::callStdFastFarA<void>(0x00449780, hovered, layer); }
    /// Original function: FUN_00449af0, moves the cursor to the next player unit.
    void FUN_00449af0_focusNextPlayerUnit(int hovered) { FP::callStdFastFarA<void>(0x00449af0, hovered); }
    /// Original function: FUN_00449c70, moves the cursor to the Item World gate.
    void FUN_00449c70_focusGate(int hovered) { FP::callStdFastFarA<void>(0x00449c70, hovered); }
    /// Original function: FUN_00449db0, moves the cursor to the base panel.
    void FUN_00449db0_focusBasePanel(int hovered) { FP::callStdFastFarA<void>(0x00449db0, hovered); }
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
    void FUN_004da100_updateStatusIcons(DisaMapUnit& unit) { FP::callStdFastFarA<void>(0x004da100, &unit); }
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

    /// Original function: FUN_0050fb90, the mouse over a row of a list moves the cursor of the list.
    void FUN_0050fb90_hoverList(int x, int y, int width, int rowHeight, int rows, int32_t* cursor, bool flag) {
        FP::callFastFarA<void>(0x0050fb90, x, y, width, rowHeight, rows, cursor, int(flag));
    }
    /// Original function: FUN_0050fc10, the mouse wheel and the scroll arrows of a list.
    void FUN_0050fc10_scrollList(int x, int y, int width, int rowHeight, int rows, int32_t* cursor, int scroll, int count) {
        FP::callFastFarA<void>(0x0050fc10, x, y, width, rowHeight, rows, cursor, scroll, count);
    }
    /// Original function: FUN_004d0d60, moves the cursor of a list by key and scrolls it, isPlayingSound plays sound 3.
    void FUN_004d0d60_updateListCursor(int32_t* cursor, int32_t* scroll, int count, int rows, bool isPlayingSound) {
        FP::callFastFarA<void>(0x004d0d60, cursor, scroll, count, rows, int(isPlayingSound));
    }
    /// Original function: FUN_00454660, draws the list of attack targets. The two middle stack slots are never read.
    void FUN_00454660_drawTargetList(DisaMapUnit& unit, int count) {
        FP::callVariadicThisFarA<void>(0x00454660, &unit, 0, 0, count);
    }
    /// Original function: FUN_004a1c60, marks a tile of the range overlay, mode 3 is the attack range.
    void FUN_004a1c60_markTile(int x, int z, int mode) { FP::callFastFarA<void>(0x004a1c60, x, z, mode); }
    /// Original function: FUN_00447d90, the damage forecast of the attack, written into the scratch character.
    int FUN_00447d90_forecastAttack(DisaMapUnit& unit, DisaPartyCharacterInfo* scratch, int facing) {
        return FP::callFastFarA<int>(0x00447d90, &unit, scratch, facing);
    }
    /// Original function: FUN_00454610, draws the forecast against an adjacent target.
    void FUN_00454610_drawForecast(DisaMapUnit& unit, DisaMapUnit& target, DisaPartyCharacterInfo* scratch, int forecast) {
        FP::callCdeclFarA<void>(0x00454610, &unit, &target, scratch, forecast);
    }
    /// Original function: FUN_004d2700, the facing that looks from the second position at the first one.
    int FUN_004d2700_getFacing(int x, int z, int fromX, int fromZ) {
        return FP::callFastFarA<int>(0x004d2700, x, z, fromX, fromZ);
    }
    /// Original function: FUN_0046e1a0, a random number below n.
    int FUN_0046e1a0_random(int n) { return static_cast<int>(FP::callStdFastFarA<uint64_t>(0x0046e1a0, n)); }
    /// Original function: FUN_0046f510, clears a queued action of DAT_005db920.
    void FUN_0046f510_clearAction(uint8_t* action) { FP::callStdFastFarA<void>(0x0046f510, action); }
    /// Original function: FUN_00470850, the SP cost of a skill for the character.
    int FUN_00470850_getSkillCost(DisaPartyCharacterInfo* character, int skill, int mode) {
        return FP::callFastFarA<int>(0x00470850, character, skill, mode);
    }

    /**
    * @brief The data of a skill, entries of 0xac bytes in the table at DAT_0066402c, nullptr when unknown.
    * Original function: FUN_0046f4a0
    */
    const uint16_t* FUN_0046f4a0_findSkill(uint32_t id) {
        const uint16_t* table = FP::getData<const uint16_t*>(0x0066402c);
        const int count = FP::getData<int32_t>(0x00664030);
        for (int i = 0; i < count; ++i) {
            if (table[i * 0x56] == id) return table + i * 0x56;
        }
        return nullptr;
    }

    /**
    * @brief Height of the tile surface, negative is up, 0 outside the grid.
    * Original function: FUN_004d2e00
    */
    int FUN_004d2e00_getTileHeight(int x, int z) {
        if (x > 63 || z > 63 || x < 0 || z < 0) return 0;
        return (&FP::getData<int16_t>(0x00671250))[z * 64 + x];
    }

    /**
    * @brief Steps of the move range of the layer to a tile, 0xff outside the grid or not reached.
    * Original function: FUN_004495a0
    */
    int FUN_004495a0_getRangeDistance(int x, int z, int8_t layer) {
        if (x < 0 || z < 0 || x >= 64 || z >= 64) return 0xff;
        return (&FP::getData<uint8_t>(0x00666f10))[x + (layer * 64 + z) * 64];
    }

    /**
    * @brief Rounds a height to the nearest whole number, halves away from zero.
    * Original function: FUN_0046e7d0
    */
    int FUN_0046e7d0_roundHeight(float value) {
        const int whole = static_cast<int>(value);
        const float fraction = value - float(whole);
        return whole + (fraction >= 0.5f ? 1 : 0) - (fraction <= -0.5f ? 1 : 0);
    }

    /**
    * @brief The tile of an entry of a skill area, turned to the direction of the area and moved to the center.
    * Original function: FUN_004446b0, a thiscall on the area table at 0x00664780, 3 bytes per entry: x, z and the kind.
    * @return The kind of the tile: 1 hits units, 2 must be free, 3 hits units within the height window.
    */
    int FUN_004446b0_getAreaTile(int index, int direction, glm::ivec2 center, glm::ivec2& tile) {
        const int8_t* entry = &FP::getData<int8_t>(0x00664780) + index * 3;
        //another direction leaves the tile as it was before the center is added
        switch (direction) {
        case 0: tile = glm::ivec2(entry[0], entry[1]); break;
        case 1: tile = glm::ivec2(entry[1], -entry[0]); break;
        case 2: tile = glm::ivec2(-entry[0], -entry[1]); break;
        case 3: tile = glm::ivec2(-entry[1], entry[0]); break;
        default: break;
        }
        tile += center;
        return entry[2];
    }

    /**
    * @brief The middle mouse button went down.
    * Original function: FUN_0050fad0
    */
    bool FUN_0050fad0_isMiddleMouseClicked() {
        return isMouseUsable() && (FP::getData<uint32_t>(0x00701c58) & 0x40000000) != 0;
    }

    /// Original function: FUN_0050fe30, a left click inside the box.
    bool FUN_0050fe30_isClickInBox(int left, int top, int right, int bottom) {
        return FP::callFastFarA<bool>(0x0050fe30, left, top, right, bottom, 0);
    }
    /// Original function: FUN_004d0ee0, the facing the stick points to, -1 none. The stack slot is never read.
    int FUN_004d0ee0_getStickFacing(DisaCameraInfo& camera) { return FP::callFastFarA<int>(0x004d0ee0, &camera, 0, 0); }
    /// Original function: FUN_004a1be0, draws the area of the skill at the cursor tile.
    void FUN_004a1be0_drawSkillArea(int area, int direction, glm::ivec2 center, int distance) {
        FP::callFastFarA<void>(0x004a1be0, area, direction, center.x, center.y, distance);
    }
    /// Original function: FUN_00456930, the window with the reasons a skill cannot be used and the rotate button.
    void FUN_00456930_drawSkillWarnings(int errorCount, int blocked, int outOfReach, int inReach, int heightTiles, int heightFree,
        int isOutOfRange, int area, DisaMapUnit& unit) {
        FP::callFastFarA<void>(0x00456930, errorCount, blocked, outOfReach, inReach, heightTiles, heightFree, isOutOfRange, area,
            &unit);
    }
    /// Original function: FUN_004572c0, the window with the reasons the summon cannot be placed.
    void FUN_004572c0_drawSummonWarnings(int isOutOfRange, int blocked) {
        FP::callStdFastFarA<void>(0x004572c0, isOutOfRange, blocked);
    }
    /// Original function: FUN_0046a9f0, the height window of the cursor tile at x.
    void FUN_0046a9f0_drawTileHeight(int x) { FP::callStdFastFarA<void>(0x0046a9f0, x); }
    /**
    * @brief Original function: FUN_004d30e0, places a character as a unit of the alliance, returns its index or -1.
    * The eighth argument is never read, the original passes whatever ecx held.
    */
    int FUN_004d30e0_spawnCharacter(int id, int alliance, int level, float x, float y, float z) {
        return FP::callFastFarA<int>(0x004d30e0, id, alliance, level, x, y, z, 0, 0, 0);
    }
    /// Original function: FUN_00478d50, starts an effect at the transform.
    void FUN_00478d50_spawnEffect(int sprite, int animation, DisaTransform* transform, float scale, int arg9) {
        FP::callFastFarA<void>(0x00478d50, sprite, animation, transform, &DAT_00730980_camera(), 0, scale, 0, 0, arg9);
    }
    /// Original function: FUN_0046db50, the character used the skill, counts its experience.
    void FUN_0046db50_useSkill(DisaPartyCharacterInfo* character, int skill) {
        FP::callStdFastFarA<void>(0x0046db50, character, skill);
    }
    /// Original function: FUN_004db610, recomputes the stats of the character.
    void FUN_004db610_updateStats(DisaPartyCharacterInfo* character, int alliance) {
        FP::callStdFastFarA<void>(0x004db610, character, alliance);
    }

    /// Like the original, the bounds move the position 6 inside the min side, and also 6 outside the max side.
    void clampToMap(float& x, float& z) {
        const int minZ = FP::getData<int32_t>(0x00664774) * 12;
        const int minX = FP::getData<int32_t>(0x00664770) * 12;
        const int maxZ = FP::getData<int32_t>(0x0066476c) * 12;
        const int maxX = FP::getData<int32_t>(0x00664768) * 12;
        if (z <= float(minZ)) z = float(minZ + 6);
        if (x <= float(minX)) x = float(minX + 6);
        if (float(maxZ) < z) z = float(maxZ + 6);
        if (float(maxX) < x) x = float(maxX + 6);
    }

    /// Names that FUN_004dd950 placed into a message line, the message draws them in their own text style.
    struct MessageNames {
        const char* const* names = nullptr;
        const int* positions = nullptr; ///< where FUN_004dd950 put the name
        const int* lines = nullptr;     ///< the y of the line of each name, nullptr for the first line
        const int* styles = nullptr;
        int count = 0;
    };

    /**
    * @brief Original function: FUN_00477550, a message window shown for a number of frames.
    * x and y -1 center it, color points to a palette entry like 0x005AD538.
    */
    void FUN_00477550_showMessage(int x, int y, int width, int lineCount, int frames, const void* color, const void* line1,
        const void* line2 = nullptr, const MessageNames& names = {}) {
        FP::callFastFarA<void>(0x00477550, x, y, width, lineCount, frames, color, line1, line2, names.names, names.positions,
            names.lines, names.styles, names.count, 0);
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

    // ---- Globals of the states above 300 and the drawing ----

    /// DAT_008895d4, frame counter of the result screens 1000 and 0x4B0 to 0x4B2 and of the item level up, -1 until they start.
    int32_t& DAT_008895d4_resultTimer() { return FP::getData<int32_t>(0x008895d4); }
    /// DAT_00889904, the step of the states 11000 and 0x2AF9: 0 battle start, 10 geo panel chain, 11 AI turn.
    int32_t& DAT_00889904_aiStep() { return FP::getData<int32_t>(0x00889904); }
    /// DAT_006645f4, the queued geo panel changes, FUN_00446090 plays them.
    int32_t& DAT_006645f4_geoChainCount() { return FP::getData<int32_t>(0x006645f4); }
    /// DAT_006645fc, the queued geo panel change FUN_00446090 plays next.
    int32_t& DAT_006645fc_geoChainIndex() { return FP::getData<int32_t>(0x006645fc); }
    /// DAT_00889584, bonus gauge points not added yet, state 0xA0 adds them.
    int32_t& DAT_00889584_bonusPointsPending() { return FP::getData<int32_t>(0x00889584); }
    /// DAT_006772bc, levels DAT_00653fa0 gains in the level up, one per floor.
    int32_t& DAT_006772bc_itemLevels() { return FP::getData<int32_t>(0x006772bc); }
    /// DAT_00889594, frames per level of the item level up, 30 to 90, 10 once skipped.
    int32_t& DAT_00889594_framesPerLevel() { return FP::getData<int32_t>(0x00889594); }
    /// DAT_00889590, 1 after the last floor, 0 when the player left at the prompt of every tenth floor.
    int32_t& DAT_00889590_isLastFloorCleared() { return FP::getData<int32_t>(0x00889590); }
    /// DAT_00653fa0, the item the floors belong to, it gains the levels.
    PG_DisaEquippedItem& DAT_00653fa0_floorItem() { return FP::getData<PG_DisaEquippedItem>(0x00653fa0); }
    /// DAT_00677278, the music volume of the settings.
    int32_t& DAT_00677278_musicVolume() { return FP::getData<int32_t>(0x00677278); }
    /// DAT_00677298, the music of the map.
    int32_t& DAT_00677298_mapMusic() { return FP::getData<int32_t>(0x00677298); }
    /// DAT_0067729c, the scene of the game loop, the battle map is 0xb.
    int32_t& DAT_0067729c_scene() { return FP::getData<int32_t>(0x0067729c); }
    /// DAT_0065dae8 and DAT_0065daec, x and z of the base panel.
    glm::vec2& DAT_0065dae8_basePanel() { return FP::getData<glm::vec2>(fTeleportPannelX); }
    /// DAT_008896b0, frames of the game over screen (state 600) and of the summon (state 0x50).
    int32_t& DAT_008896b0_stateFrames() { return FP::getData<int32_t>(0x008896b0); }
    /// DAT_006b757c, the units per alliance that can still fight, counted again while the units are drawn.
    std::array<int16_t, 4>& DAT_006b757c_aliveCounts() { return FP::getData<std::array<int16_t, 4>>(0x006b757c); }
    /// DAT_0065db39, the units that can still leave the base panel.
    int8_t& DAT_0065db39_basePanelCount() { return FP::getData<int8_t>(0x0065db39); }
    /// DAT_00889610, frames left of the unit fade.
    int32_t& DAT_00889610_fadeFrames() { return FP::getData<int32_t>(0x00889610); }
    /// DAT_00889614, the fade of the units with DisaMapUnit::field0x277: 0 none, 1 out, 2 in.
    int32_t& DAT_00889614_fadeMode() { return FP::getData<int32_t>(0x00889614); }

    /// A hit FUN_00476160 applies, cleared by FUN_0046f510. The same 0x388 bytes as an entry of DAT_005db920.
    struct DisaHit {
        uint32_t field0x0;         // 0x0
        int32_t damage;            // 0x4
        uint8_t field0x8[0x380];   // 0x8
    };
    static_assert(sizeof(DisaHit) == 0x388);

    /// A reward of the bonus gauge, entries of 0x98 bytes at DAT_00889a30.
    struct DisaBonusReward {
        PG_DisaEquippedItem item;  // 0x00 - type 0
        int32_t type;              // 0x90 - 0 item, 1 HL, 2 experience for the player units on the map
        int32_t amount;            // 0x94
    };
    static_assert(sizeof(PG_DisaEquippedItem) == 0x90);
    static_assert(offsetof(PG_DisaEquippedItem, baseStats) == 0x68);
    static_assert(offsetof(PG_DisaEquippedItem, maxResidents) == 0x81);
    static_assert(sizeof(DisaBonusReward) == 0x98);

    /// Character byte 0x309, the turns the character is depraved and gains no experience.
    uint8_t getDepraveTurns(const DisaPartyCharacterInfo* character) {
        return reinterpret_cast<const uint8_t*>(character)[0x309];
    }

    // ---- Game functions of the states above 300 and the drawing ----

    /**
    * @brief Sets a bit of the game state.
    * Original function: FUN_0046dea0
    */
    void FUN_0046dea0_setGameFlag(int bit) {
        (&FP::getData<uint8_t>(0x00619330))[bit >> 3] |= 1 << (bit & 7);
    }

    /**
    * @brief Versus mode: restarts the turn timer with the time limit of the rules (DAT_006e57b0 seconds) plus frames.
    * Original function: FUN_004452d0
    */
    void FUN_004452d0_startVersusTimer(int frames) {
        if (DAT_0065db3a_gameMode() != 5) return;
        const uint16_t seconds = FP::getData<uint16_t>(0x006e57b0);
        if (seconds == 0) return;
        FP::getData<int32_t>(0x006d4ab8) = frames + seconds * 60;
    }

    /**
    * @brief Versus mode: true when the five rule bytes of the layer at 0x006D4AC0 contain rule.
    * Original function: FUN_00444e10, false while DAT_006e57a8 is 0 and for layers other than 0 and 1.
    */
    bool FUN_00444e10_hasVersusRule(int8_t rule, int8_t layer) {
        if (DAT_0065db3a_gameMode() != 5 || FP::getData<uint8_t>(0x006e57a8) == 0) return false;
        if (layer != 0 && layer != 1) return false;
        const std::span<const int8_t, 5> rules(&FP::getData<int8_t>(0x006d4ac0) + layer * 5, 5);
        return std::ranges::find(rules, rule) != rules.end();
    }

    /**
    * @brief The length of a string of two byte characters, without the terminating 0x0000.
    * Original function: FUN_004e3790
    */
    int FUN_004e3790_getStringLength(const uint8_t* text) {
        int length = 0;
        while (text[length * 2 + 1] != 0 || text[length * 2] != 0) ++length;
        return length;
    }


    /**
    * @brief The item data of an item id, entries of 0xb8 bytes in the table at DAT_00664048, nullptr when unknown.
    * Original function: FUN_0046eff0
    * @details The base stats are 8 int16 at 0xa4, the id is the int16 at 0xb4.
    */
    const uint8_t* FUN_0046eff0_findItemData(int id) {
        const int count = FP::getData<int32_t>(0x0066404c);
        const uint8_t* table = FP::getData<const uint8_t*>(0x00664048);
        for (int i = 0; i < count; ++i) {
            if (*reinterpret_cast<const int16_t*>(table + i * 0xb8 + 0xb4) == id) return table + i * 0xb8;
        }
        return nullptr;
    }

    /// Original function: FUN_00477200, shows a banner, like the turn banners (4 and 10) or the results (5, 6, 7 and 9).
    void FUN_00477200_showBanner(int banner, int param) { FP::callStdFastFarA<void>(0x00477200, banner, param); }
    /// Original function: FUN_0046aad0, fades the music to a volume.
    void FUN_0046aad0_fadeMusic(int volume) { FP::callStdFastFarA<void>(0x0046aad0, volume); }
    /// Original function: FUN_0046a830, plays a music, 0 stops it.
    void FUN_0046a830_playMusic(int music) { FP::callStdFastFarA<void>(0x0046a830, music); }
    /// Original function: FUN_004de090, plays a sound. The fourth argument is never read, FUN_0046aab0 passes the id.
    void FUN_004de090_playSound(int id, int volume, int param3, int param5) {
        FP::callFastFarA<void>(fPlaySound, id, volume, param3, 0, param5);
    }
    /// Original function: FUN_004bd930. The call sites 0x00463F88, 0x0046470F and 0x00464B43 are not hooked.
    int FUN_004bd930_runScriptOriginal(int scriptId) { return FP::callStdFastFarA<int>(0x004bd930, scriptId, 0); }
    /// Original function: FUN_004bf7e0. The call site 0x0046602D is not hooked.
    int FUN_004bf7e0_runTalkScriptOriginal(int scriptId, int dungeonId, int flag, int param4) {
        return FP::callThisFarA<int>(0x004bf7e0, &DisaScriptManager::get(), scriptId, dungeonId, flag, param4);
    }

    /**
    * @brief The width of a string in pixels.
    * Original function: FUN_004f3930. The call sites 0x0046462B and 0x00464656 are hooked with FUN_GetWidthOfString,
    * which translates the string first, like this does.
    */
    int FUN_004f3930_getTextWidth(const uint8_t* text) {
        char* translated = reinterpret_cast<char*>(const_cast<uint8_t*>(text));
        translateReplace(translated);
        return FP::callFastFarA<int>(fGetWidthOfString, translated, 0, -1, static_cast<int*>(nullptr), -1);
    }
    /// Original function: FUN_004e37b0, the length of a converted string in characters.
    uint32_t FUN_004e37b0_getTextLength(const void* text) { return FP::callStdFastFarA<uint32_t>(0x004e37b0, text); }

    /// Original function: FUN_004549c0, draws a key hint, 1 for the prompt of every tenth floor, 2 for the level up.
    void FUN_004549c0_drawHint(int hint) { FP::callStdFastFarA<void>(0x004549c0, hint); }

    /// Original function: FUN_00454e40, draws the level up window of the item.
    void FUN_00454e40_drawItemLevelUp(PG_DisaEquippedItem& item) { FP::callStdFastFarA<void>(0x00454e40, &item); }
    /// Original function: FUN_0046f2c0, recalculates the stats of the item.
    void FUN_0046f2c0_updateItemStats(PG_DisaEquippedItem& item) { FP::callStdFastFarA<void>(0x0046f2c0, &item); }
    /// Original function: FUN_004d4520, puts a copy of the item into storage 0 bag, 1 warehouse. Returns the slot or -1.
    int FUN_004d4520_addItem(int storage, const PG_DisaEquippedItem* item) {
        return FP::callStdFastFarA<int>(0x004d4520, storage, item);
    }
    /// Original function: FUN_00454160, queues the bonus gauge window with the color, the reached reward and the HL.
    void FUN_00454160_drawBonusGauge(const uint8_t* color, int reward, int64_t hl) {
        FP::callFastFarA<void>(0x00454160, color, reward, static_cast<uint32_t>(hl), static_cast<uint32_t>(hl >> 32));
    }
    /// Original function: FUN_0046a8f0, queues FUN_004a7140 at depth 1000 with x, y and an image id.
    void FUN_0046a8f0_queueImage(int x, int y, int image) { FP::callFastFarA<void>(0x0046a8f0, x, y, image); }

    /// Original function: FUN_00446090, plays the queued geo panel changes, nonzero once done.
    int FUN_00446090_updateGeoChain() { return FP::callFarA<int>(0x00446090); }
    /// Original function: FUN_0044c2a0, the AI turn. 11000 passes 0, 0x2AF9 passes 1.
    void FUN_0044c2a0_updateAiTurn(int8_t mode) { FP::callStdFastFarA<void>(0x0044c2a0, mode); }

    /// Original function: FUN_00478ab0, starts a sprite effect at the transform. Only the sprite is traced.
    void FUN_00478ab0_spawnSpriteEffect(int param1, int sprite, int param3, int param4, float param5, DisaTransform* transform,
        DisaCameraInfo& camera, int param8) {
        FP::callFastFarA<void>(0x00478ab0, param1, sprite, param3, param4, param5, transform, &camera, param8);
    }

    /// Original function: FUN_004da020, turns the camera while DAT_006772cc counts down.
    void FUN_004da020_updateCameraSpin(DisaCameraInfo& camera) { FP::callStdFastFarA<void>(0x004da020, &camera); }
    /// Original function: FUN_004fd180, versus mode, gets the idle flag of layer 0 from the start of the frame.
    void FUN_004fd180_updateVersusInput(bool isLayerIdle) { FP::callStdFastFarA<void>(0x004fd180, int8_t(isLayerIdle)); }
    /// Original function: FUN_004576c0, the battle input of layer 1 in versus mode.
    void FUN_004576c0_updateLayer1() { FP::callFarA<void>(0x004576c0); }
    /// Original function: FUN_00445300, counts the versus turn timer down outside of the busy states.
    void FUN_00445300_tickVersusTimer() { FP::callFarA<void>(0x00445300); }
    /// Original function: FUN_004d9cb0, turns the map chunk with the index and the units on it, -1 none.
    void FUN_004d9cb0_updateChunkRotation(int chunk) { FP::callStdFastFarA<void>(0x004d9cb0, chunk); }
    /// Original function: FUN_004cfa80, draws the map background.
    void FUN_004cfa80_drawBackground() { FP::callFarA<void>(0x004cfa80); }
    /// Original function: FUN_004cfd50, draws the map background of a type, DAT_006b7564.
    void FUN_004cfd50_drawBackground(int type) { FP::callStdFastFarA<void>(0x004cfd50, type); }
    /**
    * @brief Original function: FUN_0050e5d0, draws an image of the 0x88 byte table at DAT_00749550.
    * The two stack slots after height are never read.
    */
    void FUN_0050e5d0_drawImage(const uint8_t* image, int x, int y, int depth, int width, int height, int imageWidth,
        int imageHeight) {
        FP::callFastFarA<void>(fRenderImg, image, 0, x, y, depth, width, height, 0, 0, imageWidth, imageHeight);
    }
    /// Original function: FUN_004a2050, marks the tile of a map cursor.
    void FUN_004a2050_markCursorTile(int x, int z, int mode) { FP::callFastFarA<void>(0x004a2050, x, z, mode); }
    /// Original function: FUN_004a2280, submits the tile marks of the frame (DAT_0088c638).
    void FUN_004a2280_submitTileMarks() { FP::callFarA<void>(0x004a2280); }

    /// Original function: FUN_0046c830, animates a sprite actor.
    void FUN_0046c830_updateActor(DisaMapUnit& actor, DisaCameraInfo& camera) {
        FP::callStdFastFarA<void>(0x0046c830, &actor, 0, &camera, 0);
    }
    /**
    * @brief Original function: FUN_0046a810, draws a sprite actor through FUN_0046cf80.
    * Cursors pass depthOffset 0x96 and the cursor style of the settings, units in the fog pass isFogged 1.
    */
    void FUN_0046a810_drawActor(DisaMapUnit& actor, DisaCameraInfo& camera, int depthOffset, int isFogged, int style) {
        FP::callStdFastFarA<void>(0x0046a810, &actor, 0, &camera, depthOffset, isFogged, style);
    }
    /// Original function: FUN_0046a7e0, queues FUN_0046dd20 at depth 40000 for units with drawMode 2. The pushed slot is unused.
    void FUN_0046a7e0_queueActorDraw(DisaMapUnit& actor, DisaCameraInfo& camera) {
        FP::callStdFastFarA<void>(0x0046a7e0, &actor, &camera);
    }
    /// Original function: FUN_004d2e40, aims the model of a unit with modelType at aimTargetIndex.
    void FUN_004d2e40_aimModel(DisaMapUnit& unit) { FP::callStdFastFarA<void>(0x004d2e40, &unit); }
    /// Original function: FUN_0046c660, counts removeFramesLeft down, true when the unit was removed at 0.
    bool FUN_0046c660_updateRemoval(DisaMapUnit& unit, DisaCameraInfo& camera) {
        return FP::callStdFastFarA<int>(0x0046c660, &unit, 0, &camera, 0) != 0;
    }
    /// Original function: FUN_0046bf40, draws the HP bar.
    void FUN_0046bf40_drawHpBar(DisaMapUnit& unit, DisaCameraInfo& camera) {
        FP::callStdFastFarA<void>(0x0046bf40, &unit, 0, &camera);
    }
    /// Original function: FUN_0046bd20, draws the status icons.
    void FUN_0046bd20_drawStatusIcons(DisaMapUnit& unit, DisaPartyCharacterInfo* character, DisaCameraInfo& camera) {
        FP::callStdFastFarA<void>(0x0046bd20, &unit, 0, character, &camera);
    }
    /// Original function: FUN_0046c4c0, draws the marker of a unit that moved and acted.
    void FUN_0046c4c0_drawDoneMarker(DisaMapUnit& unit, DisaCameraInfo& camera) {
        FP::callStdFastFarA<void>(0x0046c4c0, &unit, 0, &camera);
    }
    /// Original function: FUN_004e0140, submits what the units queued since DAT_006b7910.
    void FUN_004e0140_submitUnitQueue() { FP::callFarA<void>(0x004e0140); }
    /// Original function: FUN_0046d7a0, draws a unit placed by script SetActorRawPos at a whole number position.
    void FUN_0046d7a0_drawRawActor(DisaMapUnit& unit, int x, int y, int z, const void* color) {
        FP::callStdFastFarA<void>(0x0046d7a0, &unit, 0, x, y, z, color);
    }
    /// Original function: FUN_004c12d0, the script actors at 0x0071FE80.
    void FUN_004c12d0_updateScriptActors() { FP::callFarA<void>(0x004c12d0); }

    /// Original function: FUN_0046ac00, draws a projectile. The second stack slot is never read.
    void FUN_0046ac00_drawProjectile(DisaProjectile& projectile, DisaCameraInfo& camera) {
        FP::callStdFastFarA<void>(0x0046ac00, &projectile, 0, &camera, 0);
    }
    /// Original function: FUN_00476160, applies the hit to the target, attacker can be nullptr.
    void FUN_00476160_applyHit(DisaMapUnit* attacker, DisaMapUnit& target, DisaHit& hit, int param4, int param5) {
        FP::callFastFarA<void>(0x00476160, attacker, &target, &hit, param4, param5);
    }
    /// Original function: FUN_00476d90, spawns a projectile in the first free slot of DAT_00730f10, returns it or -1.
    int FUN_00476d90_spawnProjectile(int owner, int type, int power, float x, float y, float z, float param7, float directionX,
        float directionZ, int direction) {
        return FP::callFastFarA<int>(0x00476d90, owner, type, power, x, y, z, param7, directionX, directionZ, direction);
    }

    /// Original function: FUN_004449f0, versus mode: true when the layer has no unit left on the map or the base panel.
    bool FUN_004449f0_isLayerDefeated(int8_t layer) { return FP::callStdFastFarA<bool>(0x004449f0, layer); }
    /// Original function: FUN_00444a80, ends a versus battle, 0 and 1 are the defeated layer, 2 both.
    void FUN_00444a80_endVersusBattle(int8_t defeated) { FP::callStdFastFarA<void>(0x00444a80, defeated); }

    /// Copies the render flags of the scope and returns the copy. Original function: FUN_0043cec0
    DisaRenderFlags& FUN_0043cec0_pushRenderFlags() {
        int32_t& depth = FP::getData<int32_t>(0x005bb030);
        DisaRenderFlags* flags = &FP::getData<DisaRenderFlags>(0x005bb038);
        flags[depth + 1] = flags[depth];
        return flags[++depth];
    }
    /// Drops the render flags of FUN_0043cec0_pushRenderFlags.
    void popRenderFlags() { --FP::getData<int32_t>(0x005bb030); }

    /// Original function: FUN_004e13a0, draws a GEO model at an animation frame.
    void FUN_004e13a0_renderGeoModel(DisaGeoModel& model, float frame, DisaCameraInfo& camera, DisaTransform* transform,
        float param9) {
        FP::callStdFastFarA<void>(fRender3DObject, &model, 0, frame, &camera, transform, 0, 0, 0.f, 0, param9);
    }
    /// Original function: FUN_00453bc0, draws the text at DAT_008896c0.
    void FUN_00453bc0_drawInfoText() { FP::callFarA<void>(0x00453bc0); }


    // ---- The frame ----

    /// Locals of FUN_0045c0f0 that live through the whole frame, the stack offsets are the ones after the prologue.
    struct BattleFrame {
        bool isLayerIdle = false;           ///< [esp+0x77] local_1219, FUN_00444c70(0) in versus mode
        int listTop = 10;                   ///< [esp+0x4c] local_1244, the y of the target list rows, 13 with the new text style
        int listCursorAtStart = 0;          ///< esi, DAT_0088951c before the state ran
        int targetCount = 0;                ///< [esp+0x10] local_1280, DAT_00889a20 capped at 8
        glm::ivec2 cursorTile = { 0, 0 };   ///< [esp+0xc] local_1284 and [esp+0x38] local_1258
        /// [esp+0x2c] local_1264 and [esp+0x30] local_1260, the tile of the last unit the hover search looked at.
        glm::ivec2 hoverCheckedTile = { -1, -1 };
        bool isHandled = false;             ///< [esp+0x76] local_121a
        bool isSummonFiring = false;        ///< [esp+0x7f] local_1211, never set, state 0x50 reads it
    };

    /**
    * @brief Pans the camera, finds the hovered unit and prepares the locals of the frame.
    * Part of FUN_0045c0f0, from 0x0045C0F0 to the dispatch at 0x0045C6E6.
    * @details The camera moves through DisaCameraInfo::move, the port of FUN_004f8350. Unlike the original it snaps
    * axes whose step is below 0.0001 to the target and moves at once for a NaN speed.
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
                clampToMap(target.x, target.y);
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
                    const glm::vec3 cursorTarget(cursor.position.x, cursor.restPosition.y, cursor.position.z);
                    if (followedUnit == 0) camera.move(cursorTarget, speed);
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

        frame.listTop = FP::getData<uint8_t>(0x005d7c4e) != 0 || settings->field_0x22 != 0 ? 13 : 10;
        frame.listCursorAtStart = DAT_0088951c_listCursor();
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
        //both overwrite stack slots of the frame locals, listTop and targetCount are not used by state 0
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
            const MessageNames names{ &name, &namePosition, nullptr, &style, 1 };
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
            const bool hasMoveRange = unit.isGeoSymbol == 0 && unit.characterId / 10 != 320 && unit.modelType == 0;
            if (hasMoveRange) FUN_004a1eb0_drawMoveRange(unit.movement);
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

    /// Restores the color of the attack target, it pulsed red while chosen.
    void resetTargetColor(DisaMapUnit& target) {
        target.color.r = 0xff;
        target.color.g = 0xff;
        target.color.b = 0xff;
    }

    /// DAT_0088990c, the number of units the command menu found to lift.
    int16_t& DAT_0088990c_liftTargetCount() { return FP::getData<int16_t>(0x0088990c); }
    /// DAT_00889920, the unit indices of the lift targets, enemies first.
    int16_t* DAT_00889920_liftTargets() { return &FP::getData<int16_t>(0x00889920); }

    /// Original function: FUN_004563f0, draws the list of lift targets. ecx is only passed on to a sound.
    void FUN_004563f0_drawLiftList() { FP::callStdFastFarA<void>(0x004563f0, 0); }
    /// Original function: FUN_004cf290, marks the tiles of the throw line in front of the unit.
    void FUN_004cf290_markThrowLine(DisaMapUnit& unit) { FP::callStdFastFarA<void>(0x004cf290, &unit); }

    /**
    * @brief The cursor of a target list follows the mouse onto the tile of a target, the lists of state 0x1E and 0x82.
    * @details The original compares with the x87 at double width, so the ranges are taken as double.
    */
    void pickTargetByMouse(const DisaMapUnit& unit, const int16_t* targets, int count) {
        if (!DAT_005d7c40_settings()->isMouseActive || FP::getData<uint8_t>(0x00701cc6) == 0) return;
        const glm::ivec2 mouse = FP::getData<glm::ivec2>(0x00701c80);
        glm::vec3 world;
        FUN_004d1070_screenToWorld(&world, mouse.x, mouse.y, unit.position.y, true);
        for (int i = 0; i < count; ++i) {
            const glm::vec3& position = DisaMapUnit::get(targets[i])->position;
            if (double(position.x) - 6.0 <= world.x && world.x < double(position.x) + 6.0
                && double(position.z) - 6.0 <= world.z && world.z < double(position.z) + 6.0) {
                DAT_0088951c_listCursor() = i % 8;
                DAT_00889520_listScroll() = i / 8;
                return;
            }
        }
    }

    /// The marker bobs over the chosen target and the target pulses red, state 0x1E and 0x82.
    void showTargetMarker(DisaMapUnit& target) {
        DisaMapUnit& marker = DAT_00701cd0_targetMarker();
        marker.position.x = target.position.x;
        const int pulse = std::abs(FP::getData<int32_t>(0x006b7918) - 30);
        marker.position.y = float(double(target.restPosition.y) - double(pulse) / 5.0 - 2.0);
        marker.position.z = target.position.z;
        target.color.r = static_cast<int16_t>(pulse * 6 + 255);
        target.color.g = static_cast<int16_t>(255 - pulse * 4);
        target.color.b = target.color.g;
    }


    /**
    * @brief Characters 1080 to 1089 sometimes add one of their skills 702 to 800 to the attack, when their INT wins
    * against the one of the target.
    */
    void queueExtraSkill(DisaMapUnit& unit, const DisaMapUnit& target) {
        DisaPartyCharacterInfo* character = unit.character;
        const int attackRoll = (FUN_0046e1a0_random(0x1a) + 0x19) * static_cast<int>(character->activeStats.intelligence);
        const int defendRoll = (FUN_0046e1a0_random(0x2e) + 5) * static_cast<int>(target.character->activeStats.intelligence);
        if (attackRoll <= defendRoll) return;

        //the highest skill whose last digit stays below a roll of 5, character byte 0x681 is the skill count
        int skill = 0;
        const auto skillCount = [character]() { return static_cast<int>(reinterpret_cast<const int8_t*>(character)[0x681]); };
        for (int i = 0; i < skillCount(); ++i) {
            const int id = static_cast<int16_t>(character->skillId[i]);
            const int roll = FUN_0046e1a0_random(5);
            if (static_cast<uint32_t>(id - 702) <= 98 && id > skill && id % 10 < roll) skill = id;
        }
        if (!FUN_0046f4a0_findSkill(skill)) return;
        if (FUN_00470850_getSkillCost(character, skill, 1) > static_cast<int32_t>(character->currentSP)) return;

        uint8_t* action = DAT_005db920_getAction(DAT_005d9cd0_actionCount());
        FUN_0046f510_clearAction(action);
        *reinterpret_cast<int32_t*>(action + 0x154) = 0x10001;
        *reinterpret_cast<int16_t*>(action + 0x84) = static_cast<int16_t>(DAT_008895b0_selectedUnit());
        *reinterpret_cast<int16_t*>(action + 0x94) = static_cast<int16_t>(DAT_008895ac_hoveredUnit());
        *reinterpret_cast<int16_t*>(action + 0x158) = static_cast<int16_t>(skill);
        *reinterpret_cast<int16_t*>(action + 0x176) = static_cast<int16_t>(FUN_00470850_getSkillCost(character, skill, 1));
        unit.aiArea = 0;
        unit.aiTargetX = static_cast<int16_t>(toTile(target.position.x));
        unit.aiTargetZ = static_cast<int16_t>(toTile(target.position.z));
        unit.aiTargetDirection = 0;
        ++DAT_005d9cd0_actionCount();
    }

    /// Queues the attack of the selected unit on the chosen target, the geo panel under the unit can add attacks.
    void queueAttack(DisaMapUnit& unit, const DisaMapUnit& target) {
        const uint8_t* geoEffect = &FP::getData<uint8_t>(0x006640b8) + unit.geoPanelColor * 32;
        const int attackCount = geoEffect[0] + 1;
        int32_t& actionCount = DAT_005d9cd0_actionCount();
        const int first = actionCount;
        actionCount += attackCount;
        for (int i = 0; i < attackCount; ++i) {
            uint8_t* action = DAT_005db920_getAction(first + i);
            FUN_0046f510_clearAction(action);
            *reinterpret_cast<int32_t*>(action + 0x154) = 0x10001;
            *reinterpret_cast<int16_t*>(action + 0x84) = static_cast<int16_t>(DAT_008895b0_selectedUnit());
            *reinterpret_cast<int16_t*>(action + 0x94) = static_cast<int16_t>(DAT_008895ac_hoveredUnit());
            *reinterpret_cast<int16_t*>(action + 0x158) = 0; //no skill
            const int facing = FUN_004d2700_getFacing(static_cast<int>(target.position.x), static_cast<int>(target.position.z),
                static_cast<int>(unit.position.x), static_cast<int>(unit.position.z));
            *reinterpret_cast<int16_t*>(action + 0x166) = static_cast<int16_t>(facing);
            unit.facingTarget = static_cast<int16_t>(facing);
        }
        if (geoEffect[2] == 0 && static_cast<int16_t>(unit.character->characterId) / 10 == 108) queueExtraSkill(unit, target);
        //the original stores the tile twice when the skill was added
        unit.aiStandX = static_cast<int16_t>(toTile(unit.position.x));
        unit.aiStandZ = static_cast<int16_t>(toTile(unit.position.z));
    }

    /**
    * @brief State 0x1E, choose the target of the attack from the list or with the mouse, then queue the attack.
    * Part of FUN_0045c0f0 at 0x0045DCF9.
    */
    void LAB_0045dcf9_updateAttackTarget(BattleFrame& frame) {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        DisaMapUnit& cursor = DAT_00730ba0_cursor();
        FUN_004d9dc0_updateCameraControls(DAT_00730980_camera());
        DisaMapUnit& unit = *DisaMapUnit::get(DAT_008895b0_selectedUnit());
        int32_t& listCursor = DAT_0088951c_listCursor();
        int32_t& listScroll = DAT_00889520_listScroll();
        const int16_t* targets = DAT_008899a0_attackTargets();

        if (!FUN_0046de80_isGameFlagSet(0xd48)) {
            FUN_0050fb90_hoverList(0x14d, frame.listTop, 0x73, 0x10, frame.targetCount, &listCursor, false);
            FUN_0050fc10_scrollList(0x14d, frame.listTop, 0x73, 0x10, frame.targetCount, &listCursor, listScroll,
                DAT_00889a20_attackTargetCount());
        }
        //the mouse moved, the target under it is chosen
        pickTargetByMouse(unit, targets, DAT_00889a20_attackTargetCount());

        bool isSoundPlayed = frame.isHandled;
        if (frame.listCursorAtStart != listCursor) {
            FUN_0046aab0_playSound(3);
            isSoundPlayed = true;
        }
        if (DAT_00889a20_attackTargetCount() <= 0) {
            listScroll = 0;
            listCursor = 0;
        }
        else {
            FUN_004d0d60_updateListCursor(&listCursor, &listScroll, DAT_00889a20_attackTargetCount(), 8, !isSoundPlayed);
        }
        FUN_00454660_drawTargetList(unit, DAT_00889a20_attackTargetCount());

        const int targetIndex = targets[listScroll + listCursor];
        DAT_008895ac_hoveredUnit() = targetIndex;
        DisaMapUnit& target = *DisaMapUnit::get(targetIndex);
        FUN_0046a930_drawNamePlate(0xa9, target, 0);

        DAT_00701f3b_isTargetMarkerShown() = 1;
        showTargetMarker(target);

        //the weapon range, characters 0xbd6 and 0xc76 only hit in straight lines
        const glm::ivec2 unitTile(toTile(unit.position.x), toTile(unit.position.z));
        for (int z = 0; z < 64; ++z) {
            for (int x = 0; x < 64; ++x) {
                const int weaponRange = unit.character->unknownData6[4]; //character byte 0x688
                if (std::abs(unitTile.x - x) + std::abs(unitTile.y - z) > weaponRange) continue;
                const uint16_t characterId = unit.character->characterId;
                const bool isStraightOnly = characterId == 0xbd6 || characterId == 0xc76;
                if (!isStraightOnly || x == unitTile.x || z == unitTile.y) FUN_004a1c60_markTile(x, z, 3);
            }
        }

        DisaPartyCharacterInfo* scratch = getScratchCharacter();
        const int forecast = FUN_00447d90_forecastAttack(unit, scratch, unit.facingTarget);
        const glm::ivec2 targetTile(toTile(target.position.x), toTile(target.position.z));
        if (std::abs(unitTile.y - targetTile.y) + std::abs(unitTile.x - targetTile.x) == 1 && forecast > 0) {
            FUN_00454610_drawForecast(unit, target, scratch, forecast);
        }

        int32_t& mouseState = *reinterpret_cast<int32_t*>(FP::getData<uint8_t*>(0x005d7c70) + 0x30543c);
        if (keys.isKeyPressed(DisaKeyStates::Cancel) || (FUN_0050f950_isRightMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            mouseState = 0;
            keys.clear();
            FUN_0046aab0_playSound(7);
            stopCursor(cursor);
            resetTargetColor(*DisaMapUnit::get(DAT_008895ac_hoveredUnit()));
            DAT_00701f3b_isTargetMarkerShown() = 0;
            FUN_00446d50_popBattleState(0);
        }
        const bool isSelected = keys.isKeyPressed(DisaKeyStates::Select)
            || (FUN_0050f7c0_isLeftMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48));
        if (!isSelected) return;

        mouseState = 0;
        keys.clear();
        FUN_0046aab0_playSound(1);
        stopCursor(cursor);
        DisaMapUnit& chosen = *DisaMapUnit::get(DAT_008895ac_hoveredUnit());
        resetTargetColor(chosen);
        if (FP::getData<int32_t>(0x005d9cd8) == 0) unit.actionState = 1;
        queueAttack(unit, chosen);
        DAT_00701f3b_isTargetMarkerShown() = 0;
        DAT_00889618_battleState() = 0;
        DAT_008896a0_battleStateDepths()[0] = 0;
    }

    /// DAT_008895cc, the area shape of the chosen skill. From 9 on the area starts at the unit, which can still move.
    int32_t& DAT_008895cc_skillArea() { return FP::getData<int32_t>(0x008895cc); }
    /// DAT_008895c8, the direction of the skill area, 0 to 3.
    int32_t& DAT_008895c8_skillDirection() { return FP::getData<int32_t>(0x008895c8); }

    /// The skill areas the Menu key and the rotate button can turn.
    bool isRotatableArea(int area) {
        return area == 1 || area == 2 || area == 3 || area == 5 || area == 6;
    }

    /// Turns the skill area by one step with sound 9.
    void rotateSkillArea() {
        FUN_0046aab0_playSound(9);
        int32_t& direction = DAT_008895c8_skillDirection();
        ++direction;
        if (direction > 3) direction = 0;
    }

    /// The height difference leaves the window of the skill, DAT_008895b8 down and DAT_008895bc up.
    bool isOutsideSkillHeight(int heightDifference) {
        if (heightDifference < 0) return std::abs(heightDifference) > FP::getData<int32_t>(0x008895b8);
        return heightDifference > FP::getData<int32_t>(0x008895bc);
    }

    /// The tile of the second base panel, the one of player 2 in versus mode.
    glm::ivec2 getSecondBasePanelTile() {
        return glm::ivec2(toTile(FP::getData<float>(0x006e57a0)), toTile(FP::getData<float>(0x006e57a4)));
    }

    /// The tile of the Item World gate.
    glm::ivec2 getGateTile() {
        return glm::ivec2(toTile(FP::getData<float>(0x0065dae0)), toTile(FP::getData<float>(0x0065dae4)));
    }

    /// Counts how many reasons forbid a free tile of kind 2: the panels, the gate, fog, holes and No Entry.
    int countFreeTileBlocks(glm::ivec2 tile, const mpdfileTile* worldTile) {
        int blocked = 0;
        if (FP::getData<uint8_t>(0x0065db43) != 0 && getBasePanelTile() == tile) ++blocked;
        if (DAT_0065db3a_gameMode() == 5 && FP::getData<uint8_t>(0x006d4abe) != 0 && getSecondBasePanelTile() == tile) ++blocked;
        if (FP::getData<uint8_t>(0x0065db41) != 0 && getGateTile() == tile) ++blocked;
        if (FUN_00444990_getFog(0, float(tile.x * 12), float(tile.y * 12)) == 0xff) ++blocked;
        if (!worldTile || worldTile->padding[1] + static_cast<int8_t>(worldTile->mobility) != 0) ++blocked;
        return blocked;
    }

    /// Versus mode, skill 0x855 must not hit the base panel of player 2.
    bool isOnSecondPanelBlocked(const DisaPartyCharacterInfo* character, int kind, glm::ivec2 tile) {
        return DAT_0065db3a_gameMode() == 5 && character->skillId[FP::getData<int32_t>(0x008895c0)] == 0x855 && kind == 1
            && FP::getData<uint8_t>(0x006d4abe) != 0 && getSecondBasePanelTile() == tile;
    }

    /// The unit can be hit by the skill, a unit only hits itself with skill types 4 and 5 (skill byte 0xa1).
    bool isSkillTarget(int index, const DisaMapUnit& other, const uint8_t* skill) {
        if (!other.isActive) return false;
        if (DAT_008895b0_selectedUnit() == index && skill[0xa1] != 4 && skill[0xa1] != 5) return false;
        return FUN_0046f640_getCondition(other.character) != 2 && other.heldByIndex < 0;
    }

    /// The checks of the skill area at the cursor, shown as warnings by FUN_00456930.
    struct SkillAreaCheck {
        int blocked = 0;        ///< [esp+0x14] free tiles of kind 2 that are not free
        int outOfReach = 0;     ///< [esp+0x30] units the height difference keeps out
        int inReach = 0;        ///< [esp+0x44] units the skill hits
        int heightTiles = 0;    ///< [esp+0x60] tiles of kind 3 outside the height window
        int heightFree = 0;     ///< [esp+0x64] tiles of kind 2 outside the height window
    };

    /// Checks every tile of the skill area at the cursor. Part of FUN_0045c0f0 at 0x0045F142.
    SkillAreaCheck LAB_0045f142_checkSkillArea(const DisaMapUnit& unit, const uint8_t* skill, glm::ivec2 cursorTile) {
        SkillAreaCheck check;
        const int area = DAT_008895cc_skillArea();
        const int start = (&FP::getData<int16_t>(0x00664d80))[area];
        const int end = (&FP::getData<int16_t>(0x00664e40))[area];
        if (start >= end) return check;
        const int unitHeight = FUN_0046e7d0_roundHeight(unit.position.y);
        for (int i = start; i < end; ++i) {
            glm::ivec2 tile{};
            const int kind = FUN_004446b0_getAreaTile(i, DAT_008895c8_skillDirection(), cursorTile, tile);
            const int heightDifference = FUN_004d2e00_getTileHeight(tile.x, tile.y) - unitHeight;
            if (kind == 3 && isOutsideSkillHeight(heightDifference)) ++check.heightTiles;
            const mpdfileTile* worldTile = FUN_004a51b0_getWorldTile(tile.x, tile.y);
            if (kind == 2) {
                check.blocked += countFreeTileBlocks(tile, worldTile);
                if (isOutsideSkillHeight(heightDifference)) ++check.heightFree;
            }
            if (isOnSecondPanelBlocked(unit.character, kind, tile)) ++check.blocked;

            for (int index = 0; index < DisaMapUnit::count; ++index) {
                const DisaMapUnit& other = *DisaMapUnit::get(index);
                if (!isSkillTarget(index, other, skill)) continue;
                if (FUN_00444990_getFog(0, other.position.x, other.position.z) == 0xff) continue;
                if (toTile(other.position.x) != tile.x || toTile(other.position.z) != tile.y) continue;
                if (kind == 1 || kind == 3) {
                    if (isOutsideSkillHeight(heightDifference)) ++check.outOfReach;
                    else ++check.inReach;
                }
                if (kind == 2) ++check.blocked;
            }
        }
        return check;
    }

    /**
    * @brief Fills the queued action with the targets of the skill area and queues it, refuses without a visible target
    * or with a blocked tile.
    * Part of FUN_0045c0f0 at 0x0045F7A3.
    * @return True when the skill was queued.
    */
    bool LAB_0045f7a3_queueSkill(DisaMapUnit& unit, const uint8_t* skill, glm::ivec2 cursorTile, int direction) {
        DisaPartyCharacterInfo* character = unit.character;
        const int area = DAT_008895cc_skillArea();
        const uint16_t skillId = *reinterpret_cast<const uint16_t*>(skill);
        uint8_t* action = DAT_005db920_getAction(DAT_005d9cd0_actionCount());
        int targetCount = 0;
        int blocked = 0;
        bool isTargetVisible = false;
        FUN_0046f510_clearAction(action);
        *reinterpret_cast<int32_t*>(action + 0x154) = 1;
        *reinterpret_cast<int16_t*>(action + 0x84) = static_cast<int16_t>(DAT_008895b0_selectedUnit());
        *reinterpret_cast<uint16_t*>(action + 0x158) = character->skillId[FP::getData<int32_t>(0x008895c0)];
        int16_t& actionFacing = *reinterpret_cast<int16_t*>(action + 0x166);
        int cost;
        if (area >= 9) {
            //the unit turns to the area, another direction keeps the 0 of the cleared action
            constexpr std::array<int16_t, 4> facings = { 0, 0x5a, 0xb4, -0x5a };
            if (direction >= 0 && direction <= 3) actionFacing = facings[direction];
            unit.facingTarget = actionFacing;
            cost = FUN_00470850_getSkillCost(character, skillId, 1);
        }
        else {
            actionFacing = 0;
            cost = FUN_00470850_getSkillCost(character, skillId, area + 1);
        }
        *reinterpret_cast<int16_t*>(action + 0x176) = static_cast<int16_t>(cost);

        int16_t& actionTargetCount = *reinterpret_cast<int16_t*>(action + 0x156);
        int16_t* actionTargets = reinterpret_cast<int16_t*>(action + 0x94);
        int16_t* actionTileTargets = reinterpret_cast<int16_t*>(action + 0xd4);
        const int start = (&FP::getData<int16_t>(0x00664d80))[area];
        for (int i = start; i < (&FP::getData<int16_t>(0x00664e40))[area]; ++i) {
            glm::ivec2 tile{};
            const int kind = FUN_004446b0_getAreaTile(i, direction, cursorTile, tile);
            const int heightDifference = FUN_004d2e00_getTileHeight(tile.x, tile.y) - FUN_0046e7d0_roundHeight(unit.position.y);
            //the first tile is the center of the area
            if (i == start) {
                *reinterpret_cast<int16_t*>(action + 0x168) = static_cast<int16_t>(tile.x * 12 + 6);
                *reinterpret_cast<int16_t*>(action + 0x16c) = static_cast<int16_t>(tile.y * 12 + 6);
            }
            if (kind == 3 && isOutsideSkillHeight(heightDifference)) ++blocked;
            const mpdfileTile* worldTile = FUN_004a51b0_getWorldTile(tile.x, tile.y);
            if (kind == 2) blocked += countFreeTileBlocks(tile, worldTile) + (isOutsideSkillHeight(heightDifference) ? 1 : 0);
            if (isOnSecondPanelBlocked(character, kind, tile)) ++blocked;

            for (int index = 0; index < DisaMapUnit::count; ++index) {
                const DisaMapUnit& other = *DisaMapUnit::get(index);
                if (!isSkillTarget(index, other, skill)) continue;
                if (toTile(other.position.x) != tile.x || toTile(other.position.z) != tile.y) continue;
                if ((kind == 1 || kind == 3) && !isOutsideSkillHeight(heightDifference)) {
                    if (FUN_00444990_getFog(0, other.position.x, other.position.z) != 0xff) isTargetVisible = true;
                    ++targetCount;
                    actionTargets[actionTargetCount++] = static_cast<int16_t>(index);
                    actionTileTargets[i - start] = static_cast<int16_t>(index);
                }
                if (kind == 2) ++blocked;
            }
        }
        if (targetCount <= 0 || blocked != 0 || !isTargetVisible) return false;

        FUN_0046aab0_playSound(1);
        if (FP::getData<int32_t>(0x005d9cd8) == 0) unit.actionState = 1;
        unit.aiFacing = static_cast<int16_t>(DAT_008895c8_skillDirection());
        int32_t& actionCount = DAT_005d9cd0_actionCount();
        ++actionCount;
        unit.aiPlanArea = static_cast<uint8_t>(DAT_008895cc_skillArea());
        //weapon skills 701 to 900 repeat with the extra attacks of the geo panel under the unit
        const bool isWeaponSkill = skillId >= 0x2bd && skillId < 0x385;
        const int extraAttacks = isWeaponSkill ? (&FP::getData<uint8_t>(0x006640b8))[unit.geoPanelColor * 32] : 0;
        for (int i = 0; i < extraAttacks; ++i) {
            std::copy_n(action, 0x388, DAT_005db920_getAction(actionCount + i));
        }
        if (extraAttacks > 0) actionCount += extraAttacks;

        //a walk of more than half a tile while aiming counts as the move
        const float movedX = std::abs(float(unit.position.x - unit.moveOrigin.x));
        const float movedZ = std::abs(float(unit.position.z - unit.moveOrigin.z));
        if (static_cast<int>(double(movedX) + double(movedZ)) > 6) unit.moveState = 1;
        unit.aiArea = static_cast<int16_t>(DAT_008895cc_skillArea());
        unit.aiTargetX = static_cast<int16_t>(cursorTile.x);
        unit.aiTargetZ = static_cast<int16_t>(cursorTile.y);
        unit.aiTargetDirection = static_cast<int16_t>(DAT_008895c8_skillDirection());
        unit.aiStandX = static_cast<int16_t>(toTile(unit.position.x));
        unit.aiStandZ = static_cast<int16_t>(toTile(unit.position.z));
        DAT_008896a0_battleStateDepths()[0] = 0;
        DAT_00889618_battleState() = 0;
        return true;
    }

    /// Moves the unit and the cursor back to DAT_008896a8 and DAT_008896ac, the tile the aiming started on.
    void moveBackToSkillStart(DisaMapUnit& unit) {
        const float x = float(FP::getData<int32_t>(0x008896a8));
        const float z = float(FP::getData<int32_t>(0x008896ac));
        const float y = FUN_004d2be0_getGroundHeight(nullptr, 0, x, z);
        unit.FUN_0046cab0_translateTo(x, y, z, 5.f);
        DAT_00730ba0_cursor().FUN_0046cab0_translateTo(x, y, z, 5.f);
    }

    /// Glides the unit and the cursor to a position on the ground.
    void moveUnitAndCursor(DisaMapUnit& unit, float x, float z) {
        const float y = FUN_004d2be0_getGroundHeight(nullptr, 0, x, z);
        unit.FUN_0046cab0_translateTo(x, y, z, 5.f);
        DAT_00730ba0_cursor().FUN_0046cab0_translateTo(x, y, z, 5.f);
    }

    /**
    * @brief Turns the unit to the pad, stick or mouse, the area direction follows.
    * Part of FUN_0045c0f0 at 0x0045E73A.
    */
    void LAB_0045e73a_turnSkillUser(DisaMapUnit& unit, bool isStandBlocked) {
        DisaCameraInfo& camera = DAT_00730980_camera();
        uint8_t& isAiming = FP::getData<uint8_t>(0x006b787b);
        if (isAiming != 0) {
            isAiming = 0;
            FP::getData<uint8_t>(0x006d4ade) = 1;
        }
        if (isStandBlocked) moveBackToSkillStart(unit);
        const bool isPad = (FP::getData<uint8_t>(0x00778d64) & 0xf0) != 0;
        const glm::i16vec2 stick = FP::getData<glm::i16vec2>(0x00778d60);
        const int facing = stick.x == 0 && stick.y == 0 ? FUN_004d1c90_getMouseFacing(camera, DAT_00730ba0_cursor(), unit, isPad)
            : FUN_004d0ee0_getStickFacing(camera);
        if (facing >= 0) DAT_008895c8_skillDirection() = (facing - 2) & 3;
        switch (facing) {
        case 0: unit.facingTarget = 180; break;
        case 1: unit.facingTarget = -90; break;
        case 2: unit.facingTarget = 0; break;
        case 3: unit.facingTarget = 90; break;
        default: break;
        }
    }

    /**
    * @brief Walks the unit inside its move range while it aims, tile by tile by key or freely by mouse.
    * Part of FUN_0045c0f0 at 0x0045E8FA.
    */
    void LAB_0045e8fa_walkSkillUser(DisaMapUnit& unit, bool isStandBlocked) {
        DisaCameraInfo& camera = DAT_00730980_camera();
        FP::getData<uint8_t>(0x006b787b) = 1;
        FUN_004a1eb0_drawMoveRange(unit.movement);
        const glm::ivec2 tile(toTile(unit.position.x), toTile(unit.position.z));
        const glm::ivec2 center = tile * 12 + 6;
        int direction;
        if ((FP::getData<uint8_t>(0x00778d64) & 0xf0) != 0) {
            direction = FUN_004d1c90_getMouseFacing(camera, DAT_00730ba0_cursor(), unit, true);
        }
        else {
            const glm::i16vec2 stick = FP::getData<glm::i16vec2>(0x00778d60);
            direction = stick.x == 0 && stick.y == 0 ? -1 : FUN_004d0ee0_getStickFacing(camera);
        }

        const glm::ivec2 mouse = FP::getData<glm::ivec2>(0x00701c80);
        glm::vec3 world;
        FUN_004d1070_screenToWorld(&world, mouse.x, mouse.y, unit.position.y, true);
        clampToMap(world.x, world.z);
        if (isStandBlocked) moveBackToSkillStart(unit);
        else if (FUN_0050f910_isLeftMouseDown()) moveUnitAndCursor(unit, world.x, world.z);

        //the first press steps at once, holding repeats after 20 frames every 5 frames
        int32_t& repeat = FP::getData<int32_t>(0x005d7c88);
        if (direction < 0) {
            repeat = 0;
            return;
        }
        ++repeat;
        if (repeat >= 25) repeat = 20;
        else if (repeat != 1 && repeat != 20) return;
        constexpr std::array<glm::ivec2, 4> steps = { glm::ivec2(0, -1), glm::ivec2(-1, 0), glm::ivec2(0, 1), glm::ivec2(1, 0) };
        if (direction > 3) return;
        const glm::ivec2 step = steps[direction];
        if (FUN_004495a0_getRangeDistance(tile.x + step.x, tile.y + step.y, 0) > unit.movement) return;
        const glm::ivec2 target = center + step * 12;
        moveUnitAndCursor(unit, float(target.x), float(target.y));
    }

    /**
    * @brief Areas from 9 on start at the unit: it may walk while aiming (middle mouse toggles it, pad button 0x1000
    * holds it) or else turn.
    * Part of FUN_0045c0f0 at 0x0045E674.
    * @return True when the unit stands on a tile it cannot use the skill from.
    */
    bool LAB_0045e674_updateSkillUser(DisaMapUnit& unit) {
        const glm::ivec2 tile(toTile(unit.position.x), toTile(unit.position.z));
        bool isStandBlocked = (&FP::getData<uint8_t>(0x00664f10))[tile.y * 64 + tile.x] >= 200;
        const mpdfileTile* worldTile = FUN_004a51b0_getWorldTile(tile.x, tile.y);
        //geo effect byte 0x22, a panel the unit cannot act from
        if (worldTile && worldTile->geo_mark != 0 && (&FP::getData<uint8_t>(0x006640da))[worldTile->geo_color * 32] != 0) {
            isStandBlocked = true;
        }

        uint8_t& isWalkToggled = FP::getData<uint8_t>(0x005d7c81);
        const auto isWalkHeld = [&unit]() {
            const bool isParalyzed = reinterpret_cast<const uint8_t*>(unit.character)[0x307] != 0;
            return (FP::getData<uint32_t>(0x00778d64) & 0x1000) != 0 && unit.moveState == 0 && !isParalyzed;
        };
        bool isWalking;
        if (isWalkToggled == 0) {
            if (FUN_0050fad0_isMiddleMouseClicked()) {
                isWalkToggled = 1;
                isWalking = true;
            }
            else {
                isWalking = isWalkHeld();
            }
        }
        else if (FUN_0050fad0_isMiddleMouseClicked()) {
            isWalkToggled = 0;
            isWalking = isWalkHeld();
        }
        else {
            isWalking = true;
        }
        //versus fog never walks
        if (isWalking && (DAT_0065db3a_gameMode() != 5 || FP::getData<uint8_t>(0x006e57b3) == 0)) {
            LAB_0045e8fa_walkSkillUser(unit, isStandBlocked);
        }
        else {
            LAB_0045e73a_turnSkillUser(unit, isStandBlocked);
        }
        return isStandBlocked;
    }

    /**
    * @brief Skill 0x386 summons character 0x8fc onto a free tile in range, then the summon plays in state 0x50.
    * Part of FUN_0045c0f0 at 0x0045FED0.
    */
    void LAB_0045fed0_updateSummonTarget(DisaMapUnit& unit, const uint8_t* skill, glm::ivec2 cursorTile, bool isOutOfRange,
        bool isStandBlocked) {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        const int heightDifference = static_cast<int>(double(float(FUN_004d2e00_getTileHeight(cursorTile.x, cursorTile.y)))
            - double(unit.position.y));
        const mpdfileTile* worldTile = FUN_004a51b0_getWorldTile(cursorTile.x, cursorTile.y);
        //the base panel counts even when it is not active
        int blocked = getBasePanelTile() == cursorTile ? 1 : 0;
        if (!worldTile || worldTile->padding[1] + static_cast<int8_t>(worldTile->mobility) != 0) ++blocked;
        if (isOutsideSkillHeight(heightDifference)) ++blocked;
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            const DisaMapUnit& other = *DisaMapUnit::get(i);
            if (other.isActive && toTile(other.position.x) == cursorTile.x && toTile(other.position.z) == cursorTile.y) ++blocked;
        }

        if (keys.isKeyPressed(DisaKeyStates::Select) || (FUN_0050f7c0_isLeftMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            DAT_0067726b_isInputConsumed() = 1;
            *reinterpret_cast<int32_t*>(FP::getData<uint8_t*>(0x005d7c70) + 0x30543c) = 0;
            keys.clear();
            FUN_0046cf10_snapToTile(DAT_00730ba0_cursor());
            FUN_0046cf10_snapToTile(unit);
            if (isOutOfRange || blocked != 0 || isStandBlocked) {
                FUN_0046aab0_playSound(4);
            }
            else {
                DisaPartyCharacterInfo* character = unit.character;
                const int slot = FP::getData<int32_t>(0x008895c0);
                const uint16_t skillId = *reinterpret_cast<const uint16_t*>(skill);
                const float x = float(cursorTile.x * 12 + 6);
                const float z = float(cursorTile.y * 12 + 6);
                const int skillLevel = static_cast<int8_t>(character->skillLevel[slot]);
                const int level = std::max(static_cast<int16_t>(character->level) / 2 + skillLevel, 1);
                const float y = FUN_004d2be0_getGroundHeight(nullptr, 0, x, z);
                const int index = FUN_004d30e0_spawnCharacter(0x8fc, 2, level, x, y, z);
                if (index < 0) {
                    FUN_0046aab0_playSound(4);
                }
                else {
                    DisaMapUnit& summon = *DisaMapUnit::get(index);
                    summon.aiMode = 3;
                    DisaTransform* transform = getEffectTransform();
                    const float effectY = float(double(summon.position.y) + 6.0);
                    transform->position = glm::vec4(summon.position.x, effectY, summon.position.z, 1.f);
                    transform->rotation = glm::vec4(0.f, 0.f, 0.f, 1.f);
                    transform->scale = glm::vec4(1.f);
                    FUN_00478d50_spawnEffect(-1, 0x11, transform, 1.f, 1);
                    FUN_0046aab0_playSound(0x19e);
                    FUN_0046aab0_playSound(1);
                    const int cappedLevel = std::min(skillLevel, 10);
                    FP::getData<int32_t>(0x00889538) = cappedLevel;
                    if (FP::getData<int32_t>(0x005d9cd8) == 0) unit.actionState = 2;
                    character->currentSP -= FUN_00470850_getSkillCost(character, skillId, 1);
                    FUN_0046db50_useSkill(character, skillId);
                    FUN_004db610_updateStats(character, unit.alliance);
                    FP::getData<int32_t>(0x00889534) = 0;
                    FP::getData<int32_t>(0x00889530) = 999;
                    FP::getData<int32_t>(0x008895a0) = 0;
                    FP::getData<int32_t>(0x0088958c) = 0;
                    DAT_008896a0_battleStateDepths()[0] = 0;
                    FP::getData<int32_t>(0x008896b0) = cappedLevel * 60 + 0x293;
                    FP::getData<int32_t>(0x008896b4) = cappedLevel * 60 + 0x293;
                    DAT_00730ba0_cursor().animation = 10;
                    DAT_00889618_battleState() = 0x50;
                    FP::getData<int32_t>(0x006b756c) = 1;
                }
            }
        }
        FUN_004572c0_drawSummonWarnings(isOutOfRange, blocked);
        FUN_0046a9f0_drawTileHeight(0x1a6);
        FUN_00447f00_drawTileInfo();
    }

    /**
    * @brief State 0x3E, choose the target tile of a skill. Shows the range, the area and why it cannot be used, and
    * queues the skill on confirm.
    * Part of FUN_0045c0f0 at 0x0045E59D.
    */
    void LAB_0045e59d_updateSkillTarget() {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        DisaCameraInfo& camera = DAT_00730980_camera();
        DisaMapUnit& cursor = DAT_00730ba0_cursor();
        FUN_004d9dc0_updateCameraControls(camera);
        DisaMapUnit& unit = *DisaMapUnit::get(DAT_008895b0_selectedUnit());
        const DisaPartyCharacterInfo* character = unit.character;
        const uint8_t* skill = reinterpret_cast<const uint8_t*>(
            FUN_0046f4a0_findSkill(static_cast<int16_t>(character->skillId[FP::getData<int32_t>(0x008895c0)])));
        const uint16_t skillId = *reinterpret_cast<const uint16_t*>(skill); //like the original, no check for an unknown skill
        bool isStandBlocked = false;
        if (DAT_008895cc_skillArea() < 9) {
            if (skillId == 0x386) FUN_004d1500_updateMapCursor(&camera, &cursor, DAT_008895cc_skillArea(), 0);
            if (isRotatableArea(DAT_008895cc_skillArea()) && keys.isKeyPressed(DisaKeyStates::Menu)) rotateSkillArea();
        }
        else {
            isStandBlocked = LAB_0045e674_updateSkillUser(unit);
        }

        //the range of the skill around the unit, then the area at the cursor
        const glm::ivec2 cursorTile(toTile(cursor.position.x), toTile(cursor.position.z));
        const glm::ivec2 unitTile(toTile(unit.position.x), toTile(unit.position.z));
        if (DAT_008895ac_hoveredUnit() >= 0) FUN_0046a930_drawNamePlate(0xc1, *DisaMapUnit::get(DAT_008895ac_hoveredUnit()), 0);
        const int range = FP::getData<int32_t>(0x008895c4);
        for (int z = 0; z < 64; ++z) {
            for (int x = 0; x < 64; ++x) {
                if (std::abs(unitTile.x - x) + std::abs(unitTile.y - z) <= range) FUN_004a1c60_markTile(x, z, 1);
            }
        }
        const int distance = std::abs(unitTile.y - cursorTile.y) + std::abs(unitTile.x - cursorTile.x);
        const bool isOutOfRange = distance > range;
        FUN_004a1be0_drawSkillArea(DAT_008895cc_skillArea(), DAT_008895c8_skillDirection(), cursorTile, distance);

        if (keys.isKeyPressed(DisaKeyStates::Cancel) || (FUN_0050f950_isRightMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            FP::getData<uint8_t>(0x005d7c81) = 0;
            FUN_0050fe60_clearMouseClicks();
            keys.clear();
            FUN_0046cf10_snapToTile(cursor);
            FUN_0046cf10_snapToTile(unit);
            FUN_0046aab0_playSound(7);
            if (unit.moveState == 0) {
                unit.position.x = unit.moveOrigin.x;
                unit.position.y = unit.moveOrigin.y;
                unit.position.z = unit.moveOrigin.z;
                cursor.FUN_0046cab0_translateTo(unit.position.x, unit.position.y, unit.position.z, 15.f);
            }
            FUN_00446d50_popBattleState(0);
        }

        if (skillId == 0x386) {
            LAB_0045fed0_updateSummonTarget(unit, skill, cursorTile, isOutOfRange, isStandBlocked);
            return;
        }

        const SkillAreaCheck check = LAB_0045f142_checkSkillArea(unit, skill, cursorTile);
        const bool hasOnlyOutOfReach = check.outOfReach > 0 && check.inReach == 0;
        const bool hasNoTarget = check.outOfReach == 0 && check.inReach == 0;
        const int errorCount = (check.heightFree > 0) + (check.heightTiles > 0) + (check.blocked > 0) + hasNoTarget + isOutOfRange
            + hasOnlyOutOfReach;
        //one warning line each, the rotate button sits below them
        const int warningLines = (check.blocked > 0) + (check.inReach == 0) + (check.heightTiles > 0) + (check.heightFree > 0)
            + isOutOfRange;
        const int buttonTop = warningLines * 16 + 0x11;
        const int buttonLeft = 0x1b5 - FUN_004f3930_getTextWidth(getSystemString(0x14));
        const int buttonWidth = FUN_004f3930_getTextWidth(getSystemString(0x14)) + 0xb;
        if (DAT_008895cc_skillArea() < 9) {
            const int buttonRight = buttonWidth + buttonLeft;
            if (!FUN_0050fe30_isClickInBox(buttonLeft, buttonTop, buttonRight, buttonTop + 0x15)) {
                FUN_004d1500_updateMapCursor(&camera, &cursor, 0, 0);
            }
            const auto isButtonClicked = [=]() {
                return FUN_0050fe30_isClickInBox(buttonLeft, buttonTop, buttonRight, buttonTop + 0x15);
            };
            if (isRotatableArea(DAT_008895cc_skillArea()) && isButtonClicked() && !FUN_0046de80_isGameFlagSet(0xd48)) {
                FUN_0050fe60_clearMouseClicks();
                rotateSkillArea();
            }
        }
        const int direction = DAT_008895c8_skillDirection();

        //a click only confirms while the walk is not toggled on
        if (keys.isKeyPressed(DisaKeyStates::Select)
            || (FUN_0050f7c0_isLeftMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48) && FP::getData<uint8_t>(0x005d7c81) == 0)) {
            FP::getData<uint8_t>(0x005d7c81) = 0;
            DAT_0067726b_isInputConsumed() = 1;
            keys.clear();
            FUN_0046cf10_snapToTile(cursor);
            FUN_0046cf10_snapToTile(unit);
            const bool isQueued = !isOutOfRange && !isStandBlocked && LAB_0045f7a3_queueSkill(unit, skill, cursorTile, direction);
            if (!isQueued) FUN_0046aab0_playSound(4);
        }
        FUN_00456930_drawSkillWarnings(errorCount, check.blocked, check.outOfReach, check.inReach, check.heightTiles,
            check.heightFree, isOutOfRange, DAT_008895cc_skillArea(), unit);
        FUN_0046a9f0_drawTileHeight(0x1a6);
        FUN_00447f00_drawTileInfo();
    }

    /// DAT_006541e0, the item bag of the party, 0x90 bytes per item like the equipment.
    PG_DisaEquippedItem& DAT_006541e0_getBagItem(int slot) { return (&FP::getData<PG_DisaEquippedItem>(0x006541e0))[slot]; }

    /// Original function: FUN_004b3d40, the help line at the bottom of the screen.
    void FUN_004b3d40_drawHelp(const void* text, int arg) { FP::callStdFastFarA<void>(0x004b3d40, text, arg); }
    /// Original function: FUN_004d0850, the rising number at the transform, mode 1 heals.
    void FUN_004d0850_showNumber(int mode, int value, DisaTransform* transform) {
        FP::callFastFarA<void>(0x004d0850, mode, value, transform, &DAT_00730980_camera());
    }
    /// Original function: FUN_004d4270, turns the item into another one, like an empty bottle.
    void FUN_004d4270_changeItem(PG_DisaEquippedItem& item, int id, int value) {
        FP::callFastFarA<void>(0x004d4270, &item, id, value, 0);
    }
    /// Original function: FUN_00504600, updates the stats of a changed item.
    void FUN_00504600_updateItem(PG_DisaEquippedItem& item) { FP::callStdFastFarA<void>(0x00504600, &item); }
    /// Original function: FUN_004d4630, removes the item from the bag.
    void FUN_004d4630_removeBagItem(int bag, int slot) { FP::callStdFastFarA<void>(0x004d4630, bag, slot); }
    /// Original function: FUN_00456610, the window of the item hand over or the steal, rows and the cursor of the list.
    void FUN_00456610_drawItemTransfer(DisaMapUnit& unit, DisaMapUnit& target, PG_DisaEquippedItem& item, int rows, int cursor) {
        FP::callFastFarA<void>(0x00456610, &unit, &target, &item, rows, cursor);
    }

    /// Sets the transform to a position without rotation and with scale 1.
    void setEffectTransform(DisaTransform* transform, float x, float y, float z) {
        transform->position = glm::vec4(x, y, z, 1.f);
        transform->rotation = glm::vec4(0.f, 0.f, 0.f, 1.f);
        transform->scale = glm::vec4(1.f);
    }

    /// Uses a consumable item (type 0x14) on the hovered unit: heals, shows the numbers and the effect.
    void useItem(DisaMapUnit& unit, PG_DisaEquippedItem& item) {
        if (FP::getData<int32_t>(0x005d9cd8) == 0) {
            unit.actionState = 2;
            if (unit.moveState == 1) unit.moveState = 2;
        }
        DisaMapUnit& target = *DisaMapUnit::get(DAT_008895ac_hoveredUnit());
        DisaPartyCharacterInfo* character = target.character;
        const uint32_t oldHp = character->currentHP;
        character->currentHP = item.stats.hp + oldHp;
        character->currentSP += item.stats.sp;
        if (static_cast<int32_t>(character->currentHP) > static_cast<int32_t>(character->activeStats.hp)) {
            character->currentHP = character->activeStats.hp;
        }
        if (static_cast<int32_t>(character->currentSP) > static_cast<int32_t>(character->activeStats.sp)) {
            character->currentSP = character->activeStats.sp;
        }
        DisaTransform* transform = getEffectTransform();
        if (static_cast<int32_t>(item.stats.hp) > 0) {
            transform->position = glm::vec4(target.position.x, float(double(target.position.y) - 5.0), target.position.z, 1.f);
            FUN_004d0850_showNumber(1, static_cast<int32_t>(item.stats.hp), transform);
        }
        if (static_cast<int32_t>(item.stats.sp) > 0) {
            transform->position = glm::vec4(target.position, 1.f);
            FUN_004d0850_showNumber(1, static_cast<int32_t>(item.stats.sp), transform);
        }
        //the bar runs from the new HP to the old one
        FUN_0046b170_setHpBarTo(target, static_cast<int32_t>(oldHp), 0x3c);
        setEffectTransform(transform, target.position.x, target.position.y - 1.f, target.position.z);
        FUN_00478d50_spawnEffect(-1, 0x74, transform, 1.f, 1);
        FUN_0046aab0_playSound(0x34);

        const int value = item.unknownFlag2;
        switch (static_cast<int16_t>(item.id)) {
        case 0x7d2: //the bottle stays as an empty one
            FUN_004d4270_changeItem(item, 0x7d1, value + 1000);
            FUN_00504600_updateItem(item);
            break;
        case 0x7dc:
            FUN_004d4270_changeItem(item, 0x7db, value + 1000);
            FUN_00504600_updateItem(item);
            break;
        case 0x7f8:
        case 0x803:
            //cures poison, sleep, paralyze, forget and deprave
            for (int status = 1; status < 6; ++status) {
                reinterpret_cast<uint8_t*>(target.character)[0x304 + status] = 0;
            }
            FUN_004d4630_removeBagItem(0, FP::getData<int32_t>(0x008895c0));
            break;
        default:
            FUN_004d4630_removeBagItem(0, FP::getData<int32_t>(0x008895c0));
            break;
        }
        DAT_008896a0_battleStateDepths()[0] = 0;
        DAT_00889618_battleState() = 0;
    }

    /**
    * @brief State 0x47, choose the unit to use a bag item on. Type 0x14 heals right away, type 0x15 is handed over
    * in state 0x48.
    * Part of FUN_0045c0f0 at 0x004602E6.
    */
    void LAB_004602e6_updateItemTarget() {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        DisaCameraInfo& camera = DAT_00730980_camera();
        DisaMapUnit& cursor = DAT_00730ba0_cursor();
        if (DAT_008895ac_hoveredUnit() >= 0) FUN_0046a930_drawNamePlate(0xa9, *DisaMapUnit::get(DAT_008895ac_hoveredUnit()), 0);
        FUN_004b3d40_drawHelp(getSystemString(0x258), 0);
        FUN_004d9dc0_updateCameraControls(camera);
        FUN_004d1500_updateMapCursor(&camera, &cursor, 0, 0);

        PG_DisaEquippedItem& item = DAT_006541e0_getBagItem(FP::getData<int32_t>(0x008895c0));
        DisaMapUnit& unit = *DisaMapUnit::get(DAT_008895b0_selectedUnit());
        const glm::ivec2 cursorTile(toTile(cursor.position.x), toTile(cursor.position.z));
        const glm::ivec2 unitTile(toTile(unit.position.x), toTile(unit.position.z));
        const int range = FP::getData<int32_t>(0x008895c4);
        for (int z = 0; z < 64; ++z) {
            for (int x = 0; x < 64; ++x) {
                if (std::abs(unitTile.x - x) + std::abs(unitTile.y - z) <= range) FUN_004a1c60_markTile(x, z, 1);
            }
        }
        const int distance = std::abs(unitTile.y - cursorTile.y) + std::abs(unitTile.x - cursorTile.x);

        if (keys.isKeyPressed(DisaKeyStates::Cancel) || (FUN_0050f950_isRightMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            FUN_0050fe60_clearMouseClicks();
            keys.clear();
            FUN_0046aab0_playSound(7);
            FUN_00446d50_popBattleState(0);
            moveCursorTo(unit, 15.f);
        }
        if (keys.isKeyPressed(DisaKeyStates::Select) || (FUN_0050f7c0_isLeftMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            DAT_0067726b_isInputConsumed() = 1;
            FUN_0050fe60_clearMouseClicks();
            keys.clear();
            FUN_0046cf10_snapToTile(cursor);
            if (item.unknownFlag3 == 0x14) {
                if (DAT_008895ac_hoveredUnit() < 0 || distance > range) FUN_0046aab0_playSound(4);
                else useItem(unit, item);
            }
            if (item.unknownFlag3 == 0x15) {
                const int hovered = DAT_008895ac_hoveredUnit();
                if (hovered < 0 || distance > range || hovered == DAT_008895b0_selectedUnit()) {
                    FUN_0046aab0_playSound(4);
                }
                else {
                    DAT_0088951c_listCursor() = 0;
                    FUN_00446d10_pushBattleState(0x48, 0);
                    FUN_0046aab0_playSound(1);
                }
            }
        }
        if (item.unknownFlag3 != 0x15) {
            FUN_00447f00_drawTileInfo();
            return;
        }
        if (DAT_008895ac_hoveredUnit() >= 0) {
            FUN_00456610_drawItemTransfer(unit, *DisaMapUnit::get(DAT_008895ac_hoveredUnit()), item, 4, -1);
        }
    }

    /// The sprintf of the game, imported at 0x0052814C.
    template<typename... Args>
    void sprintfGame(void* buffer, const void* format, Args... args) {
        using Sprintf = int(__cdecl*)(char*, const char*, ...);
        FP::getData<Sprintf>(0x0052814c)(static_cast<char*>(buffer), static_cast<const char*>(format), args...);
    }

    /// Original function: FUN_004e42c0, sprintf for the new text style, the format is converted first.
    template<typename... Args>
    void FUN_004e42c0_format(void* buffer, const void* format, Args... args) {
        FP::callCdeclFarA<void>(0x004e42c0, buffer, format, args...);
    }


    /**
    * @brief A mouse flag of this frame, 0x2000000 of DAT_00701c58, FUN_0050f7c0 takes it as a click too.
    * Original function: FUN_0050f790, without the isMouseActive check of the others.
    */
    bool FUN_0050f790_isMouseTap() {
        const uint8_t* window = FP::getData<uint8_t*>(0x005d7c70);
        return window[0x305438] == 0 && window[0x30543a] != 0 && (FP::getData<uint32_t>(0x00701c58) & 0x2000000) != 0;
    }

    /// Original function: FUN_004495e0, the chance in percent to steal from the slot, 4 is the steal type row.
    uint32_t FUN_004495e0_getStealChance(DisaMapUnit& unit, DisaMapUnit& target, int slot, int bonus) {
        return FP::callFastFarA<uint32_t>(0x004495e0, &unit, &target, slot, bonus);
    }
    /// Original function: FUN_00477b20, the message that an item was received. The first four stack slots are unused.
    void FUN_00477b20_showItemMessage(const void* text, PG_DisaEquippedItem& item) {
        FP::callCdeclFarA<void>(0x00477b20, 0, 0, 0, 0, text, &item);
    }
    /// Original function: FUN_0046d9c0, the experience a character is worth.
    int64_t FUN_0046d9c0_getExperienceWorth(DisaPartyCharacterInfo* character) {
        return FP::callStdFastFarA<int64_t>(0x0046d9c0, character);
    }
    /// Original function: FUN_0046da60, the HL a character is worth.
    int64_t FUN_0046da60_getMoneyWorth(DisaPartyCharacterInfo* character) {
        return FP::callStdFastFarA<int64_t>(0x0046da60, character);
    }
    /// Original function: FUN_0046e670, the decimal digits of a number, 1 for negative ones.
    int FUN_0046e670_countDigits(int64_t value) { return FP::callCdeclFarA<int>(0x0046e670, value); }
    /// Original function: FUN_004b3e60, a help line made of texts in their own styles. The last two stack slots are unused.
    void FUN_004b3e60_drawHelpLine(const void* const* texts, const uint32_t* styles, const int* positions) {
        FP::callFastFarA<void>(0x004b3e60, texts, styles, positions, 0, 0);
    }

    /**
    * @brief DAT_008be560, the names of the steal types: the eight stats, two empty ones, experience and HL.
    * Filled on first use like the original, bit 0 of DAT_008ca424 marks it.
    */
    std::array<const void*, 12>& DAT_008be560_getStealNames() {
        std::array<const void*, 12>& names = FP::getData<std::array<const void*, 12>>(0x008be560);
        uint32_t& isFilled = FP::getData<uint32_t>(0x008ca424);
        if ((isFilled & 1) == 0) {
            isFilled |= 1;
            names[8] = &FP::getData<uint8_t>(0x00528c0a);
            names[9] = &FP::getData<uint8_t>(0x00528c0b);
            for (int i = 0; i < 8; ++i) names[i] = getSystemString(0x8c0 + i * 4);
            names[10] = getSystemString(0x8e0);
            names[11] = getSystemString(0x8e4);
        }
        return names;
    }

    /// The steal type entry of the target, 0x24 bytes at *0x006192B8, 1 based by DisaMapUnit::field0x3f6.
    const uint8_t* getStealEntry(const DisaMapUnit& target) {
        return FP::getData<const uint8_t*>(0x006192b8) + target.field0x3f6 * 0x24 - 0x24;
    }

    /// Width of the two line steal message with the new text style, the longest line in characters.
    int getStealMessageWidth(const uint8_t* line1Format, const uint8_t* line2, int64_t amount) {
        const int digits = FUN_0046e670_countDigits(amount);
        return std::max(FUN_004e3790_getStringLength(line1Format), FUN_004e3790_getStringLength(line2) + digits / 2) * 13 + 11;
    }

    /**
    * @brief Steals stat type of the target, between level / 3 + 2 of the weaker one and the cap of the thief.
    * Part of FUN_0045c0f0 at 0x00460BD6.
    */
    void LAB_00460bd6_stealStat(DisaMapUnit& unit, DisaMapUnit& target, int type, const uint8_t* line1Format, const uint8_t* line1,
        int line1NamePosition) {
        DisaPartyCharacterInfo* thief = unit.character;
        DisaPartyCharacterInfo* victim = target.character;
        int amount = std::min(static_cast<int16_t>(victim->level) / 3 + 2, static_cast<int16_t>(thief->level) / 3 + 2);
        amount += static_cast<int>(static_cast<uint32_t>(FUN_0046e1a0_random(0x1f)) / 10);
        //character bytes 0x658 to 0x65f
        amount = std::min(amount, reinterpret_cast<const uint8_t*>(thief)[0x658 + type] + 5);
        int32_t& victimStat = reinterpret_cast<int32_t*>(&victim->stats)[type];
        victimStat -= amount;
        if (victimStat < 1) victimStat = 1;
        reinterpret_cast<int32_t*>(&thief->stats)[type] += amount;
        FUN_004db610_updateStats(thief, unit.alliance);

        //line 2 keeps a %s for the name of the thief, FUN_004dd950 fills it in
        std::array<uint8_t, 0x100> line2{};
        std::array<uint8_t, 0x80> line2Format{};
        int line2NamePosition = 0;
        const void* statName = DAT_008be560_getStealNames()[type];
        const bool isOld = isOldTextStyle();
        int width = -1;
        if (isOld) {
            sprintfGame(line2Format.data(), getSystemString(0x8f0), &FP::getData<char>(0x005328c4), statName, amount);
        }
        else {
            std::array<uint8_t, 0x40> convertedName{};
            FUN_004e3440_convertString(convertedName.data(), statName);
            FUN_004e42c0_format(line2Format.data(), getSystemString(0x8f0), &FP::getData<char>(0x005328c8), convertedName.data(),
                amount);
        }
        //the original indexes a zeroed array with the text style
        FUN_004dd950_formatName(line2Format.data(), line2.data(), thief->charName.data(), &line2NamePosition,
            static_cast<int>(FUN_0046dda0_getNameStyle(thief)));
        if (!isOld) width = getStealMessageWidth(line1Format, line2Format.data(), amount);
        const std::array<const char*, 2> names = { victim->charName.data(), thief->charName.data() };
        const std::array<int, 2> positions = { line1NamePosition, line2NamePosition };
        const std::array<int, 2> lines = { 0, 0x10 };
        const std::array<int, 2> styles = { static_cast<int>(FUN_0046dda0_getNameStyle(victim)),
            static_cast<int>(FUN_0046dda0_getNameStyle(thief)) };
        FUN_00477550_showMessage(-1, 0x3a, width, 2, 0x78, &FP::getData<uint8_t>(0x005ad538), line1, line2.data(),
            { names.data(), positions.data(), lines.data(), styles.data(), 2 });
    }

    /// The one name message of the experience and HL steal.
    void showStealAmountMessage(DisaMapUnit& target, int format, int64_t amount, const uint8_t* line1Format, const uint8_t* line1,
        int line1NamePosition) {
        std::array<uint8_t, 0x80> line2{};
        int width = -1;
        if (isOldTextStyle()) {
            sprintfGame(line2.data(), getSystemString(format), amount);
        }
        else {
            FUN_004e42c0_format(line2.data(), getSystemString(format), amount);
            width = getStealMessageWidth(line1Format, line2.data(), amount);
        }
        const char* name = target.character->charName.data();
        const int line = 0;
        const int style = static_cast<int>(FUN_0046dda0_getNameStyle(target.character));
        FUN_00477550_showMessage(-1, 0x3a, width, 2, 0x78, &FP::getData<uint8_t>(0x005ad538), line1, line2.data(),
            { &name, &line1NamePosition, &line, &style, 1 });
    }

    /**
    * @brief Steals half the experience the target is worth, the thief may level up.
    * Part of FUN_0045c0f0 at 0x00460FC4.
    */
    void LAB_00460fc4_stealExperience(DisaMapUnit& unit, DisaMapUnit& target, const uint8_t* line1Format, const uint8_t* line1,
        int line1NamePosition) {
        const int64_t experience = FUN_0046d9c0_getExperienceWorth(target.character) / 2;
        DisaPartyCharacterInfo* thief = unit.character;
        //the experience of the next level, scale of the character data times the table at 0x005E9B28
        const uint8_t* characterData = FP::getData<const uint8_t*>(0x005d9cdc) + static_cast<int16_t>(thief->charDatIndex) * 0xd8;
        const int64_t scale = *reinterpret_cast<const int16_t*>(characterData + 0x72);
        const int64_t levelExperience = (&FP::getData<int64_t>(0x005e9b28))[static_cast<int16_t>(thief->level)];
        const int64_t needed = scale * levelExperience - static_cast<int64_t>(thief->experience);
        if (experience >= needed) {
            DisaTransform* transform = getEffectTransform();
            transform->position.x = unit.position.x;
            transform->position.y = float(double(unit.position.y) - 6.0);
            transform->position.z = unit.position.z;
            FUN_004783d0_spawnEffect(1000, 0xb, transform);
            unit.pose = 9;
            unit.poseFramesLeft = 0x3c;
            FUN_0046b170_setHpBarTo(unit, static_cast<int32_t>(thief->currentHP), 0);
            FUN_0046aab0_playSound(0x17);
        }
        thief->experience += experience;
        FUN_004db610_updateStats(thief, unit.alliance);
        showStealAmountMessage(target, 0x85c, experience, line1Format, line1, line1NamePosition);
    }

    /**
    * @brief Steals half the HL the target is worth into the HL of the party, capped at 9999999999999.
    * Part of FUN_0045c0f0 at 0x00461228.
    */
    void LAB_00461228_stealMoney(DisaMapUnit& target, const uint8_t* line1Format, const uint8_t* line1, int line1NamePosition) {
        const int64_t money = FUN_0046da60_getMoneyWorth(target.character) / 2;
        int64_t& partyMoney = FP::getData<int64_t>(0x00619570);
        partyMoney += money;
        if (partyMoney > 9999999999999) partyMoney = 9999999999999;
        showStealAmountMessage(target, 0x8f4, money, line1Format, line1, line1NamePosition);
    }

    /// Steals by the steal type of the target (row 4), its name and the steal type make the first line.
    void stealByType(DisaMapUnit& unit, DisaMapUnit& target) {
        const uint8_t* entry = getStealEntry(target);
        target.field0x3f6 = 0;
        const int type = entry[0x1d];
        std::array<uint8_t, 0x100> line1{};
        std::array<uint8_t, 0x100> line1Format{};
        int line1NamePosition = 0;
        if (isOldTextStyle()) sprintfGame(line1Format.data(), getSystemString(0x8ec), &FP::getData<char>(0x005328b8), entry);
        else FUN_004e42c0_format(line1Format.data(), getSystemString(0x8ec), &FP::getData<char>(0x005328bc), entry);
        FUN_004dd950_formatName(line1Format.data(), line1.data(), target.character->charName.data(), &line1NamePosition,
            static_cast<int>(FUN_0046dda0_getNameStyle(target.character)));
        if (type <= 7) LAB_00460bd6_stealStat(unit, target, type, line1Format.data(), line1.data(), line1NamePosition);
        else if (type == 10) LAB_00460fc4_stealExperience(unit, target, line1Format.data(), line1.data(), line1NamePosition);
        else if (type == 11) LAB_00461228_stealMoney(target, line1Format.data(), line1.data(), line1NamePosition);
    }

    /**
    * @brief Rolls the steal, takes the equipment of slot 0 to 3 or steals by type for row 4.
    * Part of FUN_0045c0f0 at 0x00460A6D.
    */
    void LAB_00460a6d_steal(DisaMapUnit& unit, DisaMapUnit& target, PG_DisaEquippedItem& tool) {
        if (FP::getData<int32_t>(0x005d9cd8) == 0) {
            unit.actionState = 2;
            if (unit.moveState == 1) unit.moveState = 2;
        }
        DisaTransform* transform = getEffectTransform();
        const int32_t bonus = static_cast<int32_t>(tool.stats.hit);
        const uint32_t chance = FUN_004495e0_getStealChance(unit, target, DAT_0088951c_listCursor(), bonus);
        if (chance <= static_cast<uint32_t>(FUN_0046e1a0_random(100))) {
            //missed
            transform->position = glm::vec4(target.position, 1.f);
            FUN_004d0850_showNumber(2, 0, transform);
            FUN_0046aab0_playSound(0xc);
            DAT_008896a0_battleStateDepths()[0] = 0;
            DAT_00889618_battleState() = 0;
            return;
        }
        FUN_004d4630_removeBagItem(0, FP::getData<int32_t>(0x008895c0));
        const int slot = DAT_0088951c_listCursor();
        if (slot >= 0 && slot <= 3) {
            PG_DisaEquippedItem& equipment = (&target.character->weapon)[slot];
            if (FUN_004d4520_addItem(0, &equipment) >= 0) {
                FUN_00477b20_showItemMessage(getSystemString(0x8e8), equipment);
                equipment.id = 0;
            }
        }
        else if (slot == 4) {
            stealByType(unit, target);
        }
        //the target lost equipment or stats
        DisaPartyCharacterInfo* victim = target.character;
        FUN_004db610_updateStats(victim, target.alliance);
        target.weaponType = static_cast<int8_t>(victim->unknownData6[5]);
        target.weaponSprite = static_cast<int8_t>(victim->field_0x68b);
        target.movement = victim->movement;
        target.jumpPower = static_cast<int8_t>(victim->jumpingPower);
        setEffectTransform(transform, target.position.x, target.position.y + 1.f, target.position.z);
        FUN_00478d50_spawnEffect(-1, 0x91, transform, 1.f, 1);
        FUN_0046aab0_playSound(6);
        DAT_008896a0_battleStateDepths()[0] = 0;
        DAT_00889618_battleState() = 0;
    }

    /// The help line of an equipment slot, nothing to steal or the name of the item.
    void drawStealSlotHelp(DisaMapUnit& target, int slot) {
        std::array<uint8_t, 0x100> help{};
        std::array<uint8_t, 0x100> format{};
        const uint16_t id = (&target.character->weapon)[slot].id;
        const bool isOld = isOldTextStyle();
        if (id == 0) {
            if (isOld) sprintfGame(format.data(), getSystemString(0x8f8));
            else FUN_004e42c0_format(format.data(), getSystemString(0x8f8));
            FUN_004b3d40_drawHelp(format.data(), isOld ? 0 : 1);
            return;
        }
        const void* itemData = FUN_0046eff0_findItemData(static_cast<int16_t>(id));
        if (isOld) sprintfGame(format.data(), getSystemString(0x8fc), &FP::getData<char>(0x005328d0), itemData);
        else FUN_004e42c0_format(format.data(), getSystemString(0x8fc), &FP::getData<char>(0x005328d4), itemData);
        int namePosition = 0;
        FUN_004dd950_formatName(format.data(), help.data(), target.character->charName.data(), &namePosition,
            static_cast<int>(FUN_0046dda0_getNameStyle(target.character)));
        const std::array<const void*, 2> texts = { help.data(), target.character->charName.data() };
        const std::array<uint32_t, 2> styles = { DAT_005d7c40_settings()->field_0x30, FUN_0046dda0_getNameStyle(target.character) };
        const std::array<int, 2> positions = { 0, namePosition };
        FUN_004b3e60_drawHelpLine(texts.data(), styles.data(), positions.data());
    }

    /// The help line of row 4, the steal type of the target.
    void drawStealTypeHelp(DisaMapUnit& target) {
        const uint8_t* entry = getStealEntry(target);
        const void* statName = DAT_008be560_getStealNames()[entry[0x1d]];
        std::array<uint8_t, 0x100> help{};
        std::array<uint8_t, 0x100> format{};
        if (isOldTextStyle()) {
            sprintfGame(format.data(), getSystemString(0x900), &FP::getData<char>(0x005328dc), entry, statName);
        }
        else {
            std::array<uint8_t, 0x40> convertedName{};
            FUN_004e3440_convertString(convertedName.data(), statName);
            FUN_004e42c0_format(format.data(), getSystemString(0x900), &FP::getData<char>(0x005328e0), entry, convertedName.data());
        }
        int namePosition = 0;
        FUN_004dd950_formatName(format.data(), help.data(), target.character->charName.data(), &namePosition,
            static_cast<int>(FUN_0046dda0_getNameStyle(target.character)));
        const std::array<const void*, 2> texts = { help.data(), target.character->charName.data() };
        const std::array<uint32_t, 2> styles = { DAT_005d7c40_settings()->field_0x30, FUN_0046dda0_getNameStyle(target.character) };
        const std::array<int, 2> positions = { 0, namePosition };
        FUN_004b3e60_drawHelpLine(texts.data(), styles.data(), positions.data());
    }

    /**
    * @brief State 0x48, Steal Hand: choose the equipment slot or the steal type of the target, then roll.
    * Part of FUN_0045c0f0 at 0x004607EA.
    */
    void LAB_004607ea_updateSteal(BattleFrame& frame) {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        DAT_008be560_getStealNames();
        DisaMapUnit& unit = *DisaMapUnit::get(DAT_008895b0_selectedUnit());
        PG_DisaEquippedItem& tool = DAT_006541e0_getBagItem(FP::getData<int32_t>(0x008895c0));
        //edi still holds the hovered unit of the prelude
        DisaMapUnit& target = *DisaMapUnit::get(DAT_008895ac_hoveredUnit());
        //characters 1170 to 1179 and 2200 to 2209 also steal by the steal type
        const int thiefClass = static_cast<int16_t>(unit.character->characterId) / 10;
        const int rows = (thiefClass == 117 || thiefClass == 220) && target.field0x3f6 != 0 ? 5 : 4;

        int32_t& listCursor = DAT_0088951c_listCursor();
        if (!FUN_0046de80_isGameFlagSet(0xd48)) {
            FUN_0050fb90_hoverList(0x20, 10, 0xb6, 0x10, rows, &listCursor, false);
            if (frame.listCursorAtStart != listCursor) FUN_0046aab0_playSound(3);
        }
        //held, not just pressed
        if (keys.states[DisaKeyStates::Up] != 0) {
            --listCursor;
            FUN_0046aab0_playSound(3);
        }
        if (keys.states[DisaKeyStates::Down] != 0) {
            ++listCursor;
            FUN_0046aab0_playSound(3);
        }
        if (listCursor < 0) listCursor = rows - 1;
        if (listCursor > rows - 1) listCursor = 0;

        if (keys.isKeyPressed(DisaKeyStates::Cancel) || (FUN_0050f950_isRightMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            keys.clear();
            FUN_0046aab0_playSound(7);
            FUN_00446d50_popBattleState(0);
        }
        //the click helpers record the press position, so they only run without Select like in the original
        const auto isClicked = [rows]() {
            return FUN_0050fe30_isClickInBox(0x20, 10, 0xd6, rows * 16 + 10) || FUN_0050f790_isMouseTap();
        };
        if (keys.isKeyPressed(DisaKeyStates::Select) || (isClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            FUN_0050fe60_clearMouseClicks();
            keys.clear();
            const int slot = listCursor;
            if (slot < 4 && (&target.character->weapon)[slot].id == 0) FUN_0046aab0_playSound(4);
            else LAB_00460a6d_steal(unit, target, tool);
        }

        FUN_0046a930_drawNamePlate(0xa9, target, 0);
        const int slot = listCursor;
        if (slot >= 0 && slot <= 3) drawStealSlotHelp(target, slot);
        else if (slot == 4) drawStealTypeHelp(target);
        FUN_00456610_drawItemTransfer(unit, target, tool, rows, listCursor);
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

    /// State 0x96, plays the queued geo panel changes. Part of FUN_0045c0f0 at 0x00463C8E.
    void LAB_00463c8e_updateGeoChain() {
        if (FUN_00446090_updateGeoChain() == 0) return;
        DAT_00889618_battleStates() = { 0xa0, 0xa0 };
        DAT_008896a0_battleStateDepths() = { 0, 0 };
    }

    /// State 0xA0, the turn change. Part of FUN_0045c0f0 at 0x00463CC2.
    void LAB_00463cc2_updateTurnChange() {
        if (FP::callFarA<int>(0x004455c0) != 0) {
            DAT_008896a0_battleStateDepths()[0] = 0;
            //versus mode continues with the second player
            DAT_00889618_battleState() = DAT_006d4ab2_turnLayer() != 0 ? 1 : 0;
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
            FUN_00444a80_endVersusBattle(2);
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

    // ---- States above 300 ----

    /// The jump target LAB_0046502b, every state that changes the state empties the stack of layer 0.
    void LAB_0046502b_clearStateStack() { DAT_008896a0_battleStateDepths()[0] = 0; }

    /// The idle units of the alliance cheer, all alliances for nullopt. Characters 2210 to 2219 keep their pose frames.
    void setVictoryPoses(std::optional<int8_t> alliance) {
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            DisaMapUnit& unit = *DisaMapUnit::get(i);
            if (!unit.isActive || unit.holdingIndex >= 0 || unit.heldByIndex >= 0) continue;
            if (alliance && unit.alliance != *alliance) continue;
            unit.pose = 9;
            if (static_cast<int16_t>(unit.character->characterId) / 10 != 221) unit.poseFramesLeft = 0x3c;
        }
    }

    /// Shared by state 0x3F2 and game mode 3, runs the script of the next floor. Part of FUN_0045c0f0 at 0x0046470A.
    void LAB_0046470a_nextFloor() {
        FUN_004bd930_runScriptOriginal(0xd);
        DAT_00889618_battleState() = 0x3ea;
        LAB_0046502b_clearStateStack();
    }

    /// Game mode 3: counts the floor and the levels of DAT_00653fa0. Part of FUN_0045c0f0 at 0x00463EA9.
    void LAB_00463ea9_endFloor() {
        uint32_t& levels = reinterpret_cast<uint32_t&>(DAT_006772bc_itemLevels());
        const uint32_t newLevels = ++levels;
        uint16_t& floor = FP::getData<uint16_t>(0x0065db2c);
        ++floor;
        uint32_t& levelsRecord = FP::getData<uint32_t>(0x0065e10c);
        if (newLevels > levelsRecord) levelsRecord = std::min<uint32_t>(newLevels, 100);
        //DAT_0065401c, the int16 at item + 0x7c
        if (int(floor) > FP::getData<int16_t>(0x0065401c)) {
            DAT_00889590_isLastFloorCleared() = 1;
            DAT_00889618_battleState() = 0x44c;
            LAB_0046502b_clearStateStack();
            return;
        }
        if (floor % 10 != 0) {
            LAB_0046470a_nextFloor();
            return;
        }
        DAT_0088951c_listCursor() = 0;
        DAT_00889618_battleState() = 0x3f2;
        LAB_0046502b_clearStateStack();
    }

    /**
    * @brief Game mode 1: raises the difficulty of the dungeon, stores it and runs its closing talk script.
    * Part of FUN_0045c0f0 at 0x00463F2B.
    */
    void LAB_00463f2b_endDungeon() {
        DisaDungeon& dungeon = FP::getData<DisaDungeon>(0x00653a98);
        dungeon.difficulty += dungeon.difficulty < 9999 ? 1 : 0;
        (&FP::getData<DisaDungeon>(aDungeonEntries))[FP::getData<int16_t>(0x0065db1a)] = dungeon;
        if (DisaScriptManager::get().FUN_004bf7e0_RunTalkScript(0, dungeon.id, 2, 0) < 0) FUN_004bd930_runScriptOriginal(0xe);
        DAT_00889618_battleState() = 0x3ea;
        LAB_0046502b_clearStateStack();
    }

    /**
    * @brief Game modes 0 and 2: the last active unit with sprite 1 goes back into the base panel and the cursor takes its
    * place, the cursor goes to the base panel without one. Then the loop leaves the map (scene 0x16).
    * Part of FUN_0045c0f0 at 0x00463F9C.
    */
    void LAB_00463f9c_leaveMap() {
        int last = -1;
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            const DisaMapUnit& unit = *DisaMapUnit::get(i);
            if (unit.isActive && unit.spriteId == 1) last = i;
        }
        DisaMapUnit& cursor = DAT_00730ba0_cursor();
        if (last >= 0) {
            DisaMapUnit& unit = *DisaMapUnit::get(last);
            unit.isActive = false;
            cursor.position.x = unit.position.x;
            cursor.position.z = unit.position.z;
        }
        else {
            const glm::vec2 panel = DAT_0065dae8_basePanel();
            cursor.position.x = float(double(panel.x) + 6.0);
            cursor.position.z = float(6.0 + double(panel.y));
        }
        const int32_t music = DAT_00677298_mapMusic();
        ++DAT_0065db39_basePanelCount();
        DAT_0067729c_scene() = 0x16;
        FUN_0046a830_playMusic(music);
    }

    /// Game mode 5, versus: sets game flag 0xE10. Part of FUN_0045c0f0 at 0x0046403A.
    void LAB_0046403a_endVersus() {
        FUN_0046dea0_setGameFlag(0xe10);
        DAT_00889618_battleState() = 0x3ea;
        LAB_0046502b_clearStateStack();
    }

    /// Game mode 4, also the victory of state 1000 in this mode. Part of FUN_0045c0f0 at 0x0046411B.
    void LAB_0046411b_leaveToScene33() {
        DAT_0067729c_scene() = 0x33;
    }

    /// State 0x3E9, the map is over, the game mode decides what follows. Part of FUN_0045c0f0 at 0x00463E69.
    void LAB_00463e69_endMap() {
        if (DAT_006b7578_busyCount() != 0) return;
        if (DAT_0065db3a_gameMode() != 5) {
            FUN_0046dec0_clearGameFlag(0xcef);
            FUN_0046dec0_clearGameFlag(0xcf0);
        }
        switch (static_cast<int8_t>(DAT_0065db3a_gameMode())) {
        case 0:
        case 2: LAB_00463f9c_leaveMap(); break;
        case 1: LAB_00463f2b_endDungeon(); break;
        case 3: LAB_00463ea9_endFloor(); break;
        case 4: LAB_0046411b_leaveToScene33(); break;
        case 5: LAB_0046403a_endVersus(); break;
        default: break;
        }
    }

    /// The result states end with this once their timer started. Part of FUN_0045c0f0 at 0x00464816.
    void LAB_00464816_endResult() {
        FUN_0046aad0_fadeMusic(DAT_00677278_musicVolume());
        DAT_00889618_battleState() = 0x3e9;
        LAB_0046502b_clearStateStack();
    }

    /// Starts a result: the banner, the victory sound and the poses of the winners, nullopt for every alliance.
    void startResult(int banner, std::optional<int8_t> winners) {
        FUN_00477200_showBanner(banner, 0);
        FUN_0046dec0_clearGameFlag(0xce4);
        FUN_0046aab0_playSound(0x48);
        setVictoryPoses(winners);
    }

    /// The first frame of state 1000. Part of FUN_0045c0f0 at 0x0046406F.
    void LAB_0046406f_startVictory() {
        startResult(5, std::nullopt);
        //without bonus points the gauge starts counted down
        DAT_008895d4_resultTimer() = FP::getData<int32_t>(0x008895dc) == 0 ? 300 : 0;
    }

    /// Shows how many reward items found no room. Part of FUN_0045c0f0 at 0x00464417.
    void LAB_00464417_showLostItems(int lostItems) {
        const uint8_t* color = &FP::getData<uint8_t>(0x005ae3ec);
        if (isOldTextStyle()) {
            //the original buffer is 0x80 bytes
            std::array<char, 0x200> line2{};
            sprintfGame(line2.data(), getSystemString(0x904), lostItems);
            FUN_00477550_showMessage(-1, 0x3a, -1, 2, 0xb4, color, getSystemString(0x860), line2.data());
            return;
        }
        std::array<uint8_t, 0x400> line2{};
        std::array<uint8_t, 0x400> line1{};
        FUN_004e42c0_format(line2.data(), getSystemString(0x904), lostItems);
        FUN_004e3440_convertString(line1.data(), getSystemString(0x860));
        const int width = static_cast<int>(FUN_004e37b0_getTextLength(line1.data()) * 13) / 2 + 0xb;
        FUN_00477550_showMessage(-1, 0x3a, width, 2, 0xb4, color, line1.data(), line2.data());
    }

    /**
    * @brief Hands out the rewards the bonus gauge reached and the HL of the battle, then the map ends.
    * Part of FUN_0045c0f0 at 0x00464312.
    */
    void LAB_00464312_giveRewards() {
        FUN_0046aad0_fadeMusic(DAT_00677278_musicVolume());
        int lostItems = 0;
        const int32_t count = FP::getData<int32_t>(0x008895e0) + 1;
        DisaBonusReward* rewards = &FP::getData<DisaBonusReward>(0x00889a30);
        int64_t& hl = FP::getData<int64_t>(0x00619570);
        for (int i = 0; i < count; ++i) {
            DisaBonusReward& reward = rewards[i];
            if (reward.type == 0) {
                if (FUN_004d4520_addItem(0, &reward.item) < 0 && FUN_004d4520_addItem(1, &reward.item) < 0) ++lostItems;
            }
            else if (reward.type == 1) {
                hl += reward.amount;
            }
            else if (reward.type == 2) {
                for (int u = 0; u < DisaMapUnit::count; ++u) {
                    const DisaMapUnit& unit = *DisaMapUnit::get(u);
                    if (!unit.isActive || unit.alliance != 0 || getDepraveTurns(unit.character) != 0) continue;
                    unit.character->experience += static_cast<uint64_t>(static_cast<int64_t>(reward.amount));
                }
            }
        }
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            const DisaMapUnit& unit = *DisaMapUnit::get(i);
            if (unit.isActive && unit.alliance == 0) FUN_004db610_updateStats(unit.character, 0);
        }
        if (lostItems > 0) LAB_00464417_showLostItems(lostItems);
        //the HL of the battle, DAT_00619580
        hl += FP::getData<int64_t>(0x00619580);
        constexpr int64_t maxHl = 9999999999999;
        if (hl > maxHl) hl = maxHl;
        DAT_00889618_battleState() = 0x3e9;
        LAB_0046502b_clearStateStack();
    }

    /**
    * @brief The bonus gauge counts down over 300 frames and shows the reached reward, a key skips to the end.
    * Part of FUN_0045c0f0 at 0x0046412A.
    * @details DAT_008895dc holds the points and DAT_008895e0 the rank of the gauge at the end of the battle,
    * DAT_008895d8 the best reward shown so far.
    */
    void LAB_0046412a_countBonusGauge() {
        int32_t& timer = DAT_008895d4_resultTimer();
        int32_t frames = timer;
        if (frames != 300) timer = ++frames;

        uint8_t alpha;
        if (frames > 300) {
            const int fadeOut = (0x14b - frames) * 2;
            if (fadeOut < 0) alpha = 0;
            else alpha = fadeOut < 0x80 ? static_cast<uint8_t>(fadeOut * 2) : 0xff;
        }
        else {
            alpha = static_cast<uint8_t>(std::min(frames * 3, 0x40) * 2);
        }
        const std::array<uint8_t, 4> color = { 0x38, 0x18, 0x0c, alpha };
        const int64_t shownHl = int64_t(std::min(frames, 300)) * FP::getData<int64_t>(0x00619580) / 300;
        if (frames % 5 == 0 && frames < 300) {
            FUN_004de090_playSound(0x1a, 0x46, 0, 0);
            frames = timer;
        }
        const uint32_t remaining = static_cast<uint32_t>(300 - std::min(frames, 300)) * FP::getData<uint32_t>(0x008895dc);
        const int32_t points = static_cast<int32_t>(remaining) / 300;
        const int32_t rank = points / 100;
        FP::getData<int16_t>(0x0065db26) = static_cast<int16_t>(rank);
        FP::getData<int16_t>(0x0065db24) = static_cast<int16_t>(points - rank * 100);
        const int32_t shownRank = frames >= 300 ? -1 : int32_t(static_cast<uint16_t>(rank));
        const int32_t reward = FP::getData<int32_t>(0x008895e0) - shownRank;
        int32_t& bestReward = FP::getData<int32_t>(0x008895d8);
        if (reward > bestReward) {
            bestReward = reward;
            FUN_0046aab0_playSound(6);
            frames = timer;
        }
        FUN_00454160_drawBonusGauge(color.data(), reward, shownHl);
        FUN_0046a8f0_queueImage(0xaa, 0x58, 0xd06b);

        DisaKeyStates& keys = DAT_00778d98_keyStates();
        const bool isSkipped = keys.isKeyPressed(DisaKeyStates::Select) || keys.isKeyPressed(DisaKeyStates::Cancel)
            || FUN_0050f7c0_isLeftMouseClicked() || FUN_0050f950_isRightMouseClicked();
        if (isSkipped && frames <= 300) {
            FUN_0046aab0_playSound(1);
            timer = frames == 300 ? 0x12d : 300;
            return;
        }
        if (frames < 0x14a) return;
        LAB_00464312_giveRewards();
    }

    /// State 1000, the victory: the banner and the poses, then the bonus gauge. Part of FUN_0045c0f0 at 0x00464053.
    void LAB_00464053_updateVictory() {
        if (DAT_006b7578_busyCount() != 0) return;
        if (DAT_008895d4_resultTimer() == -1) {
            LAB_0046406f_startVictory();
            return;
        }
        if (DAT_0065db3a_gameMode() == 4) {
            FUN_0046dec0_clearGameFlag(0xcef);
            FUN_0046dec0_clearGameFlag(0xcf0);
            LAB_0046411b_leaveToScene33();
            return;
        }
        LAB_0046412a_countBonusGauge();
    }

    /// State 600, the game over image fades in, then a key runs script 0x11. Part of FUN_0045c0f0 at 0x00464544.
    void LAB_00464544_updateGameOver() {
        int32_t& frames = DAT_008896b0_stateFrames();
        const int doubled = frames * 2;
        const uint8_t alpha = doubled < 0x80 ? static_cast<uint8_t>(doubled * 2) : uint8_t(0xff);
        const std::array<uint8_t, 4> color = { 0xff, 0xff, 0xff, alpha };
        //x, y, image, then the cutouts, as FUN_RenderCutout in Ghidra
        FP::callFastFarA<void>(fRenderCutout, &FP::getData<uint8_t>(0x0074c0e0), color.data(), 134.f, 121.f, 0xd903, 0xd4, 0x1e, 0,
            0x3c, 0xd4, 0x1e);
        frames += frames < 0x78 ? 1 : 0;
        if (frames < 0x78) return;

        DisaKeyStates& keys = DAT_00778d98_keyStates();
        const bool isConfirmed = keys.isKeyPressed(DisaKeyStates::Select) || keys.isKeyPressed(DisaKeyStates::Cancel)
            || FUN_0050f7c0_isLeftMouseClicked() || FUN_0050f950_isRightMouseClicked();
        if (!isConfirmed) return;
        keys.clear();
        FUN_0046aab0_playSound(7);
        FUN_004bd930_RunScript(0x11, nullptr);
    }

    /**
    * @brief State 0x3F2, every tenth floor asks whether to go on (1) or to leave (0).
    * Part of FUN_0045c0f0 at 0x00464612.
    */
    void LAB_00464612_updateFloorPrompt(const BattleFrame& frame) {
        int32_t& choice = DAT_0088951c_listCursor();
        int32_t& lastChoice = FP::getData<int32_t>(0x005d7c84);
        lastChoice = frame.listCursorAtStart;
        const int x = (0x1d5 - FUN_004f3930_getTextWidth(getSystemString(0x86c))) / 2;
        const int width = FUN_004f3930_getTextWidth(getSystemString(0x86c)) + 0xb;
        FUN_0050fb90_hoverList(x, 0x48, width, 0x10, 2, &choice, false);
        FUN_004549c0_drawHint(1);

        DisaKeyStates& keys = DAT_00778d98_keyStates();
        if (keys.isKeyPressed(DisaKeyStates::Up) || keys.isKeyPressed(DisaKeyStates::Down)) choice = choice == 0 ? 1 : 0;
        if (lastChoice != choice) FUN_0046aab0_playSound(3);
        const bool isConfirmed = keys.isKeyPressed(DisaKeyStates::Select) || FUN_0050fe30_isClickInBox(x, 0x48, x + width, 0x68)
            || FUN_0050f790_isMouseTap();
        if (!isConfirmed) return;
        keys.clear();
        if (choice == 1) {
            FUN_0046aab0_playSound(7);
            LAB_0046470a_nextFloor();
            return;
        }
        FUN_0046aab0_playSound(1);
        DAT_00889590_isLastFloorCleared() = 0;
        DAT_00889618_battleState() = 0x44c;
        LAB_0046502b_clearStateStack();
    }

    /// State 0x4B0, versus: layer 0 won. Part of FUN_0045c0f0 at 0x00464768.
    void LAB_00464768_updateLayer0Won() {
        if (DAT_006b7578_busyCount() != 0) return;
        if (DAT_008895d4_resultTimer() != -1) {
            LAB_00464816_endResult();
            return;
        }
        startResult(6, int8_t(0));
        DAT_008895d4_resultTimer() = 0;
    }

    /// State 0x4B1, versus: layer 1 won. Part of FUN_0045c0f0 at 0x00464BE4.
    void LAB_00464be4_updateLayer1Won() {
        if (DAT_006b7578_busyCount() != 0) return;
        if (DAT_008895d4_resultTimer() != -1) {
            LAB_00464816_endResult();
            return;
        }
        startResult(9, int8_t(1));
        DAT_008895d4_resultTimer() = 0;
    }

    /// State 0x4B2, versus: a draw, nobody cheers. Part of FUN_0045c0f0 at 0x00464D3A.
    void LAB_00464d3a_updateDraw() {
        if (DAT_006b7578_busyCount() != 0) return;
        if (DAT_008895d4_resultTimer() != -1) {
            LAB_00464816_endResult();
            return;
        }
        FUN_00477200_showBanner(7, 0);
        FUN_0046dec0_clearGameFlag(0xce4);
        FUN_0046aab0_playSound(0x48);
        DAT_008895d4_resultTimer() = 0;
    }

    /// Leaves the floors: stores the item, runs script 0x10 and the map music again. Part of FUN_0045c0f0 at 0x00464B03.
    void LAB_00464b03_leaveFloors() {
        PG_DisaEquippedItem& item = DAT_00653fa0_floorItem();
        FUN_0046f2c0_updateItemStats(item);
        if (FUN_004d4520_addItem(0, &item) < 0 && FUN_004d4520_addItem(1, &item) < 0) {
            FP::getData<PG_DisaEquippedItem>(0x00661b58) = item; //bag and warehouse are full
        }
        FUN_004bd930_runScriptOriginal(0x10);
        FUN_0046a830_playMusic(DAT_00677298_mapMusic());
        DAT_00889618_battleState() = 0x3ea;
        LAB_0046502b_clearStateStack();
    }

    /**
    * @brief The last level: the stats grow by the base stats of the item data and the residents of each stat, and the
    * item gets more resident slots. Part of FUN_0045c0f0 at 0x0046489F.
    * @details DAT_006772a4, DAT_006772a8 and DAT_006772ac weigh 1, 2 and 3. Each stat keeps the hundredths of its gain
    * in the byte at item + 0x86 + stat.
    */
    void LAB_0046489f_growItemStats(PG_DisaEquippedItem& item) {
        const int32_t weightOne = FP::getData<int32_t>(0x006772a4);
        const int32_t weightTwo = FP::getData<int32_t>(0x006772a8);
        const int32_t weightThree = FP::getData<int32_t>(0x006772ac);
        const int32_t multiplier = weightOne + weightThree * 3 + weightTwo * 2;
        const uint8_t residentSlots = static_cast<uint8_t>(item.maxResidents + static_cast<uint8_t>(weightTwo + weightThree));
        item.maxResidents = residentSlots > 0x10 ? 0x10 : residentSlots;

        const uint8_t* itemData = FUN_0046eff0_findItemData(static_cast<int16_t>(item.id));
        if (!itemData || multiplier <= 0) return;
        int16_t* stats = reinterpret_cast<int16_t*>(&item.baseStats);
        uint8_t* remainders = reinterpret_cast<uint8_t*>(&item) + 0x86;
        const int16_t* dataStats = reinterpret_cast<const int16_t*>(itemData + 0xa4);
        for (int stat = 0; stat < 8; ++stat) {
            int residents = 0;
            for (int i = 0; i < item.maxResidents; ++i) {
                if (item.residentSlots[i].jobIdentifier == stat + 1) ++residents;
            }
            const uint32_t base = static_cast<uint32_t>(std::abs(int(dataStats[stat])) + residents * 30);
            const int32_t gain = static_cast<int32_t>(remainders[stat] + base * static_cast<uint32_t>(multiplier) * 10u);
            const int32_t whole = gain / 100;
            remainders[stat] = static_cast<uint8_t>(static_cast<uint32_t>(gain) - static_cast<uint32_t>(whole) * 100u);
            stats[stat] = static_cast<int16_t>(std::clamp(stats[stat] + whole, -32000, 32000));
        }
    }

    /// No levels left: waits, then a key leaves the floors. Part of FUN_0045c0f0 at 0x00464AAB.
    void LAB_00464aab_finishItemLevelUp() {
        int32_t& timer = DAT_008895d4_resultTimer();
        if (timer > 0) {
            --timer;
            return;
        }
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        if (!keys.isKeyPressed(DisaKeyStates::Select) && !FUN_0050f7c0_isLeftMouseClicked()) {
            FUN_004549c0_drawHint(2);
            return;
        }
        FUN_0050fe60_clearMouseClicks();
        keys.clear();
        FUN_0046aab0_playSound(1);
        FUN_0046aad0_fadeMusic(DAT_00677278_musicVolume());
        LAB_00464b03_leaveFloors();
    }

    /**
    * @brief State 0x44D, the item gains one level every framesPerLevel frames, a key speeds it up to 10.
    * Part of FUN_0045c0f0 at 0x00464830.
    */
    void LAB_00464830_updateItemLevelUp() {
        PG_DisaEquippedItem& item = DAT_00653fa0_floorItem();
        FUN_00454e40_drawItemLevelUp(item);
        int32_t& levels = DAT_006772bc_itemLevels();
        const int32_t levelsLeft = levels;
        if (levelsLeft <= 0) {
            LAB_00464aab_finishItemLevelUp();
            return;
        }
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        int32_t& framesPerLevel = DAT_00889594_framesPerLevel();
        if (keys.isKeyPressed(DisaKeyStates::Select) || FUN_0050f7c0_isLeftMouseClicked()) {
            FUN_0050fe60_clearMouseClicks();
            keys.clear();
            framesPerLevel = 10;
        }
        const int32_t frames = framesPerLevel;
        int32_t& timer = DAT_008895d4_resultTimer();
        if (--timer > 0) return;
        timer = frames;
        levels = levelsLeft - 1;
        if (levels == 0) LAB_0046489f_growItemStats(item);
        ++item.level;
        FUN_0046f2c0_updateItemStats(item);
        if (frames >= 0x1e) {
            const uint8_t* color = &FP::getData<uint8_t>(0x005ae3e0);
            if (isOldTextStyle()) {
                FUN_00477550_showMessage(0x149, 0xc, -1, 1, frames - 4, color, getSystemString(0x908));
            }
            else {
                std::array<uint8_t, 0x400> line{};
                FUN_004e42c0_convertString(line.data(), getSystemString(0x908));
                FUN_00477550_showMessage(0x149, 0xc, -1, 1, frames - 4, color, line.data());
            }
        }
        FUN_0046aab0_playSound(0x17);
    }

    /// State 0x44C, starts the level up of the item. Part of FUN_0045c0f0 at 0x00464B62.
    void LAB_00464b62_startItemLevelUp() {
        FUN_0046f2c0_updateItemStats(DAT_00653fa0_floorItem());
        FUN_0046aad0_fadeMusic(0x28);
        FUN_0046aab0_playSound(0x19);
        const int32_t levels = DAT_006772bc_itemLevels();
        //300 frames for all levels, 30 to 90 per level
        const int32_t frames = 300 / std::max(levels, 1);
        DAT_00889594_framesPerLevel() = frames < 0x1e ? 0x1e : std::min(frames, 0x5a);
        FP::getData<int32_t>(0x00889598) = levels;
        FP::getData<int32_t>(0x0088959c) = FP::getData<int32_t>(0x006b7574);
        DAT_008895d4_resultTimer() = 0x5a;
        DAT_00889618_battleState() = 0x44d;
        LAB_0046502b_clearStateStack();
    }

    /// States 0x2711 and 0x2712, a turn of the layer starts with its banner once the script ended.
    void startTurn(int32_t state, uint8_t layer) {
        if (DisaScriptManager::get().isActive != 0) return;
        const uint8_t mode = DAT_0065db3a_gameMode();
        FUN_00477200_showBanner(mode == 2 || mode == 4 || mode == 5 ? 10 : 4, 0x1e);
        DAT_008896a0_battleStateDepths()[0] = 0;
        DAT_00889618_battleState() = state;
        FUN_004452d0_startVersusTimer(0x96);
        DAT_006d4ab2_turnLayer() = layer;
    }

    /// State 0x2711, the turn of layer 0. Part of FUN_0045c0f0 at 0x00464CC1.
    void LAB_00464cc1_startLayer0Turn() { startTurn(0, 0); }
    /// State 0x2712, the turn of layer 1. Part of FUN_0045c0f0 at 0x00464D81.
    void LAB_00464d81_startLayer1Turn() { startTurn(1, 1); }

    /// State 9000, waits for the script, then returns to the pushed state. Part of FUN_0045c0f0 at 0x00464D21.
    void LAB_00464d21_waitForScript() {
        if (DisaScriptManager::get().isActive == 0) FUN_00446d50_popBattleState(0);
    }

    /// AI step 0, the battle start, then the queued geo panel changes. Part of FUN_0045c0f0 at 0x00464E43.
    void LAB_00464e43_startAiTurn() {
        if (FUN_00466970_updateBattleStart(0) != 1) return;
        if (DAT_006645f4_geoChainCount() <= 0) {
            DAT_00889904_aiStep() = 0xb;
            return;
        }
        DAT_006645fc_geoChainIndex() = 0;
        FP::getData<int32_t>(0x006645f8) = 0; //step of the geo panel chain
        DAT_00889904_aiStep() = 10;
    }

    /// AI step 10, the geo panel changes. Part of FUN_0045c0f0 at 0x00464E27.
    void LAB_00464e27_updateGeoChain() {
        if (FUN_00446090_updateGeoChain() != 0) DAT_00889904_aiStep() = 0xb;
    }

    /// States 11000 and 0x2AF9, the battle start, the geo panel changes, then the AI turn.
    void updateAiTurnStates(int8_t mode) {
        const int32_t step = DAT_00889904_aiStep();
        if (step == 0) LAB_00464e43_startAiTurn();
        else if (step == 10) LAB_00464e27_updateGeoChain();
        else if (step == 0xb) FUN_0044c2a0_updateAiTurn(mode);
    }

    /// State 0x2AF9. Part of FUN_0045c0f0 at 0x00464E05.
    void LAB_00464e05_updateAiTurn() { updateAiTurnStates(1); }
    /// State 11000. Part of FUN_0045c0f0 at 0x00464E7F.
    void LAB_00464e7f_updateAiTurn() { updateAiTurnStates(0); }

    /**
    * @brief State 0x271A: an effect at unit DAT_0088930c, then the cursor and the camera glide to the base panel and
    * the turn of layer 0 starts again. Part of FUN_0045c0f0 at 0x00464EA1.
    */
    void LAB_00464ea1_returnToBasePanel() {
        int32_t& frames = FP::getData<int32_t>(0x008895a0);
        ++frames;
        if (frames == 0x14) {
            const DisaMapUnit& unit = *DisaMapUnit::get(FP::getData<int32_t>(0x0088930c));
            DisaTransform* transform = getEffectTransform();
            FP::getData<int32_t>(0x006772a0) = 5000; //read by the effect
            transform->position.x = unit.position.x;
            transform->position.y = float(double(unit.position.y) - 16.0);
            transform->position.z = unit.position.z;
            transform->position.w = 1.f;
            //rotation.w and scale.w are not written
            transform->rotation.z = 0.f;
            transform->rotation.y = 0.f;
            transform->rotation.x = 0.f;
            transform->scale.z = 1.f;
            transform->scale.y = 1.f;
            transform->scale.x = 1.f;
            FUN_00478ab0_spawnSpriteEffect(-1, 1000, 0xd, 0x5a, 100.f, transform, DAT_00730980_camera(), 0);
        }
        if (frames == 0x5a) {
            const glm::vec2 panel = DAT_0065dae8_basePanel();
            const float x = float(static_cast<int>(double(panel.x) + 6.0));
            const float z = float(static_cast<int>(double(panel.y) + 6.0));
            const float y = FUN_004d2be0_getGroundHeight(nullptr, 0, x, z);
            DAT_00730ba0_cursor().FUN_0046cab0_translateTo(x, y, z, 20.f);
            DAT_00730980_camera().move(glm::vec3(x, y, z), 20.f);
        }
        if (frames < 0x6e) return;
        DAT_00889618_battleState() = 0x2711;
        LAB_0046502b_clearStateStack();
    }

    /// The states above 300. Part of FUN_0045c0f0 at 0x00463E1F.
    void LAB_00463e1f_updateHighState(BattleFrame& frame) {
        switch (DAT_00889618_battleState()) {
        case 0x3e9: LAB_00463e69_endMap(); break;
        case 1000: LAB_00464053_updateVictory(); break;
        case 600: LAB_00464544_updateGameOver(); break;
        case 0x3f2: LAB_00464612_updateFloorPrompt(frame); break;
        case 0x4b0: LAB_00464768_updateLayer0Won(); break;
        case 0x44d: LAB_00464830_updateItemLevelUp(); break;
        case 0x44c: LAB_00464b62_startItemLevelUp(); break;
        case 0x4b1: LAB_00464be4_updateLayer1Won(); break;
        case 0x2711: LAB_00464cc1_startLayer0Turn(); break;
        case 9000: LAB_00464d21_waitForScript(); break;
        case 0x4b2: LAB_00464d3a_updateDraw(); break;
        case 0x2712: LAB_00464d81_startLayer1Turn(); break;
        case 11000: LAB_00464e7f_updateAiTurn(); break;
        case 0x2af9: LAB_00464e05_updateAiTurn(); break;
        case 0x271a: LAB_00464ea1_returnToBasePanel(); break;
        default: break; //0x3EA and the rest wait
        }
    }


    /**
    * @brief State 0x4B, the prompt to leave the Item World with an item: 0 leaves, 1 stays.
    * Part of FUN_0045c0f0 at 0x0046196C.
    */
    void LAB_0046196c_updateLeavePrompt(const BattleFrame& frame) {
        int32_t& choice = DAT_0088951c_listCursor();
        int32_t& lastChoice = FP::getData<int32_t>(0x005d7c84);
        lastChoice = frame.listCursorAtStart;
        const int x = (0x1d5 - FUN_004f3930_getTextWidth(getSystemString(0x86c))) / 2;
        const int width = FUN_004f3930_getTextWidth(getSystemString(0x86c)) + 0xb;
        FUN_0050fb90_hoverList(x, 0x48, width, 0x10, 2, &choice, false);
        FUN_004549c0_drawHint(0);

        DisaKeyStates& keys = DAT_00778d98_keyStates();
        if (keys.isKeyPressed(DisaKeyStates::Up) || keys.isKeyPressed(DisaKeyStates::Down)) choice = choice == 0 ? 1 : 0;
        if (lastChoice != choice) FUN_0046aab0_playSound(3);
        //unlike the other states a right click cancels even with the tutorial flag
        if (keys.isKeyPressed(DisaKeyStates::Cancel) || FUN_0050f950_isRightMouseClicked()) {
            keys.clear();
            FUN_0046aab0_playSound(7);
            FUN_00446d50_popBattleState(0);
        }
        const bool isConfirmed = keys.isKeyPressed(DisaKeyStates::Select) || FUN_0050fe30_isClickInBox(x, 0x48, x + width, 0x68)
            || FUN_0050f790_isMouseTap();
        if (!isConfirmed) return;
        keys.clear();
        if (choice == 1) {
            FUN_0046aab0_playSound(7);
            FUN_00446d50_popBattleState(0);
            return;
        }
        FUN_0046aab0_playSound(1);
        FUN_004d4630_removeBagItem(0, FP::getData<int32_t>(0x008895c0));
        //the levels of the floors so far go to the item, DAT_0065401a is the int16 at item + 0x7a
        FP::getData<int16_t>(0x0065401a) += static_cast<int16_t>(DAT_006772bc_itemLevels());
        FUN_0046f2c0_updateItemStats(DAT_00653fa0_floorItem());
        LAB_00464b03_leaveFloors();
    }

    /// Original function: FUN_0046e4e0, atan2 in degrees, x87 rounded to float before and after the factor.
    float FUN_0046e4e0_atan2Degrees(float y, float x) { return FP::callCdeclFarA<float>(0x0046e4e0, y, x); }
    /// Original function: FUN_004fe820, queues a draw callback at a depth, returns its entry.
    void* FUN_004fe820_queueDraw(uint32_t callback, int depth, int flags) {
        const uint32_t address = reinterpret_cast<uint32_t>(&FP::getData<uint8_t>(callback));
        return FP::callFastFarA<void*>(0x004fe820, address, depth, flags);
    }

    /// The summoned models of skill 0x386: active, player or neutral, a model, standing.
    bool isSummonedModel(const DisaMapUnit& unit) {
        return unit.isActive && (unit.alliance == 0 || unit.alliance == 2) && unit.modelType != 0 && unit.modelPose == 0;
    }

    /// Every summoned model shoots at its facing, the projectile flies to the hovered unit when there is one.
    void fireSummons() {
        int shots = 0;
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            DisaMapUnit& unit = *DisaMapUnit::get(i);
            if (!isSummonedModel(unit)) continue;
            FUN_004e32a0_rotateXZ(unit.modelRotation.x, 0.f, 3.f);
            const float directionX = FP::getData<float>(0x006b791c);
            const float directionZ = FP::getData<float>(0x006b7a60);
            const int index = FUN_00476d90_spawnProjectile(i, 2, static_cast<int32_t>(unit.character->activeStats.attack),
                unit.position.x, float(double(unit.position.y) - 3.0), unit.position.z, 0.f, directionX, directionZ,
                static_cast<int>(unit.modelRotation.y));
            if (index < 0) continue;
            ++shots;
            unit.modelFrame = 0;
            unit.isModelAnimating = 1;
            int32_t& shotsLeft = FP::getData<int32_t>(0x00889530);
            shotsLeft = std::max(shotsLeft - 1, 0);
            DisaProjectile& projectile = DAT_00730f10_projectiles()[index];
            const int hovered = DAT_008895ac_hoveredUnit();
            if (hovered < 0 || hovered == i) continue;
            //the aim point spreads up to 3 around the hovered unit
            DisaMapUnit& target = *DisaMapUnit::get(hovered);
            const auto spread = []() { return float(double(float(FUN_0046e1a0_random(0x3d) - 30)) / 10.0); };
            projectile.target.x = target.position.x + spread();
            projectile.target.y = float(double(target.position.y) + double(spread()) - 12.0);
            projectile.target.z = target.position.z + spread();
            DisaMapUnit& cursor = DAT_00730ba0_cursor();
            cursor.currentAnimation = -1;
            cursor.animation = 10;
            ++projectile.field0xa8;
            ++target.field0x279[0]; //the shots on their way to the target
        }
        if (shots > 0) FUN_004de090_playSound(0x6c, 0x5a, 16000, 0);
    }

    /// The summoned models turn towards the cursor once it is further than sqrt(72) away and aim up or down at it.
    void aimSummons() {
        const DisaMapUnit& cursor = DAT_00730ba0_cursor();
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            DisaMapUnit& unit = *DisaMapUnit::get(i);
            if (!isSummonedModel(unit)) continue;
            const float dx = cursor.position.x - unit.position.x;
            const float dz = cursor.position.z - unit.position.z;
            const double distanceSquared = double(dz) * dz + double(dx) * dx;
            if (distanceSquared < 72.0) continue;
            unit.modelYawTarget = FUN_0046e4e0_atan2Degrees(dx, dz);
            const float distance = static_cast<float>(std::sqrt(double(float(distanceSquared))));
            const float dy = float(double(cursor.restPosition.y) - 3.0 - double(unit.position.y));
            const float pitch = float(double(FUN_0046e4e0_atan2Degrees(distance, dy)) - 90.0);
            unit.modelPitchTarget = pitch;
            if (double(pitch) < -30.0) unit.modelPitchTarget = -30.f;
            if (double(unit.modelPitchTarget) > 60.0) unit.modelPitchTarget = 60.f;
        }
    }

    /**
    * @brief State 0x50, the summoned models of skill 0x386 shoot on Select or the left button while the timer
    * DAT_008896b0 runs, then the summon ends.
    * Part of FUN_0045c0f0 at 0x00461AF0.
    */
    void LAB_00461af0_updateSummonShots(const BattleFrame& frame) {
        DisaCameraInfo& camera = DAT_00730980_camera();
        FUN_004d9dc0_updateCameraControls(camera);
        if ((FP::getData<uint32_t>(0x00778d64) & 0x8000) == 0) FUN_004d1500_updateMapCursor(&camera, &DAT_00730ba0_cursor(), 0, 0);

        int32_t& framesLeft = DAT_008896b0_stateFrames();
        int32_t& lastShot = FP::getData<int32_t>(0x008896b4);
        const int32_t frames = framesLeft;
        //the mouse fires at most every 5 frames, the key every frame it is held
        bool isFiring = frame.isSummonFiring;
        if (lastShot - frames > 5 && (FUN_0050ff80_isLeftMouseLongPress() || FUN_0050f910_isLeftMouseDown())) {
            lastShot = frames;
            isFiring = true;
        }
        if (DAT_00778d98_keyStates().states[DisaKeyStates::Select] != 0 || isFiring) fireSummons();
        aimSummons();

        int32_t& shownFrames = FP::getData<int32_t>(0x008895a0);
        shownFrames = std::min(shownFrames + 1, 0x1e);
        if (frames <= 0x1e) shownFrames = frames;
        FUN_004fe820_queueDraw(0x00457410, 1000, 0); //FUN_00457410 draws the timer
        framesLeft = frames - 1;
        if (framesLeft > 0) return;

        for (int i = 0; i < DisaMapUnit::count; ++i) {
            DisaMapUnit& unit = *DisaMapUnit::get(i);
            if (unit.isActive && unit.alliance == 0 && unit.modelType != 0 && unit.modelPose == 0) {
                FUN_0046b170_setHpBarTo(unit, static_cast<int32_t>(unit.character->currentHP), 0);
            }
        }
        for (DisaProjectile& projectile : DAT_00730f10_projectiles()) {
            if (projectile.isActive == 0) continue;
            projectile.isActive = 0;
            projectile.field0xa8 = 0;
        }
        DAT_00889618_battleState() = 0;
        DAT_00730ba0_cursor().animation = 3;
        DAT_008896a0_battleStateDepths()[0] = 0;
    }

    /// Lifts the hovered unit: the lifter turns to it, the unit goes on top and its old place is kept for the undo.
    void liftTarget(DisaMapUnit& unit) {
        DisaMapUnit& target = *DisaMapUnit::get(DAT_008895ac_hoveredUnit());
        resetTargetColor(target);
        //a model that was knocked over stands up again
        if (target.modelType == 1 && target.modelPose != 0) {
            target.modelPose = 0;
            FP::getData<int32_t>(0x00889534) = 1;
        }
        FUN_004de090_playSound(0x2e, FP::getData<int32_t>(0x00677274), 30000, 0);
        target.heldUnitOrigin = target.position;
        target.heldUnitOriginFacing = target.facingTarget;
        unit.actionState = 1;
        unit.facingTarget = static_cast<int16_t>(FUN_004d2700_getFacing(static_cast<int>(target.position.x),
            static_cast<int>(target.position.z), static_cast<int>(unit.position.x), static_cast<int>(unit.position.z)));
        unit.pose = 8;
        unit.heldUnitFacingOffset = static_cast<int16_t>(target.facingTarget - unit.facingTarget);
        if (target.holdingIndex < 0) target.pose = 5;
        target.heldByIndex = static_cast<int8_t>(DAT_008895b0_selectedUnit());
        unit.holdingIndex = static_cast<int8_t>(DAT_008895ac_hoveredUnit());
        unit.throwDistance = 1;
        FUN_004d2960_updateGeoPanels();
        target.position.x = unit.position.x;
        DAT_008896a0_battleStateDepths()[0] = 0;
        DAT_00889618_battleState() = 0;
        const float lift = unit.spriteId == 1 ? 1.f : 0.f;
        target.position.y = float(double(unit.position.y) - 14.0 + double(lift));
        target.position.z = unit.position.z;
        FUN_004da320_refreshUnits();
        FUN_00444810_updateFog(0);
        FUN_00444810_updateFog(1);
    }

    /**
    * @brief State 0x82, choose the unit to lift from the list or with the mouse. A unit that already holds one only
    * sees its throw line.
    * Part of FUN_0045c0f0 at 0x00461FD5.
    */
    void LAB_00461fd5_updateLiftTarget(const BattleFrame& frame) {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        DisaCameraInfo& camera = DAT_00730980_camera();
        FUN_004d9dc0_updateCameraControls(camera);
        DisaMapUnit& unit = *DisaMapUnit::get(DAT_008895b0_selectedUnit());
        const int rows = std::min<int>(DAT_0088990c_liftTargetCount(), 8);

        if (unit.holdingIndex < 0) {
            int32_t& listCursor = DAT_0088951c_listCursor();
            int32_t& listScroll = DAT_00889520_listScroll();
            if (!FUN_0046de80_isGameFlagSet(0xd48)) {
                FUN_0050fb90_hoverList(0x14d, 0xc, 0x73, 0xe, rows, &listCursor, false);
                FUN_0050fc10_scrollList(0x14d, 0xc, 0x73, 0xe, rows, &listCursor, listScroll, DAT_0088990c_liftTargetCount());
            }
            bool isSoundPlayed = frame.isHandled;
            if (frame.listCursorAtStart != listCursor) {
                FUN_0046aab0_playSound(3);
                isSoundPlayed = true;
            }
            if (DAT_0088990c_liftTargetCount() <= 0) {
                listScroll = 0;
                listCursor = 0;
            }
            else {
                FUN_004d0d60_updateListCursor(&listCursor, &listScroll, DAT_0088990c_liftTargetCount(), 8, !isSoundPlayed);
            }
            pickTargetByMouse(unit, DAT_00889920_liftTargets(), DAT_0088990c_liftTargetCount());
            FUN_004563f0_drawLiftList();
            DAT_00701f3b_isTargetMarkerShown() = 1;
            const int targetIndex = DAT_00889920_liftTargets()[listScroll + listCursor];
            DAT_008895ac_hoveredUnit() = targetIndex;
            showTargetMarker(*DisaMapUnit::get(targetIndex));

            const glm::ivec2 unitTile(toTile(unit.position.x), toTile(unit.position.z));
            for (int z = 0; z < 64; ++z) {
                for (int x = 0; x < 64; ++x) {
                    if (std::abs(unitTile.x - x) + std::abs(unitTile.y - z) <= 1) FUN_004a1c60_markTile(x, z, 3);
                }
            }
        }
        else {
            //ecx still holds the 8 the rows were capped with, the argument is unused
            FUN_004d1500_updateMapCursor(&camera, &DAT_00730ba0_cursor(), 8, 0);
            FUN_004cf290_markThrowLine(unit);
            FUN_0046a930_drawNamePlate(0xc1, *DisaMapUnit::get(DAT_008895ac_hoveredUnit()), 0);
        }

        int32_t& mouseState = *reinterpret_cast<int32_t*>(FP::getData<uint8_t*>(0x005d7c70) + 0x30543c);
        if (keys.isKeyPressed(DisaKeyStates::Cancel) || (FUN_0050f950_isRightMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            mouseState = 0;
            keys.clear();
            FUN_0046aab0_playSound(7);
            DAT_00701f3b_isTargetMarkerShown() = 0;
            //without a hovered unit the original also writes in front of the unit array
            resetTargetColor(*DisaMapUnit::get(DAT_008895ac_hoveredUnit()));
            FUN_00446d50_popBattleState(0);
        }
        const bool isSelected = keys.isKeyPressed(DisaKeyStates::Select)
            || (FUN_0050f7c0_isLeftMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48));
        if (!isSelected) return;
        mouseState = 0;
        keys.clear();
        DAT_00701f3b_isTargetMarkerShown() = 0;
        if (unit.holdingIndex < 0) liftTarget(unit);
    }

    /// Original function: FUN_00456100, the window of the throw.
    void FUN_00456100_drawThrowWindow(DisaMapUnit& unit) { FP::callStdFastFarA<void>(0x00456100, &unit); }
    /// Original function: FUN_00475c50, 1 when the held unit cannot be thrown onto the other one.
    int FUN_00475c50_isStackRefused(DisaMapUnit& held, DisaMapUnit& other) {
        return FP::callStdFastFarA<int>(0x00475c50, &held, &other);
    }
    /// Original function: FUN_004de990, ends the loop of the sprite animation so it plays out.
    void FUN_004de990_endAnimationLoop(DisaMapUnit& unit) { FP::callStdFastFarA<void>(0x004de990, &unit.sprite); }
    /// Original function: FUN_0046c9b0_jumpTo, the unit jumps to the position with an arc.
    void FUN_0046c9b0_jumpTo(DisaMapUnit& unit, float x, float y, float z, float height, float frames) {
        FP::callThisFarA<void>(0x0046c9b0, &unit, x, y, z, height, frames);
    }

    /**
    * @brief The tile at a world position, nullptr outside or for a hole.
    * Original function: FUN_004a51e0
    */
    const mpdfileTile* FUN_004a51e0_getWorldTileAt(float x, float z) {
        return FUN_004a51b0_getWorldTile(toTile(x), toTile(z));
    }

    /// The held unit may be captured by throwing it into a base panel, the checks shared by both panels.
    bool isCapturable(const DisaMapUnit& held) {
        //character bytes 0x684 and 0x69e
        const uint8_t* character = reinterpret_cast<const uint8_t*>(held.character);
        const bool isFree = static_cast<int8_t>(character[0x684]) < 0 && character[0x69e] == 0 && held.holdingIndex < 0
            && held.isGeoSymbol == 0;
        return isFree && held.itemWorldTitle == 0 && held.itemWorldRank == 0 && held.modelType == 0;
    }

    /**
    * @brief Aims the throw: the facing picks the direction, pushing it again throws further up to throwRange.
    * Part of FUN_0045c0f0 at 0x004625AF to 0x004627D8.
    */
    void aimThrow(DisaMapUnit& unit) {
        DisaCameraInfo& camera = DAT_00730980_camera();
        uint8_t& isKeyAimed = FP::getData<uint8_t>(0x008be55c);
        bool isPad = false;
        if ((FP::getData<uint8_t>(0x00778d64) & 0xf0) != 0) {
            isPad = true;
            DAT_0067726b_isInputConsumed() = 1;
            isKeyAimed = 1;
            *reinterpret_cast<int32_t*>(FP::getData<uint8_t*>(0x005d7c70) + 0x30543c) = 0;
        }
        const glm::i16vec2 stick = FP::getData<glm::i16vec2>(0x00778d60);
        const bool isStick = stick.x != 0 || stick.y != 0;
        if (isStick) {
            isPad = true;
            DAT_0067726b_isInputConsumed() = 1;
            isKeyAimed = 1;
        }
        int facing;
        if (FP::getData<int32_t>(0x0071fd34) != 0) facing = FUN_004d0db0_getPadFacing(camera, false);
        else if (isStick) facing = FUN_004d0ee0_getStickFacing(camera);
        else facing = FUN_004d1c90_getMouseFacing(camera, DAT_00730ba0_cursor(), unit, isPad);

        //once the camera reached the cursor the mouse takes over again
        if (isKeyAimed != 0 && !isPad) {
            const DisaMapUnit& cursor = DAT_00730ba0_cursor();
            const float distanceX = std::abs(float(cursor.position.x - camera.eyeOffset.x));
            const float distanceZ = std::abs(float(cursor.position.z - camera.eyeOffset.z));
            if (1.f <= distanceX || 1.f <= distanceZ) DAT_0067726b_isInputConsumed() = 1;
            else isKeyAimed = 0;
        }

        int32_t& repeat = FP::getData<int32_t>(0x005d7c88);
        if (facing < 0) {
            repeat = 0;
            return;
        }
        ++repeat;
        if (repeat >= 25) repeat = 20;
        else if (repeat != 1 && repeat != 20) return;
        //facing 0 to 3 looks at 180, -90, 0 and 90, the opposite one pulls the throw closer
        constexpr std::array<int, 4> forwards = { 180, -90, 0, 90 };
        constexpr std::array<int, 4> backwards = { 0, 90, 180, -90 };
        //another facing compares with a stale stack value, it never matches a facing
        const int forward = facing <= 3 ? forwards[facing] : INT32_MIN;
        const int backward = facing <= 3 ? backwards[facing] : INT32_MIN;
        if (unit.facingTarget == forward) {
            if (isPad && unit.throwDistance < unit.throwRange) ++unit.throwDistance;
            return;
        }
        if (unit.facingTarget == backward && isPad) {
            --unit.throwDistance;
            if (unit.throwDistance >= 1) return;
            unit.throwDistance = 1;
        }
        FUN_004cf1d0_turnUnit(unit, facing);
    }

    /**
    * @brief Checks the landing tile and every step of the throw line. -1 allows the throw, anything else refuses it.
    * Part of FUN_0045c0f0 at 0x004628AB to 0x00462EF8.
    * @details Like the original a refusal is the index of the unit on the tile or 0 for the other reasons.
    */
    int checkThrow(DisaMapUnit& unit, DisaMapUnit& held, glm::ivec2 cursorTile) {
        //the unit on the landing tile, -1 a free tile or one the held unit can be stacked on
        int landing = -1;
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            DisaMapUnit& other = *DisaMapUnit::get(i);
            if (!other.isActive || &other == &held || other.heldByIndex >= 0) continue;
            if (FUN_0046f640_getCondition(other.character) == 2) continue;
            if (toTile(other.position.x) != cursorTile.x || toTile(other.position.z) != cursorTile.y) continue;
            landing = i;
            if (held.holdingIndex < 0 && FUN_00475c50_isStackRefused(held, other) == 0) landing = -1;
            break;
        }
        const int16_t characterId = held.characterId;
        //the exploding characters land anywhere outside of dungeons 1100 to 1199
        if ((characterId / 10 == 221 || characterId == 0xc62) && FP::getData<DisaDungeon>(0x00653a98).id / 100 != 11) landing = -1;
        if (characterId == 0xc81) landing = -1;
        int check = landing;
        if (characterId == 0xc80 && landing != -1 && DisaMapUnit::get(landing)->characterId != 0xc80
            && DisaMapUnit::get(landing)->holdingIndex == -1) {
            //the tile is compared with panel / 12, which is never a whole number, so this always allows the throw
            const auto isOnPanel = [cursorTile](float x, float z) {
                constexpr double tileFactor = double(1.f / 12.f);
                return float(cursorTile.x) == double(x) * tileFactor && float(cursorTile.y) == double(z) * tileFactor;
            };
            const bool isOnSecondPanel = FP::getData<uint8_t>(0x006d4abe) != 0
                && isOnPanel(FP::getData<float>(0x006e57a0), FP::getData<float>(0x006e57a4));
            const bool isOnFirstPanel = FP::getData<uint8_t>(0x0065db43) != 0
                && isOnPanel(FP::getData<float>(fTeleportPannelX), FP::getData<float>(fTeleportPannelZ));
            if (!isOnSecondPanel && !isOnFirstPanel) check = -1;
        }
        //no tile, a hole
        if (!FUN_004a51b0_getWorldTile(cursorTile.x, cursorTile.y)) check = 0;

        const bool isVersus = DAT_0065db3a_gameMode() == 5;
        const auto checkPanelCapture = [&held, characterId, &check](int8_t ownAlliance) {
            if (held.alliance != ownAlliance && characterId != 0xc80 && characterId != 0xc82) check = 0;
            if (!isCapturable(held)) check = 0;
        };
        if (FP::getData<uint8_t>(0x0065db43) != 0 && getBasePanelTile() == cursorTile) {
            if (isVersus) {
                checkPanelCapture(0);
            }
            else {
                if (!isCapturable(held)) check = 0;
                if (held.alliance == 2) check = 0;
                if (DAT_0065db3a_gameMode() == 4 && held.alliance != 0) check = 0;
                //the party has room for 128
                const int partySize = FP::getData<int16_t>(0x0065db12) + FP::getData<int16_t>(0x0065db10);
                if (partySize >= 0x80 && held.alliance != 0) check = 0;
            }
        }
        if (isVersus && FP::getData<uint8_t>(0x006d4abe) != 0 && getSecondBasePanelTile() == cursorTile) checkPanelCapture(1);

        //every step of the line, the last one is the landing tile
        const int distance = unit.throwDistance;
        if (distance < 1) return check;
        const DisaMapUnit& cursor = DAT_00730ba0_cursor();
        const float lineX = cursor.position.x - unit.position.x;
        const float lineZ = cursor.position.z - unit.position.z;
        for (int step = 1; step <= distance; ++step) {
            const int x = static_cast<int>(double(unit.position.x) + double(lineX) * double(float(step)) / double(distance));
            const int z = static_cast<int>(double(unit.position.z) + double(lineZ) * double(float(step)) / double(distance));
            const mpdfileTile* tile = FUN_004a51e0_getWorldTileAt(float(x), float(z));
            if (!tile) continue;
            if (tile->padding[1] != 0) check = 0; //No Entry
            //the held unit flies 14 above the unit, the tile height is the average of its corners
            const int cornerHeight = (tile->cornersTop[2] + tile->cornersTop[3] + tile->cornersTop[1] + tile->cornersTop[0]) / 4;
            const float clearance = float(double(unit.restPosition.y) - 14.0 - double(float(cornerHeight)));
            if (clearance > float((unit.throwRange + 1) * 3)) check = 0;
            if (step != distance) continue;
            if (tile->mobility != 0) check = 0;
            if (FUN_00444990_getFog(0, float(x), float(z)) == 0xff) check = 0;
        }
        return check;
    }

    /// Throws the held unit onto the cursor tile, it jumps there in state 0x87.
    void throwHeld(DisaMapUnit& unit, DisaMapUnit& held, glm::ivec2 cursorTile) {
        FUN_0046aab0_playSound(1);
        FUN_0046cf10_snapToTile(DAT_00730ba0_cursor());
        if (FP::getData<int32_t>(0x005d9cd8) == 0) unit.actionState = 2;
        FUN_004de990_endAnimationLoop(unit);
        unit.poseFramesLeft = 0x2d;
        const int landX = cursorTile.x * 12 + 6;
        const float x = float(landX);
        const float z = float(cursorTile.y * 12 + 6);
        const float height = float((unit.throwRange + 2) * 3);
        const float frames = float((unit.throwDistance + 3) * 5);
        FUN_0046c9b0_jumpTo(held, x, FUN_004d2be0_getGroundHeight(nullptr, 0, x, z), z, height, frames);
        held.moveOriginFacing = held.facing;
        held.moveOrigin.x = float(landX);
        held.moveOrigin.y = FUN_004d2be0_getGroundHeight(nullptr, 0, x, z);
        held.moveOrigin.z = z;
        held.isFromBasePanel = false;
        if (held.holdingIndex >= 0) held.isLiftLocked = true;
        FUN_0046aab0_playSound(0x2e);

        FP::getData<int32_t>(0x00889604) = DAT_008895b0_selectedUnit();
        int32_t& thrownUnit = FP::getData<int32_t>(0x00889600);
        thrownUnit = unit.holdingIndex;
        //-2 marks the throw until it landed
        unit.holdingIndex = -2;
        held.heldByIndex = -2;
        FP::getData<int32_t>(0x006772c4) = thrownUnit + 1; //the camera follows the thrown unit
        if (held.moveState == 1) held.moveState = 2;
        FP::getData<int32_t>(0x0088960c) = unit.facing;
        FP::getData<int32_t>(0x00889608) = 0;
        DAT_008896a0_battleStateDepths()[0] = 0;
        DAT_00889618_battleState() = 0x87;
    }

    /**
    * @brief State 0x83, choose where to throw the held unit, into a base panel captures it.
    * Part of FUN_0045c0f0 at 0x004625AF.
    */
    void LAB_004625af_updateThrowTarget() {
        DisaKeyStates& keys = DAT_00778d98_keyStates();
        FUN_00447f00_drawTileInfo();
        FUN_004d9dc0_updateCameraControls(DAT_00730980_camera());
        DisaMapUnit& unit = *DisaMapUnit::get(DAT_008895b0_selectedUnit());
        FUN_00456100_drawThrowWindow(unit);
        aimThrow(unit);

        FUN_004cf290_markThrowLine(unit);
        FUN_004e32a0_rotateXZ(float(unit.facingTarget), 0.f, float(unit.throwDistance * 12));
        const int landX = static_cast<int>(double(unit.position.x) - double(FP::getData<float>(0x006b791c)));
        const int landZ = static_cast<int>(double(unit.position.z) + double(FP::getData<float>(0x006b7a60)));
        DisaMapUnit& cursor = DAT_00730ba0_cursor();
        const float y = FUN_004d2be0_getGroundHeight(nullptr, 0, float(landX), float(landZ));
        cursor.FUN_0046cab0_translateTo(float(landX), y, float(landZ), 8.f);

        //whole world units divided by 12, not toTile
        const glm::ivec2 cursorTile(static_cast<int>(cursor.position.x) / 12, static_cast<int>(cursor.position.z) / 12);
        DisaMapUnit& held = *DisaMapUnit::get(unit.holdingIndex);
        const int check = checkThrow(unit, held, cursorTile);

        if (keys.isKeyPressed(DisaKeyStates::Select) || (FUN_0050f7c0_isLeftMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            DAT_0067726b_isInputConsumed() = 1;
            keys.clear();
            if (check < 0) throwHeld(unit, held, cursorTile);
            else FUN_0046aab0_playSound(4);
        }
        if (keys.isKeyPressed(DisaKeyStates::Cancel) || (FUN_0050f950_isRightMouseClicked() && !FUN_0046de80_isGameFlagSet(0xd48))) {
            keys.clear();
            FUN_0046aab0_playSound(7);
            cursor.FUN_0046cab0_translateTo(unit.position.x, unit.position.y, unit.position.z, 5.f);
            FUN_00446d50_popBattleState(0);
        }
    }

    /// Original function: FUN_00476090, deals damage to the unit and shows the number, ecx is never read.
    void FUN_00476090_dealDamage(DisaMapUnit& unit, int damage, int mode) {
        FP::callFastFarA<void>(0x00476090, 0, &unit, damage, mode);
    }
    /// Original function: FUN_00475e30, merges the thrown unit into the one it landed on, returns the merged unit.
    DisaMapUnit* FUN_00475e30_mergeUnits(DisaMapUnit& base, DisaMapUnit& thrown) {
        return FP::callStdFastFarA<DisaMapUnit*>(0x00475e30, &base, &thrown);
    }
    /// Original function: FUN_00448800, character 0xc80 was thrown into the base panel of the layer.
    void FUN_00448800_enterPanelObject(int8_t layer) { FP::callStdFastFarA<void>(0x00448800, layer, 0); }
    /// Original function: FUN_00448500, undoes the Defend or lift of the unit before it leaves the map.
    void FUN_00448500_undoAction(int index) { FP::callStdFastFarA<void>(0x00448500, index); }
    /// Original function: FUN_004d3fa0, copies the character into the party, the new party index or -1 when it is full.
    int FUN_004d3fa0_joinParty(DisaPartyCharacterInfo* character, int alliance) {
        return FP::callStdFastFarA<int>(0x004d3fa0, character, alliance);
    }

    /// The thrown units keep -2 in heldByIndex and holdingIndex until the throw is over.
    void clearThrowMarks() {
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            DisaMapUnit& unit = *DisaMapUnit::get(i);
            if (!unit.isActive) continue;
            if (unit.heldByIndex == -2) unit.heldByIndex = -1;
            if (unit.holdingIndex == -2) unit.holdingIndex = -1;
        }
    }

    /// A unit of a temporary character, the map enemies at 0x005FE0B0, forgets its character.
    void releaseTemporaryCharacter(DisaMapUnit& unit) {
        const DisaPartyCharacterInfo* temporaryBegin = &FP::getData<DisaPartyCharacterInfo>(0x005fe0b0);
        const DisaPartyCharacterInfo* temporaryEnd = temporaryBegin + 64;
        if (unit.character < temporaryBegin || unit.character >= temporaryEnd) return;
        unit.character->charName.fill(0);
        unit.character = nullptr;
    }

    /**
    * @brief The thrown unit and everything it held landed in the base panel of the layer: own units go back into the
    * panel, others join the party, character 0xc82 counts and leaves.
    * Part of FUN_0045c0f0 at 0x004632A8 (layer 0) and 0x0046351C (layer 1).
    */
    void enterBasePanel(DisaMapUnit& thrown, int8_t layer) {
        int16_t& joinedIndex = FP::getData<int16_t>(0x0088a7e4);
        joinedIndex = -1;
        DisaMapUnit* unit = &thrown;
        while (true) {
            if (unit->characterId == 0xc80) {
                FUN_00448800_enterPanelObject(layer);
            }
            else if (unit->characterId == 0xc82) {
                int8_t& count = FP::getData<int8_t>(layer == 0 ? 0x006d4abc : 0x006d4abd);
                count += count < 99 ? 1 : 0;
                const uint8_t* message = getSystemString(layer == 0 ? 0x970 : 0x9b8);
                FUN_00477550_showMessage(-1, 0x3a, -1, 1, 0x5a, &FP::getData<uint8_t>(0x005ad538), message);
                unit->isActive = false;
                releaseTemporaryCharacter(*unit);
            }
            else if (unit->alliance == layer) {
                FUN_00448500_undoAction(FP::getData<int32_t>(0x00889600));
                if (layer == 1) ++FP::getData<int8_t>(0x006d4ab6); //units of layer 1 in its panel
                reinterpret_cast<uint8_t*>(unit->character)[0x69c] = unit->actionState;
                reinterpret_cast<uint8_t*>(unit->character)[0x69d] = unit->moveState;
                unit->isActive = false;
                if (layer == 0) ++FP::getData<int8_t>(0x0065db39);
            }
            else {
                joinedIndex = static_cast<int16_t>(FUN_004d3fa0_joinParty(unit->character, unit->alliance));
                if (joinedIndex >= 0) unit->isVisible = false;
                else unit->heldByIndex = -1;
            }
            if (unit->holdingIndex < 0) break;
            unit = DisaMapUnit::get(unit->holdingIndex);
        }
        FP::getData<int32_t>(0x006772c4) = 0;
        clearThrowMarks();
        if (joinedIndex < 0) {
            DAT_008896a0_battleStateDepths()[0] = 0;
            thrown.poseFramesLeft = 10;
            DAT_00889618_battleState() = 0;
        }
        else {
            //a new party member, state 0x8C names it
            std::array<int16_t, 4>& joinValues = FP::getData<std::array<int16_t, 4>>(0x0088a7d8);
            joinValues.fill(0);
            DAT_00889618_battleState() = 0x8c;
            FP::getData<int16_t>(0x0088a7e8) = 0;
            FP::getData<int16_t>(0x0088a7e0) = 0;
            DAT_008896a0_battleStateDepths()[0] = 0;
        }
        FUN_004d2ad0_updateGeoPanel(thrown);
        FUN_004da320_refreshUnits();
        FUN_00444810_updateFog(0);
        FUN_00444810_updateFog(1);
    }

    /// The exploding characters 2210 to 2219 and 3170, outside of dungeons 1100 to 1199.
    bool isExploding(int16_t characterId) {
        return (characterId / 10 == 221 || characterId == 0xc62) && FP::getData<DisaDungeon>(0x00653a98).id / 100 != 11;
    }

    /**
    * @brief The thrown unit landed on another unit: geo symbols and 0xc80 take their HP as damage, others merge.
    * Part of FUN_0045c0f0 at 0x004637C4.
    */
    void landOnUnit(DisaMapUnit& thrown, DisaMapUnit& base) {
        int damaged = 0;
        if (thrown.isGeoSymbol != 0) {
            FUN_00476090_dealDamage(thrown, static_cast<int32_t>(thrown.character->currentHP), 1);
            damaged = 1;
        }
        if (base.isGeoSymbol != 0) {
            FUN_00476090_dealDamage(base, static_cast<int32_t>(base.character->currentHP), 1);
            ++damaged;
        }
        if (thrown.characterId == 0xc80) {
            FUN_00476090_dealDamage(base, static_cast<int32_t>(base.character->currentHP), 1);
            ++damaged;
        }
        if (damaged == 0) {
            DisaMapUnit& merged = *FUN_00475e30_mergeUnits(base, thrown);
            DisaTransform* transform = getEffectTransform();
            transform->position.x = merged.position.x;
            transform->position.y = float(double(merged.position.y) - 6.0);
            transform->position.z = merged.position.z;
            FUN_004783d0_spawnEffect(1000, 0xb, transform);
            FUN_0046aab0_playSound(0x17);
            merged.pose = 9;
            merged.poseFramesLeft = 0x3c;
        }
        FUN_004d2ad0_updateGeoPanel(thrown);
        FUN_004da320_refreshUnits();
    }

    /**
    * @brief State 0x87, waits until the thrown unit came down, then it lands in a base panel, on a unit, explodes or
    * ends the Item World floor on the gate.
    * Part of FUN_0045c0f0 at 0x0046316D.
    */
    void LAB_0046316d_updateThrowLanding() {
        DisaMapUnit& thrown = *DisaMapUnit::get(FP::getData<int32_t>(0x00889600));
        if (thrown.moveFramesLeft > 0 || thrown.velocity.y < 0.f) return;

        thrown.gravity = std::bit_cast<float>(0x3e26e979u); //0.163
        FP::getData<uint32_t>(0x00730a8c) = 0xf0008; //camera + 0x10c
        DisaTransform* transform = getEffectTransform();
        FP::getData<int32_t>(0x006772a0) = 200; //read by the effect
        //the 1 for y is the constant the dispatch left on the FPU stack
        transform->position = glm::vec4(thrown.position.x, thrown.restPosition.y - 1.f, thrown.position.z, 1.f);
        transform->rotation = glm::vec4(0.f, 0.f, 0.f, 1.f);
        transform->scale = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
        FUN_00478d50_spawnEffect(-1, 0x1b, transform, 1.f, 1);
        FUN_0046aab0_playSound(0x22);

        const glm::ivec2 tile(toTile(thrown.position.x), toTile(thrown.position.z));
        if (getBasePanelTile() == tile && FP::getData<uint8_t>(0x0065db43) != 0) {
            enterBasePanel(thrown, 0);
            return;
        }
        if (DAT_0065db3a_gameMode() == 5 && FP::getData<uint8_t>(0x006d4abe) != 0 && getSecondBasePanelTile() == tile) {
            enterBasePanel(thrown, 1);
            return;
        }

        for (int i = 0; i < DisaMapUnit::count; ++i) {
            DisaMapUnit& base = *DisaMapUnit::get(i);
            if (!base.isActive || &base == &thrown || base.heldByIndex >= 0) continue;
            //an exploding unit does not land on others, it explodes below
            if (isExploding(thrown.characterId)) continue;
            if (FUN_0046f640_getCondition(base.character) == 2) continue;
            if (toTile(base.position.x) != tile.x || toTile(base.position.z) != tile.y) continue;
            landOnUnit(thrown, base);
            break;
        }

        //here the original reads the id of the character, above the one of the unit
        const int16_t characterId = static_cast<int16_t>(thrown.character->characterId);
        if (isExploding(characterId)) {
            ++FP::getData<int16_t>(0x0065db1e);
            FP::getData<int32_t>(0x0088952c) = 0x5a;
            const float y = float(double(FUN_004d2be0_getGroundHeight(nullptr, 0, thrown.position.x, thrown.position.z)) - 3.0);
            const int power = static_cast<int32_t>(thrown.character->currentHP) / 2 + 1;
            FUN_00476d90_spawnProjectile(-1, 10, power, thrown.position.x, y, thrown.position.z, 0.f, 0.f, 0.f, 0);
            FUN_00476090_dealDamage(thrown, static_cast<int32_t>(thrown.character->currentHP), 1);
            thrown.pose = 5;
            thrown.poseFramesLeft = 10;
        }
        else if (thrown.alliance == 0 && getGateTile() == tile && FP::getData<uint8_t>(0x0065db41) != 0) {
            enterGate(thrown);
            FUN_004d2ad0_updateGeoPanel(thrown);
            FUN_004da320_refreshUnits();
            return;
        }

        FUN_00444810_updateFog(0);
        FUN_00444810_updateFog(1);
        DAT_008896a0_battleStateDepths()[0] = 0;
        thrown.poseFramesLeft = 10;
        DAT_00889618_battleState() = 0;
        FP::getData<int32_t>(0x006772c4) = 0;
        clearThrowMarks();
        FUN_004d2ad0_updateGeoPanel(thrown);
        FUN_004da320_refreshUnits();
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
        case 0x1e: LAB_0045dcf9_updateAttackTarget(frame); break;
        case 0x3e: LAB_0045e59d_updateSkillTarget(); break;
        case 0x47: LAB_004602e6_updateItemTarget(); break;
        case 0x48: LAB_004607ea_updateSteal(frame); break;
        case 0x4b: LAB_0046196c_updateLeavePrompt(frame); break;
        case 0x50: LAB_00461af0_updateSummonShots(frame); break;
        case 0x82: LAB_00461fd5_updateLiftTarget(frame); break;
        case 0x83: LAB_004625af_updateThrowTarget(); break;
        case 0x87: LAB_0046316d_updateThrowLanding(); break;
        case 0x0a: LAB_0045d06e_updateCommandMenu(); break;
        case 0x28: LAB_0045e546_updateBattleStart(); break;
        case 0x3c: LAB_0045e57d_updateSpecialList(); break;
        case 0x3d: LAB_0045e58d_updateState3d(); break;
        case 0x46: LAB_004602d8_updateItemList(); break;
        case 0x4c: LAB_00461ad0_updateVersusItemList(); break;
        case 0x4d: LAB_00461ae0_updateState4d(); break;
        case 0x8c: LAB_00463c6e_updateState8c(); break;
        case 0x96: LAB_00463c8e_updateGeoChain(); break;
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

    // ---- The drawing, from LAB_00465035 to the end ----

    /// Versus mode: the replay input, the battle of layer 1 and the turn timer. Part of FUN_0045c0f0 at 0x0046503F.
    void LAB_0046503f_updateVersusLayer(const BattleFrame& frame) {
        if (DAT_0065db3a_gameMode() != 5) return;
        FUN_004fd180_updateVersusInput(frame.isLayerIdle);
        FUN_004576c0_updateLayer1();
        const uint8_t layer = DAT_006d4ab2_turnLayer();
        if (layer == 0 || layer == 1) FUN_00445300_tickVersusTimer();
    }

    /**
    * @brief Once layer 0 waits for input, or layer 1 on its turn: plays the queued geo panel changes (state 0x96) or adds
    * the pending bonus points (state 0xA0). Part of FUN_0045c0f0 at 0x00465068.
    */
    void LAB_00465068_startPendingStates() {
        if (DAT_006b7578_busyCount() != 0) return;
        std::array<int32_t, 2>& states = DAT_00889618_battleStates();
        if (states[0] != 0 && (DAT_006d4ab2_turnLayer() != 1 || states[1] != 0)) return;
        if (DAT_006645f4_geoChainCount() > 0) {
            DAT_006645fc_geoChainIndex() = 0;
            states = { 0x96, 0x3ea };
        }
        else if (DAT_00889584_bonusPointsPending() >= 1) {
            states = { 0xa0, 0xa0 };
        }
        else {
            return;
        }
        DAT_008896a0_battleStateDepths() = { 0, 0 };
    }

    static_assert(offsetof(DisaCameraInfo, field_0xf8) == 0xf8);

    /**
    * @brief The chunk rotation, the background, the fullscreen image and the map chunks with their event tiles.
    * Part of FUN_0045c0f0 at 0x004650E6.
    */
    void LAB_004650e6_drawMap() {
        DisaCameraInfo& camera = DAT_00730980_camera();
        FUN_004d9cb0_updateChunkRotation(FP::getData<int32_t>(0x008895a4));
        DisaRenderFlags& flags = FUN_0043cec0_pushRenderFlags();
        flags.castsShadow = 0;
        flags.isFullyLit = 0;
        const int32_t background = FP::getData<int32_t>(0x006b7564);
        if (background == 0) FUN_004cfa80_drawBackground();
        else FUN_004cfd50_drawBackground(background);
        popRenderFlags();

        const int32_t image = FP::getData<int32_t>(0x0067728c); //a fullscreen image, -1 none
        if (image >= 0) {
            const uint8_t* entry = &FP::getData<uint8_t>(0x00749550) + image * 0x88;
            const int16_t imageWidth = *reinterpret_cast<const int16_t*>(entry + 0x4e);
            const int16_t imageHeight = *reinterpret_cast<const int16_t*>(entry + 0x50);
            FUN_0050e5d0_drawImage(entry, 0, static_cast<int>(camera.field_0xf8), 0xbaa1, 0x1e0, 0x110, imageWidth, imageHeight);
        }

        const int8_t* chunkStates = &FP::getData<int8_t>(0x0065db48); //2 and up are part of the world grid
        for (int i = 0; i < DisaMapChunk::count; ++i) {
            DisaMapChunk* chunk = DisaMapChunk::get(i);
            if (!chunk->isActive || chunkStates[i] < 2) continue;
            for (const mpdfileChunkEventTile& tile : chunk->eventTiles) {
                if (tile.index != 0) FUN_004a1c60_markTile(static_cast<uint8_t>(tile.x), static_cast<uint8_t>(tile.z), 2);
            }
            FUN_004a4c60_SubmitMapChunk(chunk, 0, &camera);
        }
    }

    /// The y a map cursor floats at, it bobs with the 60 frame counter DAT_006b7918.
    float getCursorFloatY(float groundY) {
        const float bob = float(std::abs(FP::getData<int32_t>(0x006b7918) - 30));
        return float(double(groundY) - double(bob) / 5.0 - 2.0);
    }

    /// Puts the map cursor of the layer on the ground, in state 0x50 it hangs 5 above the ground.
    void placeCursor(DisaMapUnit& cursor, int layer) {
        FUN_0046c830_updateActor(cursor, DAT_00730980_camera());
        int chunkIndex = -1;
        const float groundY = FUN_004d2be0_getGroundHeight(&chunkIndex, 0, cursor.position.x, cursor.position.z);
        if (chunkIndex >= 0) {
            cursor.restPosition.y = groundY;
            cursor.chunkIndex = static_cast<int8_t>(chunkIndex);
        }
        if (DAT_00889618_battleStates()[layer] == 0x50) cursor.position.y = float(double(cursor.restPosition.y) - 5.0);
        else cursor.position.y = getCursorFloatY(groundY);
    }

    /// Bit 0 of DAT_00664f04 hides the cursors.
    bool areCursorsHidden() { return (FP::getData<uint32_t>(0x00664f04) & 1) != 0; }

    /// Both map cursors and the marks of their tiles. Part of FUN_0045c0f0 at 0x004651FF.
    void LAB_004651ff_drawCursors() {
        DisaCameraInfo& camera = DAT_00730980_camera();
        if (DAT_0065db3a_gameMode() == 5) {
            DisaMapUnit& cursorB = DAT_00778a10_cursorB();
            placeCursor(cursorB, 1);
            if (!areCursorsHidden() && FUN_00444990_getFog(0, cursorB.position.x, cursorB.position.z) != 0xff) {
                FUN_0046a810_drawActor(cursorB, camera, 0x96, 0, 0);
                if (DAT_00889618_battleStates()[1] != 0x50) {
                    FUN_004a2050_markCursorTile(toTile(cursorB.position.x), toTile(cursorB.position.z), 0);
                }
            }
        }
        DisaMapUnit& cursor = DAT_00730ba0_cursor();
        placeCursor(cursor, 0);
        if (areCursorsHidden()) return;
        FUN_0046a810_drawActor(cursor, camera, 0x96, 0, DAT_005d7c40_settings()->field_0x21);
        if (DAT_00889618_battleState() != 0x50) FUN_004a2050_markCursorTile(toTile(cursor.position.x), toTile(cursor.position.z), 0);
    }

    /// Draws a unit with FUN_0046a810, DAT_005d9cd4 carries settings byte 0x2b into the draw.
    void drawUnitSprite(DisaMapUnit& unit, int isFogged) {
        uint32_t& drawSetting = FP::getData<uint32_t>(0x005d9cd4);
        const DisaSettings* settings = DAT_005d7c40_settings();
        drawSetting = FP::getData<uint8_t*>(0x005d7c70) != nullptr && settings->field_0x2b != 0 ? settings->field_0x2b : 0;
        FUN_0046a810_drawActor(unit, DAT_00730980_camera(), 0, isFogged, 0);
        drawSetting = 0;
    }

    /// A unit that blinked out drops the unit it held and leaves GEO effect 1. Part of FUN_0045c0f0 at 0x0046550F.
    void LAB_0046550f_removeUnit(DisaMapUnit& unit, uint32_t& fogRefresh) {
        if (unit.alliance == 0) fogRefresh |= 1;
        else if (unit.alliance == 1) fogRefresh |= 2;
        if (unit.holdingIndex >= 0) {
            DisaMapUnit::get(unit.holdingIndex)->heldByIndex = -1;
            DisaMapUnit::get(unit.holdingIndex)->poseFramesLeft = 5;
            unit.holdingIndex = -1;
        }
        DisaTransform* transform = getEffectTransform();
        transform->position = glm::vec4(unit.position, 1.f);
        transform->rotation = glm::vec4(0.f, 0.f, 0.f, 1.f);
        transform->scale = glm::vec4(1.f);
        FUN_00478d50_spawnEffect(-1, 1, transform, 1.f, 1);
        FUN_004da320_refreshUnits();
    }

    /**
    * @brief Updates, counts and draws one unit that is not placed by script. The body of the loop at 0x00465462.
    * @param unitAlpha The alpha of the unit fade, kept from the last fading unit when DAT_00889614 is 0.
    * @param fogRefresh Gets bit 0 or 1 when a unit of alliance 0 or 1 was removed.
    */
    void drawUnit(int index, int& unitAlpha, uint32_t& fogRefresh) {
        DisaMapUnit& unit = *DisaMapUnit::get(index);
        DisaCameraInfo& camera = DAT_00730980_camera();
        if (unit.modelType != 0 && DAT_00889618_battleState() != 0x50) FUN_004d2e40_aimModel(unit);
        //indexed by alliance without a range check, like the original
        DAT_006b757c_aliveCounts().data()[unit.alliance] += FUN_0046f640_getCondition(unit.character) != 2 ? 1 : 0;

        int chunkIndex = -1;
        const float groundY = FUN_004d2be0_getGroundHeight(&chunkIndex, 0, unit.position.x, unit.position.z);
        if (chunkIndex >= 0) {
            unit.restPosition.y = groundY;
            unit.chunkIndex = static_cast<int8_t>(chunkIndex);
        }
        if (FUN_0046c660_updateRemoval(unit, camera)) LAB_0046550f_removeUnit(unit, fogRefresh);
        if (unit.heldByIndex >= 0) {
            const DisaMapUnit& holder = *DisaMapUnit::get(unit.heldByIndex);
            unit.position.x = holder.position.x;
            const float lift = holder.spriteId == 1 ? 1.f : 0.f;
            unit.position.y = float(double(holder.position.y) - 14.0 + double(lift));
            unit.position.z = holder.position.z;
        }
        if (unit.field0x277 != 0) {
            const int32_t fadeMode = DAT_00889614_fadeMode();
            if (fadeMode == 1) unitAlpha = DAT_00889610_fadeFrames() * 6;
            else if (fadeMode == 2) unitAlpha = (20 - DAT_00889610_fadeFrames()) * 6;
            unit.color.w = static_cast<int16_t>(unitAlpha - (unitAlpha > 0xff ? 1 : 0));
        }

        //DAT_005d7c90, per unit, nonzero shows it through the fog
        const bool isSeen = unit.alliance == 0 || FUN_00444990_getFog(0, unit.position.x, unit.position.z) != 0xff
            || (&FP::getData<uint8_t>(0x005d7c90))[index] != 0;
        if (!isSeen) {
            if (unit.drawMode != 2) drawUnitSprite(unit, 1);
            return;
        }
        if (unit.drawMode == 2) FUN_0046a7e0_queueActorDraw(unit, camera);
        else drawUnitSprite(unit, 0);
        if (unit.isGeoSymbol == 0 && unit.characterId / 10 != 320) FUN_0046bf40_drawHpBar(unit, camera);
        if (!unit.isVisible || DAT_00889614_fadeMode() != 0) return;
        FUN_0046bd20_drawStatusIcons(unit, unit.character, camera);
        if (unit.alliance == 0) FUN_0046c4c0_drawDoneMarker(unit, camera);
    }

    /**
    * @brief Updates and draws the units, counts the ones that can still fight and rebuilds the fog after a removal.
    * Part of FUN_0045c0f0 at 0x00465434.
    */
    void LAB_00465434_drawUnits(int& unitAlpha) {
        FUN_004a2280_submitTileMarks();
        DAT_006b757c_aliveCounts() = { 0, 0, 0, 0 };
        FP::getData<int32_t>(0x006b7910) = FP::getData<int32_t>(0x008ad894);
        uint32_t fogRefresh = 0;
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            const DisaMapUnit& unit = *DisaMapUnit::get(i);
            if (unit.isActive && !unit.isRawPositioned) drawUnit(i, unitAlpha, fogRefresh);
        }
        FUN_004e0140_submitUnitQueue();
        if (fogRefresh == 0) return;
        FUN_00444810_updateFog(0);
        FUN_00444810_updateFog(1);
    }

    /// Counts the unit fade down, the end of a fade in shows the units fully again. Part of FUN_0045c0f0 at 0x0046580F.
    void LAB_0046580f_updateUnitFade() {
        int32_t& frames = DAT_00889610_fadeFrames();
        if (frames <= 0 || --frames != 0 || DAT_00889614_fadeMode() != 2) return;
        DAT_00889614_fadeMode() = 0;
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            DisaMapUnit& unit = *DisaMapUnit::get(i);
            if (!unit.isActive || unit.field0x277 == 0) continue;
            unit.field0x277 = 0;
            unit.color.w = 0xff;
        }
    }

    /// The units placed by script SetActorRawPos. Part of FUN_0045c0f0 at 0x00465861.
    void LAB_00465861_drawRawUnits() {
        DisaCameraInfo& camera = DAT_00730980_camera();
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            DisaMapUnit& unit = *DisaMapUnit::get(i);
            if (!unit.isActive || !unit.isRawPositioned) continue;
            if (unit.drawMode == 2) {
                FUN_0046a7e0_queueActorDraw(unit, camera);
                continue;
            }
            FUN_0046d7a0_drawRawActor(unit, static_cast<int>(unit.position.x), static_cast<int>(unit.position.y),
                static_cast<int>(unit.position.z), &FP::getData<uint8_t>(0x005ad534));
        }
    }

    /// The script actors and the target marker. Part of FUN_0045c0f0 at 0x004658C9.
    void LAB_004658c9_drawTargetMarker() {
        FUN_004c12d0_updateScriptActors();
        if (DAT_00701f3b_isTargetMarkerShown() == 0) return;
        DisaMapUnit& marker = DAT_00701cd0_targetMarker();
        DisaCameraInfo& camera = DAT_00730980_camera();
        FUN_0046c830_updateActor(marker, camera);
        if (FUN_00444990_getFog(0, marker.position.x, marker.position.z) == 0xff) return;
        FUN_0046a810_drawActor(marker, camera, 0x96, 0, DAT_005d7c40_settings()->field_0x21);
    }

    /**
    * @brief Type 2 projectiles burst on the unit: GEO effect 0x24 at a random offset and size and sound 0x131.
    * Part of FUN_0045c0f0 at 0x00465D8F.
    * @details Only position.xyz, rotation and scale are written, position.w keeps the 1 of the damage number.
    */
    void LAB_00465d8f_spawnBurst(const DisaMapUnit& unit) {
        DisaTransform* transform = getEffectTransform();
        const auto randomOffset = []() { return float(double(float(FUN_0046e1a0_random(0x3d) - 30)) / 10.0); };
        transform->position.x = float(double(randomOffset()) + double(unit.position.x));
        transform->position.y = float(double(randomOffset()) + double(unit.position.y) - 5.0);
        transform->position.z = float(double(randomOffset()) + double(unit.position.z));
        transform->rotation.z = 0.f;
        transform->rotation.y = 0.f;
        transform->rotation.x = 0.f;
        //the sum is converted as unsigned
        const float size = float(double(float(double(static_cast<uint32_t>(FUN_0046e1a0_random(0x29) + 0x50)))) / 100.0);
        FP::getData<int32_t>(0x006772a0) = 0x8ae; //read by the effect
        transform->scale.z = size;
        transform->scale.y = size;
        transform->scale.x = size;
        transform->scale.w = 1.f;
        transform->rotation.w = 1.f;
        FUN_00478d50_spawnEffect(-1, 0x24, transform, 1.f, 1);
        FUN_004de090_playSound(0x131, 0x46, 25000, 0);
        FP::getData<int32_t>(0x00730a8c) = 0x80003; //camera + 0x10c
    }

    /**
    * @brief One hit of a projectile: the damage, the explosion of characters 2210 to 2219 and 3170, the damage number and
    * the burst of type 2. Part of FUN_0045c0f0 at 0x00465B01.
    * @param hit Like the stack local of the original, fields FUN_0046f510 leaves alone keep the last hit of the frame.
    */
    void LAB_00465b01_hitUnit(DisaProjectile& projectile, int index, DisaHit& hit) {
        DisaMapUnit& unit = *DisaMapUnit::get(index);
        DisaCameraInfo& camera = DAT_00730980_camera();
        DisaTransform* transform = getEffectTransform();
        FUN_0046f510_clearAction(reinterpret_cast<uint8_t*>(&hit));
        int32_t damage = projectile.power;
        if (projectile.type != 10) {
            //unsigned like the original
            const uint32_t roll = static_cast<uint32_t>(FUN_0046e1a0_random(0x1f) + 30);
            const uint32_t blocked = roll * unit.character->activeStats.defence / 100u;
            damage = std::max(static_cast<int32_t>(static_cast<uint32_t>(projectile.power) - blocked) / 5, 1);
        }
        if (unit.characterId == 0xc80) hit.damage = 0;
        else hit.damage = FUN_00444e10_hasVersusRule(2, unit.alliance) ? 0 : damage;
        const int16_t owner = projectile.owner;
        //owners 64 to 99 point past the units like in the original
        DisaMapUnit* attacker = owner >= 0 && owner < 100 ? DisaMapUnit::get(0) + owner : nullptr;
        FUN_00476160_applyHit(attacker, unit, hit, 0, 0);

        const int16_t characterId = static_cast<int16_t>(unit.character->characterId);
        const bool isExploding = characterId / 10 == 221 || static_cast<uint16_t>(characterId) == 0xc62;
        if (isExploding && FP::getData<DisaDungeon>(0x00653a98).id / 100 != 11) {
            ++FP::getData<int16_t>(0x0065db1e);
            FP::getData<int32_t>(0x0088952c) = 0x5a;
            const int32_t power = static_cast<int32_t>(unit.character->currentHP) / 2 + 1;
            FUN_00476d90_spawnProjectile(index, 10, power, unit.position.x, float(double(unit.position.y) - 3.0), unit.position.z,
                0.f, 0.f, 0.f, 0);
            FUN_0046f510_clearAction(reinterpret_cast<uint8_t*>(&hit));
            hit.damage = static_cast<int32_t>(unit.character->currentHP);
            FUN_00476160_applyHit(nullptr, unit, hit, 0, 1);
            unit.pose = 5;
            unit.poseFramesLeft = 10;
            transform->position = glm::vec4(unit.position.x, unit.restPosition.y, unit.position.z, 1.f);
            FUN_004d0850_showNumber(0, hit.damage, transform);
        }
        transform->position = glm::vec4(unit.position.x, float(double(unit.position.y) - 1.0), unit.position.z, 1.f);
        FUN_004d0850_showNumber(0, hit.damage, transform);
        ++FP::getData<int32_t>(0x0088958c);
        int8_t& hitCounter = reinterpret_cast<int8_t&>(unit.field0x279[0]);
        hitCounter -= hitCounter > 0 ? 1 : 0;
        if (projectile.type == 2 && unit.poseFramesLeft == 0) LAB_00465d8f_spawnBurst(unit);
    }

    /**
    * @brief Tests the projectile against every unit in reach and hits them. Part of FUN_0045c0f0 at 0x004659F8.
    * @details A unit index as owner spares that unit, 200 + alliance spares the alliance except characters 2210 to 2219
    * and 3170. The distance is measured from 5 above the unit.
    */
    void LAB_004659f8_hitUnits(DisaProjectile& projectile, DisaHit& hit) {
        int hitCount = 0;
        for (int i = 0; i < DisaMapUnit::count; ++i) {
            DisaMapUnit& unit = *DisaMapUnit::get(i);
            if (!unit.isActive || !unit.isVisible || unit.heldByIndex >= 0) continue;
            if (FUN_0046f640_getCondition(unit.character) == 2) continue;
            const int16_t owner = projectile.owner;
            if (owner >= 200) {
                const int16_t characterId = static_cast<int16_t>(unit.character->characterId);
                if (characterId / 10 != 221 && characterId != 0xc62 && unit.alliance == owner - 200) continue;
            }
            if (i == owner) continue;
            //the x87 code keeps the sum in double, the differences are stored as float
            const float dx = unit.position.x - projectile.position.x;
            const float dy = float(double(unit.position.y) - 5.0 - double(projectile.position.y));
            const float dz = unit.position.z - projectile.position.z;
            const float distanceSquared = float(double(dy) * dy + double(dx) * dx + double(dz) * dz);
            if (distanceSquared > projectile.hitRadiusSquared) continue;
            LAB_00465b01_hitUnit(projectile, i, hit);
            ++hitCount;
            if (unit.poseFramesLeft == 0) {
                unit.pose = 5;
                unit.poseFramesLeft = 6;
            }
        }
        if (hitCount <= 0) return;
        projectile.isActive = 0;
        projectile.field0xa8 = 0;
    }

    /**
    * @brief Removes the projectiles below the ground, outside of the map or out of time, lets the others hit and draws
    * them. Part of FUN_0045c0f0 at 0x0046592C.
    */
    void LAB_0046592c_updateProjectiles() {
        DisaHit hit{};
        for (DisaProjectile& projectile : DAT_00730f10_projectiles()) {
            if (projectile.isActive == 0) continue;
            const float x = projectile.position.x;
            const float z = projectile.position.z;
            const float groundY = float(FUN_004d2e00_getTileHeight(toTile(x), toTile(z)));
            //y grows downwards
            const bool isOutside = projectile.position.y > groundY || x < -12.f || z < -12.f || x > 780.f || z > 780.f;
            if (isOutside || (projectile.lifetime > 0 && --projectile.lifetime <= 0)) {
                projectile.isActive = 0;
                continue;
            }
            if (projectile.delay > 0) --projectile.delay;
            else LAB_004659f8_hitUnits(projectile, hit);
            if (FUN_00444990_getFog(0, projectile.position.x, projectile.position.z) == 0xff) continue;
            FUN_0046ac00_drawProjectile(projectile, DAT_00730980_camera());
        }
    }

    /**
    * @brief Ends the battle. In versus mode once a layer has no units left, otherwise the game over when the player has
    * none, or the victory (FUN_00446e20) when the enemies have none. Part of FUN_0045c0f0 at 0x00465F8D.
    */
    void LAB_00465f8d_checkBattleEnd() {
        const int32_t state = DAT_00889618_battleState();
        if (DAT_0065db3a_gameMode() == 5) {
            if (state != 0 && state != 1 && state != 0x17) return;
            if (FUN_00444760_countProjectiles() != 0) return;
            const bool isLayer0Defeated = FUN_004449f0_isLayerDefeated(0);
            const bool isLayer1Defeated = FUN_004449f0_isLayerDefeated(1);
            if (isLayer0Defeated) FUN_00444a80_endVersusBattle(isLayer1Defeated ? 2 : 0);
            else if (isLayer1Defeated) FUN_00444a80_endVersusBattle(1);
            return;
        }
        const std::array<int16_t, 4>& alive = DAT_006b757c_aliveCounts();
        if (alive[0] + DAT_0065db39_basePanelCount() == 0) {
            if (state != 0) return;
            //talk script 0x5DD may replace the game over
            if (FUN_004bf7e0_runTalkScriptOriginal(0x5dd, 0, 0, 0) < 0) {
                DAT_008896b0_stateFrames() = 0;
                FUN_0046a830_playMusic(0x68);
                FUN_00444740_setBattleState(600, 0);
            }
            else {
                FUN_00444740_setBattleState(0x3ea, 0);
            }
            return;
        }
        constexpr std::array<int32_t, 5> busyStates = { 200, 0x28, 0x96, 0xa0, 0x50 };
        if (state >= 500 || std::ranges::find(busyStates, state) != busyStates.end()) return;
        //DAT_0065db3f, the alliance to defeat: 0 alliance 1, 1 alliance 3
        const uint8_t targetAlliance = FP::getData<uint8_t>(0x0065db3f);
        if ((alive[1] == 0 && targetAlliance == 0) || (alive[3] == 0 && targetAlliance == 1)) FUN_00446e20(0, 0);
    }

    /**
    * @brief Draws a looping GEO model standing on the ground, the base panels and the Item World gate.
    * @details Like the original only position, rotation.xyz and scale.xyz of the transform are written.
    */
    void drawGroundModel(DisaGeoModel& model, glm::vec2 position, int16_t frame, float param9) {
        DisaTransform* transform = getEffectTransform();
        transform->position.x = position.x;
        transform->position.y = FUN_004d2be0_getGroundHeight(nullptr, 0, position.x, position.y);
        transform->position.z = position.y;
        transform->position.w = 1.f;
        transform->rotation.z = 0.f;
        transform->rotation.y = 0.f;
        transform->rotation.x = 0.f;
        transform->scale.z = 1.f;
        transform->scale.y = 1.f;
        transform->scale.x = 1.f;
        FUN_0043cec0_pushRenderFlags().useLighting = 0;
        FUN_004e13a0_renderGeoModel(model, float(frame), DAT_00730980_camera(), transform, param9);
        popRenderFlags();
    }

    /// The next animation frame of a looping GEO model.
    void nextModelFrame(int16_t& frame, const DisaGeoModel& model) {
        ++frame;
        if (frame >= model.animationFrameMax) frame = 0;
    }

    /// The base panels and the Item World gate. Part of FUN_0045c0f0 at 0x004660B7.
    void LAB_004660b7_drawPanels() {
        if (FP::getData<uint8_t>(0x0065db43) != 0) { //the base panel is on the map
            DisaGeoModel& model = FP::getData<DisaGeoModel>(0x00730ab8);
            int16_t& frame = FP::getData<int16_t>(0x0065db0a);
            drawGroundModel(model, DAT_0065dae8_basePanel(), frame, 10.f);
            nextModelFrame(frame, model);
        }
        if (DAT_0065db3a_gameMode() == 5 && FP::getData<uint8_t>(0x006d4abe) != 0) { //the base panel of layer 1
            DisaGeoModel& model = FP::getData<DisaGeoModel>(0x00730e28);
            int16_t& frame = FP::getData<int16_t>(0x006d4ab0);
            const glm::vec2 panel = FP::getData<glm::vec2>(0x006e57a0);
            if (FUN_00444990_getFog(0, panel.x, panel.y) != 0xff) drawGroundModel(model, panel, frame, 10.f);
            nextModelFrame(frame, model);
        }
        if (FP::getData<uint8_t>(0x0065db41) != 0 && FP::getData<uint8_t>(0x0065db40) == 0) { //the Item World gate
            DisaGeoModel& model = FP::getData<DisaGeoModel>(0x00749210);
            int16_t& frame = FP::getData<int16_t>(0x0065db08);
            const glm::vec2 gate = FP::getData<glm::vec2>(0x0065dae0);
            if (FUN_00444990_getFog(0, gate.x, gate.y) != 0xff) drawGroundModel(model, gate, frame, 100.f);
            nextModelFrame(frame, model);
        }
    }

    /// The text at DAT_008896c0 is shown while nothing moves. Part of FUN_0045c0f0 at 0x004663DA.
    void LAB_004663da_drawInfoText() {
        const uint8_t* text = &FP::getData<uint8_t>(0x008896c0);
        const bool hasText = isOldTextStyle() ? text[0] != 0 : FUN_004e3790_getStringLength(text) != 0;
        if (hasText && DAT_006b7578_busyCount() == 0) FUN_00453bc0_drawInfoText();
    }

    /**
    * @brief Draws the map, the cursors, the units and the projectiles, and ends the battle once a side is defeated.
    * Part of FUN_0045c0f0, from LAB_00465035 to the end. The stack cookie check at the end is dropped.
    */
    void LAB_00465035_drawBattleMap(BattleFrame& frame) {
        FUN_004da020_updateCameraSpin(DAT_00730980_camera());
        LAB_0046503f_updateVersusLayer(frame);
        LAB_00465068_startPendingStates();
        LAB_004650e6_drawMap();
        //the original keeps the fade alpha in [esp+8], which the chunk loop leaves at the chunk count
        int unitAlpha = DisaMapChunk::count;
        LAB_004651ff_drawCursors();
        LAB_00465434_drawUnits(unitAlpha);
        LAB_0046580f_updateUnitFade();
        LAB_00465861_drawRawUnits();
        LAB_004658c9_drawTargetMarker();
        LAB_0046592c_updateProjectiles();
        LAB_00465f8d_checkBattleEnd();
        LAB_004660b7_drawPanels();
        LAB_004663da_drawInfoText();
    }
}

void FUN_0045c0f0_updateBattleMap() {
    BattleFrame frame;
    LAB_0045c0f0_updateCameraAndHover(frame);
    LAB_0045c6e6_updateBattleState(frame);
    LAB_00465035_drawBattleMap(frame);
}
