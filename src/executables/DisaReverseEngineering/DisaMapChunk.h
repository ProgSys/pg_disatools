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
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <Disa/fileformats/PG_MPD.h>
#include <Resources/PG_Model.h>

#include "DisaFunctions.h"

/**
* @brief One textured quad of a map chunk, ready to submit. FUN_004a3ab0_buildChunkFaces builds the static tile faces,
* FUN_004a4660 appends the per frame overlays after them.
* Size: 0x78
*/
struct DisaMapFace {
    struct Vertex {
        glm::vec2 uv;
        glm::vec3 pos;
    };

    uint32_t blendTag;       // 0x00 - 'D' normal, 'H' blend mode 1, 'b' blend mode 2
    uint32_t field0x04;      // 0x04 - always 0x80
    uint8_t* textureEntry;   // 0x08 - 0x88 byte texture entry, +0x7c texture, +0x80 normal map
    int32_t field0x0c;       // 0x0c - always 0
    int32_t slot;            // 0x10 - tile texture slot 0 to 10, overlays use 11
    uint32_t r;              // 0x14 - 0 to 255
    uint32_t g;              // 0x18
    uint32_t b;              // 0x1c
    uint32_t a;              // 0x20 - 255 is drawn at depth 40000, anything else at 45000
    Vertex vertices[4];      // 0x24 - strip order NW, NE, SW, SE
    uint32_t field0x74;      // 0x74
};
static_assert(sizeof(DisaMapFace) == 0x78);

/**
* @brief Pointer and count handed to the face submitters FUN_004f1070 and FUN_004f02e0.
* Size: 0x08
*/
struct DisaFaceList {
    DisaMapFace* faces; // 0x00
    int32_t count;      // 0x04
};

/**
* @brief A display list whose content is recorded by calling a callback. Created by FUN_004fde70_createChunkMesh,
* recorded again by FUN_004fdff0 and destroyed by FUN_004fe080_destroyCachedMesh.
* Size: 0x20
*/
struct DisaCachedMesh {
    void* callback;          // 0x00 - records the geometry, e.g. FUN_004f25d0
    void* userData;          // 0x04 - DisaFaceList* for map chunks
    uint32_t field0x08;      // 0x08 - never written
    uint32_t field0x0c;      // 0x0c - never written
    uint32_t field0x10;      // 0x10 - always 0x40
    uint32_t field0x14;      // 0x14 - always 2
    void* list;              // 0x18 - 0x30 byte list object owning the GL list ids and VBOs
    uint32_t listId;         // 0x1c - active GL list id, refreshed after every recording
};
static_assert(sizeof(DisaCachedMesh) == 0x20);

/**
* @brief Runtime state of one bound GEO file. Filled by FUN_004e0a40, drawn by FUN_004e13a0.
* Size: 0xe8
*/
struct DisaGeoModel {
    bool isInstanced;                 // 0x00 - deferred instance slots were allocated by FUN_004e07c0
    uint8_t field0x01[0x33];          // 0x01
    int32_t* field0x34;               // 0x34
    int32_t animationFrameMax;        // 0x38 - largest transformation or alpha keyframe time, 0 for static models
    uint32_t field0x3c;               // 0x3c - GEO header[0]
    int32_t numberOfMeshes;           // 0x40
    int32_t numberOfNodes;            // 0x44
    int32_t numberOfTransformations;  // 0x48
    int32_t numberOfAlphaKeyframes;   // 0x4c
    int32_t offsetToFirstMesh;        // 0x50
    int32_t field0x54;                // 0x54 - GEO header[2], node index that gets a special flag
    int32_t isBound;                  // 0x58
    void* geoFileData;                // 0x5c
    geofileTransformation* transformations; // 0x60
    geofileKeyframe* alphaKeyframes;  // 0x64
    geofileTreeNode* nodes;           // 0x68
    DisaObjectModel* meshes;          // 0x6c
    DisaTransform transform;          // 0x70 - identity, not read by FUN_004e13a0
    glm::vec4 field0xa0;              // 0xa0 - 1, 1, 1, 1
    glm::vec4 field0xb0;              // 0xb0 - 1, 1, 1, 1
    uint32_t field0xc0[5];            // 0xc0
    DisaObjectScrollInfo uvScroll;    // 0xd4 - every mesh's m_uvScrollObject points here
    uint32_t field0xe4;               // 0xe4
};
static_assert(sizeof(DisaGeoModel) == 0xe8);

