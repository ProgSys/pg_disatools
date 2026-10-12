/*
 * GNU Lesser General Public License (LGPL-3.0)
 *
 * Copyright (c) 2026 ProgSys
 *
 * This code is part of a free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * This code is distributed in the hope that it will be useful, without any
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 * or https://www.gnu.org/licenses/lgpl-3.0.md
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once
#include <cstdint>
#include <cstddef>

#include "DisaFunctions.h"

struct DisaPartyCharacterInfo;

/**
* @brief Plays one animation of a sprite. FUN_004de8f0 starts it, FUN_004de9e0 advances it once per frame.
* Size: 0x20
*/
struct DisaSpriteAnimation {
    void* spriteData;                       // 0x00 - from FUN_004de880_getSpriteData, nullptr when the sprite is missing
    uint8_t* frame;                         // 0x04 - current 8 byte frame entry, byte 2 is its duration, byte 3 its type
    int32_t frameTicksLeft;                 // 0x08
    int32_t speed;                          // 0x0c - subtracted from frameTicksLeft per frame, always 1
    int32_t palette;                        // 0x10
    int32_t paletteOffset;                  // 0x14 - added to palette
    int16_t loopCount;                      // 0x18 - counts frames of type 0xf
    int16_t endState;                       // 0x1a - 1 after a type 3 marker, 2 after a type 2 marker, FUN_004de990 clears it
    uint16_t isMirrored;                    // 0x1c - from a negative animation id, flips x
    uint8_t field0x1e[0x2];                 // 0x1e
};
static_assert(sizeof(DisaSpriteAnimation) == 0x20);

/**
* @brief Moves four values to a target over a number of frames. FUN_0046e510 starts it, FUN_0046e620 steps it.
* Also used for the 32 render groups at 0x005FD3A8 (scale and rotation) and 0x005FDA28 (color).
* Size: 0x34
*/
struct DisaVec4Morph {
    glm::vec4 current;                      // 0x00 - RGBA, for 0x005FD3A8 scale x and y added to 1, then rotation in radians
    glm::vec4 target;                       // 0x10 - FUN_0046e510 takes percent and stores factors
    glm::vec4 step;                         // 0x20
    int32_t framesLeft;                     // 0x30
};
static_assert(sizeof(DisaVec4Morph) == 0x34);

/**
* @brief Circles a unit around a center while the radius moves from radiusStart to radiusEnd, script OrbitActor.
* FUN_0046cba0 runs it. The center is DisaMapUnit::orbitCenter, which lies outside this block at 0x1a8.
* Size: 0x20
*/
struct DisaOrbit {
    glm::vec3* anchor;                      // 0x00 - position the orbit follows, nullptr for a fixed center
    float angle;                            // 0x04 - degrees
    float step;                             // 0x08 - degrees per frame
    float radiusStart;                      // 0x0c
    float radiusEnd;                        // 0x10
    int32_t isActive;                       // 0x14
    int32_t framesLeft;                     // 0x18
    int32_t frames;                         // 0x1c
};
static_assert(sizeof(DisaOrbit) == 0x20);

