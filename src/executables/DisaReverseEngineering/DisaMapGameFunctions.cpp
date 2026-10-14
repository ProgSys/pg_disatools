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

/*
* Port of FUN_0045c0f0. Globals keep their address in the name (DAT_xxxxxxxx_name), parts of the original function
* are named after the address of their first instruction (LAB_xxxxxxxx_name) and callees after their own address
* (FUN_xxxxxxxx_name), so all of them can be found again in Ghidra, the disassembly and memory dumps.
*/
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

    // ---- Game functions ----

    /// Original function: FUN_00444810, rebuilds the versus fog of war of the layer and the map overlay.
    void FUN_00444810_updateFog(int8_t layer) { FP::callStdFastFarA<void>(0x00444810, layer); }
    /// Original function: FUN_004fd640, records the input of the frame for the versus replay.
    void FUN_004fd640_recordVersusInput() { FP::callFarA<void>(0x004fd640); }
    /// Original function: FUN_00466970, the deployment and the battle start, 1 once done.
    int FUN_00466970_updateBattleStart(int mode) { return FP::callStdFastFarA<int>(0x00466970, mode); }
    /// Original function: FUN_0044b480, the info windows of the hovered tile and unit.
    void FUN_0044b480_drawHoverInfo() { FP::callFarA<void>(0x0044b480); }

    /**
    * @brief Original function: FUN_00477550, a message window shown for a number of frames.
    * x and y -1 center it, color points to a palette entry like 0x005AD538.
    */
    void FUN_00477550_showMessage(int x, int y, int width, int lineCount, int frames, const void* color, const void* line1,
        const void* line2 = nullptr) {
        FP::callFastFarA<void>(0x00477550, x, y, width, lineCount, frames, color, line1, line2, 0, 0, 0, 0, 0, 0);
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