/**
* @brief One loaded MPP: up to 16 GEO models and 16 textures that chunk objects reference by model id.
* Four sets live at 0x00703480. Sets 0 and 1 belong to the two map chunks, set 3 holds the common objects.
* Size: 0x1750
*/
struct DisaGeoModelSet {
    uint8_t field0x00[0x2c];         // 0x00
    uint32_t field0x2c;              // 0x2c
    void* mppData;                   // 0x30 - start of the MPP file
    void* mppOverrideData;           // 0x34 - second MPP bound over the first
    DisaGeoModel models[16];         // 0x38
    uint8_t textures[16][0x88];      // 0xeb8
    uint8_t textureCount;            // 0x1738
    uint8_t modelCount;              // 0x1739
    uint8_t field0x173a[2];          // 0x173a
    bool isLoaded;                   // 0x173c
    uint8_t fileGroup;               // 0x173d - 0xca for maps
    bool isAssigned;                 // 0x173e - assigned to a chunk
    uint8_t padding0x173f;           // 0x173f
    int32_t fileId;                  // 0x1740
    uint32_t field0x1744;            // 0x1744
    bool hasExtraTextureBlock;       // 0x1748
    uint8_t padding0x1749[7];        // 0x1749

    /// Four sets located at 0x00703480
    static DisaGeoModelSet* get(int index);
};
static_assert(sizeof(DisaGeoModelSet) == 0x1750);

/**
* @brief Runtime map chunk: the tiles, the placed objects and the render cache of one MPD chunk.
* The game holds two of them at 0x007091c0, loaded by FUN_004d4980_CopyDungeonIntoActive.
* Size: 0x91a8
*/
struct DisaMapChunk {
    uint32_t builtWithBatching;      // 0x0000 - DisaSettings::isBatchedMapRendering when the lists were built
    DisaCachedMesh* opaqueMesh;      // 0x0004 - static faces tagged 'D', drawn at depth 40000
    DisaCachedMesh* blendedMesh;     // 0x0008 - static faces tagged 'H' or 'b', drawn at depth 40005
    DisaFaceList meshFaces;          // 0x000c - handed to the list callbacks
    int32_t meshState;               // 0x0014 - 0 lists current, 1 uncached, 2 and 3 record the lists again
    glm::vec4 position;              // 0x0018
    glm::vec4 rotation;              // 0x0028 - radians, applied Z then X then Y
    DisaMapFace* faces;              // 0x0038 - 0x6c0 faces: static, then geo panels, then highlights
    mpdfileTile tiles[256];          // 0x003c
    mpdfileChunkObject objects[32];  // 0x803c
    float anchorX;                   // 0x84bc
    float anchorZ;                   // 0x84c0
    glm::vec3 pivot;                 // 0x84c4 - subtracted from the tile position, the rotation center
    float rotateStep;                // 0x84d0 - radians added to rotation.y per frame while rotateFramesLeft > 0
    int16_t staticFaceCount;         // 0x84d4 - 0 draws no tiles and no overlays
    int16_t highlightFaceCount;      // 0x84d6
    int16_t geoPanelFaceCount;       // 0x84d8
    int16_t field0x84da;             // 0x84da
    int16_t halfSizeX;               // 0x84dc
    int16_t halfSizeZ;               // 0x84de
    bool isActive;                   // 0x84e0
    uint8_t rotationIndex;           // 0x84e1 - quarter turns 0 to 3
    int8_t rotateFramesLeft;         // 0x84e2
    int8_t modelSetIndex;            // 0x84e3 - index for DisaGeoModelSet::get
    uint16_t tileGrid[40][40];       // 0x84e4 - [x][z] tile index + 1, 0 = no tile
    mpdfileChunkEventTile eventTiles[16]; // 0x9164
    bool field0x91a4;                // 0x91a4
    uint8_t field0x91a5;             // 0x91a5
    bool isHDTexture;                // 0x91a6 - picks the normal mapped list callbacks
    uint8_t padding0x91a7;           // 0x91a7