/**
* @brief A unit placed on the map, a party member, an enemy, a geo symbol, an NPC or a senator.
* @details The first 0x284 bytes are a sprite actor shared with the map cursors at 0x00730BA0 and 0x00778A10,
* which end right before character. Script actors (script slots 64 to 95) live in a second array of 64 at 0x0071FE80.
* Size: 0x42c
*/
struct DisaMapUnit {
    DisaSpriteAnimation sprite;             // 0x000 - body, plays animation of spriteId
    uint8_t field0x020[0x40];               // 0x020
    DisaSpriteAnimation statusIconSprite;   // 0x060 - sprite 1000, animation 99 + status, FUN_0046bd20
    uint8_t field0x080[0x40];               // 0x080
    DisaSpriteAnimation doneMarkerSprite;   // 0x0c0 - sprite 1000 animation 7, FUN_0046c4c0 draws it once moved and acted
    uint8_t field0x0e0[0x40];               // 0x0e0
    DisaVec4Morph colorMorph;               // 0x120 - RGBA multiplier of color, script SetMultRGBA
    glm::vec3 position;                     // 0x154
    glm::vec3 restPosition;                 // 0x160 - y is the floor the physics lands on
    glm::vec3 velocity;                     // 0x16c - added per frame, gravity adds to y. The cursor sets x and z from the stick
    glm::vec3 moveTarget;                   // 0x178 - where FUN_0046cab0_translateTo moves the position to
    glm::vec3 moveStep;                     // 0x184 - added to the position per frame while moveFramesLeft counts down
    glm::vec3 moveOrigin;                   // 0x190 - position before the move, FUN_00448190 undoes the move to it
    glm::vec3 heldUnitOrigin;               // 0x19c - where the unit at holdingIndex stood before the lift
    glm::vec3 orbitCenter;                  // 0x1a8 - offset to orbit.anchor, or a world position without one
    float gravity;                          // 0x1b4 - 0.163, script SetActorGravity
    float restY;                            // 0x1b8 - copy of restPosition.y
    int8_t statusIcons[5];                  // 0x1bc - statuses shown in turn by statusIconSprite
    uint8_t field0x1c1[3];                  // 0x1c1
    glm::i16vec4 color;                     // 0x1c4 - RGBA 0 to 255, 16 bit in the game so not glm::rgba
    int32_t isOverbright;                   // 0x1cc - the tinted color passed 255 and was halved, FUN_0046ab50
    int32_t depthBias;                      // 0x1d0 - added to the sprite sort order, script SetActorDepth
    float shadowScale;                      // 0x1d4 - shadow is 8 * shadowScale wide, character data 0x5a / 100
    float spinStep;                         // 0x1d8 - degrees per frame while spinFramesLeft runs
    float spinAngle;                        // 0x1dc
    DisaOrbit orbit;                        // 0x1e0 - center is orbitCenter
    int32_t hpBarMax;                       // 0x200
    int32_t hpBarFrom;                      // 0x204
    int32_t hpBarTo;                        // 0x208
    glm::vec3 modelRotation;                // 0x20c - degrees, only for modelType != 0
    float modelPitchTarget;                 // 0x218 - aim, clamped to -30 to 60
    float modelYawTarget;                   // 0x21c - aim
    float jumpDrop;                         // 0x220 - height from the apex down to the target, FUN_0046c9b0
    int16_t showHpBar;                      // 0x224 - cleared while the AI walks
    int16_t hpBarColor;                     // 0x226 - alliance
    int16_t hpBarFrame;                     // 0x228
    int16_t hpBarFrames;                    // 0x22a
    int16_t spinFramesLeft;                 // 0x22c - script setActorRotation
    int16_t characterId;                    // 0x22e
    int16_t spriteId;                       // 0x230
    int16_t palette;                        // 0x232
    int16_t animation;                      // 0x234 - requested, negative mirrors
    int16_t currentAnimation;               // 0x236 - sprite restarts when animation differs
    int16_t pose;                           // 0x238 - animation set, 0 idle, 1 walk, 3 jump, 5 held, 8 lifting
    int16_t facing;                         // 0x23a - degrees, -180 to 180
    int16_t turnFramesLeft;                 // 0x23c
    int16_t facingTarget;                   // 0x23e
    int16_t turnStartFacing;                // 0x240
    int16_t turnFrames;                     // 0x242
    int16_t restFacing;                     // 0x244 - saved with restPosition by FUN_004d2ca0
    int16_t viewDirection;                  // 0x246 - facing relative to the camera, 0 to 3 or 0 to 7
    int16_t removeFramesLeft;               // 0x248 - FUN_0046c5f0 at 0, the sprite blinks until then
    int16_t poseFramesLeft;                 // 0x24a - pose returns to 0 at 0
    int16_t moveOriginFacing;               // 0x24c
    int16_t heldUnitOriginFacing;           // 0x24e
    int16_t moveFramesLeft;                 // 0x250 - frames until moveTarget is reached, 0 when not moving
    int16_t field0x252;                     // 0x252 - -1 on init
    int16_t modelType;                      // 0x254 - 0 sprite, 1 to 3 a 3D model (characters 2300 to 2302)
    int16_t aimTargetIndex;                 // 0x256 - -1 none, FUN_004d2e40
    int16_t modelFrame;                     // 0x258
    int16_t isModelAnimating;               // 0x25a
    int16_t turretShots;                    // 0x25c
    int16_t turretShotInterval;             // 0x25e - frames
    int16_t jumpRiseFramesLeft;             // 0x260 - gravity switches to land on the target at 0
    int16_t jumpFallFrames;                 // 0x262
    int16_t statusIconAnimation;            // 0x264
    int16_t heldUnitFacingOffset;           // 0x266 - added to facingTarget of the held units
    bool isVisible;                         // 0x268
    int8_t shadowMode;                      // 0x269 - 0 none, 1 at restPosition.y, 2 at position.y
    int8_t animationMode;                   // 0x26a - 0 fixed, 1 four directions, 2 eight directions
    bool isActive;                          // 0x26b
    int8_t chunkIndex;                      // 0x26c - -1 none, FUN_004d9cb0 compares it with the index of the rotating chunk
    int8_t poseStep;                        // 0x26d - progress inside a jump, float or warp, FUN_004475c0
    int8_t weaponType;                      // 0x26e - 0 none, 1 to 7, adds 1000 to the animation
    int8_t paletteOffset;                   // 0x26f
    bool isRawPositioned;                   // 0x270 - script SetActorRawPos, skips the map update
    int8_t renderGroup;                     // 0x271 - 0 none, else a morph of 0x005FD3A8 and 0x005FDA28
    int8_t weaponSprite;                    // 0x272 - % 20 sprite, / 20 palette
    int8_t heldByIndex;                     // 0x273 - unit lifting this one, -1 none
    int8_t holdingIndex;                    // 0x274 - unit this one lifts, -1 none
    uint8_t field0x275;                     // 0x275
    int8_t throwDistance;                   // 0x276 - tiles
    uint8_t field0x277;                     // 0x277
    int8_t modelPose;                       // 0x278 - 1 or 2 tips the model over, no aiming and no HP bar
    uint8_t field0x279[2];                  // 0x279
    int8_t statusIconFramesLeft;            // 0x27b - 60 per icon
    int8_t statusIconIndex;                 // 0x27c - 1 based into statusIcons, 0 none
    int8_t movementType;                    // 0x27d - 0 walk, 1 float, 2 warp
    bool isDepthWritten;                    // 0x27e - script SetActorFlag2
    uint8_t field0x27f;                     // 0x27f
    int32_t isDepthTested;                  // 0x280 - 1 on init, 0 draws the sprite over everything
    DisaPartyCharacterInfo* character;      // 0x284 - the cursors end before this field
    void* senatorInfo;                      // 0x288 - Dark Assembly entry at 0x0064FA98
    uint8_t field0x28c[4];                  // 0x28c
    int32_t talkScriptId;                   // 0x290 - script SetUnitScript, run when the unit is talked to
    int32_t senatorVoicePitch;              // 0x294
    int32_t senatorVoice;                   // 0x298
    int16_t pathLength;                     // 0x29c
    int16_t pathStep;                       // 0x29e
    int16_t pathX[64];                      // 0x2a0 - tiles, [0] is the start
    int16_t pathZ[64];                      // 0x320
    int16_t movement;                       // 0x3a0 - 99 and up moves in one step
    int16_t jumpPower;                      // 0x3a2
    int16_t throwRange;                     // 0x3a4
    int16_t aiPlanIndex;                    // 0x3a6
    int16_t aiArea;                         // 0x3a8 - area shape of aiSkillId
    int16_t aiTargetX;                      // 0x3aa - tile
    int16_t aiTargetZ;                      // 0x3ac
    int16_t aiTargetDirection;              // 0x3ae
    int16_t aiStandX;                       // 0x3b0 - tile the plan was made from
    int16_t aiStandZ;                       // 0x3b2
    int16_t index;                          // 0x3b4 - own index in its array
    int16_t field0x3b6;                     // 0x3b6 - spawn writes the character slot or id
    int16_t field0x3b8;                     // 0x3b8 - 0 on spawn
    int16_t aiCanReach;                     // 0x3ba
    int16_t aiMode;                         // 0x3bc - 2 hunts, 3 stays in place
    int16_t counterCount;                   // 0x3be - lowers the counter chance, Defend adds 2
    uint8_t field0x3c0[0xa];                // 0x3c0
    int16_t aiFacing;                       // 0x3ca - degrees
    int16_t aiSkillId;                      // 0x3cc
    int16_t difficultySpawnRule;            // 0x3ce - MPD actor
    int16_t levelsGained;                   // 0x3d0 - shown and cleared after the attack
    int16_t geoEffect;                      // 0x3d2 - MPD actor
    int16_t weaponMasteryMultiplier;        // 0x3d4 - MPD actor
    int16_t aiHold;                         // 0x3d6 - MPD actor byte 0x26, nonzero forces aiMode 3
    int16_t aiType;                         // 0x3d8 - MPD actor
    int16_t magicIndex[4];                  // 0x3da - 1 based into 0x006B7890
    int16_t magicLevel[4];                  // 0x3e2
    int16_t isGeoSymbol;                    // 0x3ea - characters 2800 to 2809
    int16_t isOnGeoPanel;                   // 0x3ec
    int16_t geoPanelColor;                  // 0x3ee - 1 based
    int16_t geoSymbolColor;                 // 0x3f0 - 0 based
    int16_t geoSymbolEffect;                // 0x3f2
    int16_t field0x3f4;                     // 0x3f4
    int16_t field0x3f6;                     // 0x3f6 - random 1 based index into the 0x24 byte table at *0x006192B8
    int16_t itemWorldRank;                  // 0x3f8
    int16_t senatorFavor;                   // 0x3fa - stance on the bill
    int16_t damageMode;                     // 0x3fc - 1 dies after hitsToKill hits, 2 survives with 1 HP
    int16_t hitsToKill;                     // 0x3fe
    bool isDefending;                       // 0x400 - halves the damage taken
    int8_t alliance;                        // 0x401 - 0 player, 1 enemy, 2 neutral
    uint8_t moveState;                      // 0x402 - 0 none, 1 undoable, 2 committed
    uint8_t actionState;                    // 0x403 - 0 none, 1 Defend or lift (undoable), 2 acted
    bool isLiftLocked;                      // 0x404
    bool isFromBasePanel;                   // 0x405 - undoing the move puts it back
    uint8_t aiPlanData[0xc];                // 0x406 - from 0x006AC550
    int8_t aiTargets[12];                   // 0x412 - unit indices
    uint8_t aiTargetCount;                  // 0x41e
    uint8_t aiPlanArea;                     // 0x41f
    uint8_t field0x420[3];                  // 0x420
    bool isSenator;                         // 0x423
    int8_t healthMultiplier;                // 0x424 - MPD actor
    uint8_t field0x425;                     // 0x425
    int8_t itemWorldTitle;                  // 0x426
    uint8_t field0x427;                     // 0x427
    int32_t drawMode;                       // 0x428 - 2 draws in its own pass at depth 40000

