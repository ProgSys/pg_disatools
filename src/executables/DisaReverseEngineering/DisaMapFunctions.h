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
* @brief A unit placed on the map, a party member, an enemy or a guest.
* The map cursors at 0x00730BA0 and 0x00778A10 are objects of the same type.
* Size: 0x42c
*/
struct DisaMapUnit {
    uint8_t field0x000[0x154];          // 0x000
    glm::vec3 position;                 // 0x154
    uint8_t field0x160[0xc];            // 0x160
    float velocityX;                    // 0x16c - cursor only, from the analog stick
    float field0x170;                   // 0x170
    float velocityZ;                    // 0x174 - cursor only, from the analog stick
    uint8_t field0x178[0xc2];           // 0x178 - 0x178 to 0x18c hold the target and the step of FUN_0046cab0
    int16_t facing;                     // 0x23a - degrees, set to 0, 90, 180 or -90 by the last cursor key
    uint8_t field0x23c[0x14];           // 0x23c
    int16_t moveFramesLeft;             // 0x250 - frames until FUN_0046cab0 reaches its target
    uint8_t field0x252[0x19];           // 0x252
    bool isActive;                      // 0x26b
    int8_t chunkIndex;                  // 0x26c - FUN_004d9cb0 compares it with the index of the rotating chunk
    uint8_t field0x26d[0x13];           // 0x26d
    int32_t field0x280;                 // 0x280 - reset to 1 by FUN_004d3060_clearMapUnits
    DisaPartyCharacterInfo* character;  // 0x284
    uint8_t field0x288[0x179];          // 0x288
    bool isTemporaryCharacter;          // 0x401 - character lives in the temporary characters at 0x005fe0b0
    uint8_t field0x402[0x2a];           // 0x402

    static constexpr int count = 64;

    /// The units located at 0x00738710.
    static DisaMapUnit* get(int index);
};
static_assert(sizeof(DisaMapUnit) == 0x42c);
static_assert(offsetof(DisaMapUnit, position) == 0x154);
static_assert(offsetof(DisaMapUnit, velocityZ) == 0x174);
static_assert(offsetof(DisaMapUnit, facing) == 0x23a);
static_assert(offsetof(DisaMapUnit, moveFramesLeft) == 0x250);
static_assert(offsetof(DisaMapUnit, isActive) == 0x26b);
static_assert(offsetof(DisaMapUnit, character) == 0x284);
static_assert(offsetof(DisaMapUnit, isTemporaryCharacter) == 0x401);

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