    static constexpr int count = 2;

    /**
    * @brief Resets the chunk to an empty, inactive chunk at (6, 0, 6) and destroys its cached meshes.
    * @param isFullClear Clears the whole tiles. Otherwise geo_color, geo_mark and the padding of each tile are kept.
    */
    void __thiscall FUN_004a27f0_clearChunk(bool isFullClear);

    /**
    * @brief Height of the tile surface at a world position, negative is up.
    * Original function: FUN_004a2af0
    * @details The position is rotated into the chunk with FUN_004e32a0_rotateXZ, which also writes its result to the
    * globals 0x006B791C and 0x006B7A60. On a tile edge the two corners of that edge are interpolated. Inside, the
    * side of the diagonal from SE to NW picks a corner and a line: on the SW side the line from SE through the
    * position is cut with the north edge, otherwise the line from SW is cut with the diagonal. The height at the
    * cut is interpolated from the two ends of the cut line, the result again between the corner and the cut.
    * @return 0 outside the 40 x 40 grid and where no visible tile is.
    */
    float __thiscall FUN_004a2af0_getHeight(float x, float z) const;

    /// The two chunks located at 0x007091c0
    static DisaMapChunk* get(int index);
};
static_assert(sizeof(DisaMapChunk) == 0x91a8);
static_assert(offsetof(DisaMapChunk, tiles) == 0x3c);
static_assert(offsetof(DisaMapChunk, objects) == 0x803c);
static_assert(offsetof(DisaMapChunk, staticFaceCount) == 0x84d4);
static_assert(offsetof(DisaMapChunk, modelSetIndex) == 0x84e3);
static_assert(offsetof(DisaMapChunk, isHDTexture) == 0x91a6);

/**
* @brief Appends the per frame overlay faces after the static faces: geo panels first, then the tile
* highlights. Consumes the highlight type (tile byte 0x73) that gameplay writes every frame.
*/
void FUN_004a4660_BuildTileOverlays(DisaMapChunk& chunk);

/**
* @brief Queues one map chunk for this frame: the cached static tile faces, the per frame overlays
* and the 32 placed GEO models.
* @param unused EDX, never read.
* @param camera The main world camera (0x00730980), only forwarded to the models for billboards.
*/
void __fastcall FUN_004a4c60_SubmitMapChunk(DisaMapChunk* chunk, int unused, DisaCameraInfo* camera);

//BEGIN custom code
/**
* @brief A custom model placed on a map chunk. @see PG_MPDCustomModel
*/
struct DisaCustomMapModel {
    PG_Model model;            /// requested in the namespace of the campaign the map belongs to
    mpdfileChunkObject object; /// position, rotation and scale as the file stores them
    std::uint64_t nodeId = 0;  /// new on every map load, so the nodes of the last map expire
};

/// DisaMapChunk index -> its custom models. Replaced on every map load, dropped when the chunk is cleared.
std::unordered_map<int, std::vector<DisaCustomMapModel>>& getCustomMapModels();

/**
* @brief Queues the custom models of the chunk for this frame.
* @details Called where the marker object of the chunk would be drawn, so whatever hides the marker
* hides them too.
*/
void submitCustomMapModels(const DisaMapChunk& chunk, const glm::mat4& chunkMatrix);
//END custom code