    static constexpr int count = 64;

    /**
    * @brief Starts a straight move to the target, or jumps there when frames is 0.
    * Original function: FUN_0046cab0
    * @details Sets moveTarget, and moveStep to the distance divided by frames. The move itself happens in
    * FUN_0046cba0, once per frame while moveFramesLeft counts down.
    * @param frames Truncated to whole frames.
    */
    void __thiscall FUN_0046cab0_translateTo(float x, float y, float z, float frames);

    /// The units located at 0x00738710.
    static DisaMapUnit* get(int index);
};
static_assert(sizeof(DisaMapUnit) == 0x42c);
static_assert(offsetof(DisaMapUnit, statusIconSprite) == 0x060);
static_assert(offsetof(DisaMapUnit, doneMarkerSprite) == 0x0c0);
static_assert(offsetof(DisaMapUnit, colorMorph) == 0x120);
static_assert(offsetof(DisaMapUnit, position) == 0x154);
static_assert(offsetof(DisaMapUnit, velocity.z) == 0x174);
static_assert(offsetof(DisaMapUnit, moveTarget) == 0x178);
static_assert(offsetof(DisaMapUnit, moveStep) == 0x184);
static_assert(offsetof(DisaMapUnit, orbitCenter) == 0x1a8);
static_assert(offsetof(DisaMapUnit, color) == 0x1c4);
static_assert(offsetof(DisaMapUnit, orbit) == 0x1e0);
static_assert(offsetof(DisaMapUnit, hpBarMax) == 0x200);
static_assert(offsetof(DisaMapUnit, facing) == 0x23a);
static_assert(offsetof(DisaMapUnit, moveFramesLeft) == 0x250);
static_assert(offsetof(DisaMapUnit, isActive) == 0x26b);
static_assert(offsetof(DisaMapUnit, chunkIndex) == 0x26c);
static_assert(offsetof(DisaMapUnit, isDepthTested) == 0x280);
static_assert(offsetof(DisaMapUnit, character) == 0x284);
static_assert(offsetof(DisaMapUnit, pathZ) == 0x320);
static_assert(offsetof(DisaMapUnit, index) == 0x3b4);
static_assert(offsetof(DisaMapUnit, alliance) == 0x401);
static_assert(offsetof(DisaMapUnit, drawMode) == 0x428);

