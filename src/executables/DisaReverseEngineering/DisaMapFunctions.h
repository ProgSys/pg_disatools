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
* Size: 0x42c
*/
struct DisaMapUnit {
    uint8_t field0x000[0x26b];          // 0x000
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