/**
* @brief Loads the MPP (map models and textures) into the first model set at 0x00703480 and binds it.
* Chapters 655 and up are custom campaigns and read from the DisaCustomCampaignManager.
*/
void _stdcall FUN_004a59c0_LoadMPP(unsigned short chapterId, unsigned short mapId);

/**
* @brief Loads the MPD of the map and copies it into the active chunks with FUN_004d4980_CopyDungeonIntoActive.
* The original takes two more stack arguments, isHDTexture and isDoNotLoadActors, and leaves them for the caller to pop.
*/
void __fastcall FUN_004d5060_LoadMPD(unsigned short chapterId, unsigned short mapId);

/**
* @brief Copies the loaded MPD (0x00668f14) into its map chunk: tiles, objects, event tiles and actors.
* Also places the teleport panel and spawns the actors.
*/
void __fastcall FUN_004d4980_CopyDungeonIntoActive(int chapterId, int mapId, int isHDTexture, bool isDoNotLoadActors);

/**
* @brief Finishes a map load: builds the base panel chunk, recenters the chunks, rebuilds the world grid,
* spawns a bonus object and resets the camera. Currently calls the original.
*/
void __fastcall FUN_004d6a30_finalizeDungeonLoad(DisaDungeon* dungeon);

/**
* @brief Clears both map chunks and marks all model sets as unloaded.
* @param isFullClear Also clears the whole tile memory and the map units. The script map loads pass true,
* the map reload after toggling HD textures passes false.
*/
void __fastcall FUN_004d7970_clearMap(bool isFullClear);

/**
* @brief Deactivates all map units and releases the temporary characters they point to.
*/
void FUN_004d3060_clearMapUnits();

/**
* @brief Ground height at a world position over both map chunks, negative is up.
* Original function: FUN_004d2be0, which leaves x and z on the stack for the caller to pop.
* Only active chunks that are part of the world grid (0x0065DB48) count, and of those the smallest height wins, so
* the topmost surface.
* @param chunkIndex Receives the chunk the height comes from, -1 when no chunk counts. Can be nullptr.
* @param unused EDX, never read.
* @return 0 when no chunk counts.
*/
float __fastcall FUN_004d2be0_getGroundHeight(int* chunkIndex, int unused, float x, float z);

/**
* @brief The world position under a screen position, used to point the map cursor at the mouse.
* Original function: FUN_004d1070, which leaves screenY, height and followTerrain on the stack for the caller to pop.
* @details The screen position is unprojected with the inverse of the matrix at 0x00819200 at depth 0 and 1, and the
* ray between them is cut with the plane y = 0. The direction from the camera eye at 0x00730980 to that point is
* the view direction. Without followTerrain the point is moved to the plane y = height in whole unit steps along the
* view direction, so x and z can be off by up to one step. With followTerrain the view ray is walked from 200 units
* before that point towards it in steps of 2, and the first sample within 0.6 of FUN_004d2be0_getGroundHeight is
* taken. If no sample hits, it falls back to the plane like without followTerrain.
* @param screenX In the 480 x 272 screen space of the mouse (0x00701C80).
* @param height Plane height for the fallback.
* @return out
*/
glm::vec3* __fastcall FUN_004d1070_screenToWorld(glm::vec3* out, int screenX, int screenY, float height, bool followTerrain);

/**
* @brief Moves the map cursor by key, analog stick or mouse, once per frame.
* Original function: FUN_004d1500, which leaves unused and useRecordedInput on the stack for the caller to pop.
* @details Nothing happens while the cursor still glides to its last target (moveFramesLeft). A key moves it one tile
* with FUN_0046cab0 over 10 frames, 5 while button 0x4000 is held, and turns facing. The analog stick sets the velocity,
* rotated by the camera yaw. Without key and stick input and with mouse control on, the cursor glides over 5 frames to
* the tile under the mouse, but only when the mouse moved and is not on the screen edge. Everything stays within the
* map bounds at 0x00664768 to 0x00664774.
* @param camera The main camera (0x00730980), its yaw turns the stick input.
* @param cursor The map cursor, 0x00730BA0 or 0x00778A10.
* @param unused Never read, the callers push ECX.
* @param useRecordedInput 1 reads the direction and the yaw from 0x006D4ACB and 0x006E57B8 instead of the pad and the
* camera. Otherwise the direction is recorded to 0x006D4ACA, plus 10 while button 0x4000 is held.
*/
void __fastcall FUN_004d1500_updateMapCursor(DisaCameraInfo* camera, DisaMapUnit* cursor, int unused, char useRecordedInput);

/**
* @brief Call site replacement for FUN_004d1500. Takes camera and cursor in ECX and EDX like the original and leaves
* the stack arguments for the caller to pop.
*/
void FUN_004d1500_updateMapCursor_NoneStd(int unused, char useRecordedInput);

/**
* @brief Call site replacement for FUN_004d1070. Takes out and screenX in ECX and EDX like the original and leaves
* the stack arguments for the caller to pop.
*/
void FUN_004d1070_screenToWorld_NoneStd(int screenY, float height, bool followTerrain);

/**
* @brief Floods the move range of a unit, breadth first over the 64 x 64 tile grid.
* Original function: FUN_004d21f0, which leaves layer on the stack for the caller to pop.
* @details Fills the distance map at 0x00666F10 with the steps to every tile the unit can pass, allies included, and the
* stop map at 0x00664F10 with the steps to every tile it may end on. Allies stay 0xfd in the stop map, every other unit
* and the blocked base panels 0xfe, tiles out of reach 0xff in both. A step needs a tile with mobility 0, no No Entry
* geo effect and a climb of at most jumpPower, falls are free. The flood ends after 32 steps. With movement 99 and more
* every free tile gets 0 instead.
* @param flags 2 ignores allies, 4 counts every other unit on the end of its path instead of its position.
* @param layer 0, or 1 for the second player in versus mode. -1 is layer 0.
*/
void __fastcall FUN_004d21f0_buildMoveRange(DisaMapUnit* unit, uint32_t flags, int8_t layer);

/**
* @brief Traces the path of a unit to a tile backwards through the distance map of FUN_004d21f0_buildMoveRange.
* Original function: FUN_004d1e50, which leaves z and layer on the stack for the caller to pop.
* @details The trace covers the whole distance and pathX[0] is the start tile. Each step back takes the first neighbor
* in the order -z, +x, +z, -x that is one step closer and whose climb fits jumpPower. When the stop map marks the tile
* at pathLength as taken, pathLength shrinks to the last free one. Warpers and units with movement 99 and more get the
* one step path [target, target].
* @param unit pathLength must hold the most steps the unit may take, it is clamped to the distance.
* @return 0, or -1 when the tile is outside the grid, out of reach, or no free end is left.
*/
int __fastcall FUN_004d1e50_buildPath(DisaMapUnit* unit, int x, int z, int8_t layer);

/**
* @brief Call site replacement for FUN_004d21f0. Takes unit and flags in ECX and EDX like the original and leaves
* layer for the caller to pop.
*/
void FUN_004d21f0_buildMoveRange_NoneStd(int8_t layer);

/**
* @brief Call site replacement for FUN_004d1e50. Takes unit and x in ECX and EDX like the original and leaves z and
* layer for the caller to pop.
*/
void FUN_004d1e50_buildPath_NoneStd(int z, int8_t layer);
