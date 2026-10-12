#include "DisaMapChunk.h"

#include <gl/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <span>
#include <Util/PG_MathGeometry.h>

#include <FunctionPatcher.h>
#include "DisaRendering.h"
#include "DisaMath.h"
#include <Managers/DisaCustomRenderManager.h>
#include <Disa/PG_DisaModelNode.h>

namespace {

    /// Copies the current render flags one level up and makes that level current.
    DisaRenderFlags& pushRenderFlags() {
        int& depth = FP::getData<int>(0x005bb030);
        DisaRenderFlags* flags = &FP::getData<DisaRenderFlags>(0x005bb038);
        flags[depth + 1] = flags[depth];
        return flags[++depth];
    }

    inline void popRenderFlags() {
        --FP::getData<int>(0x005bb030);
    }

    inline const DisaSettings* getSettings() {
        return FP::getData<DisaSettings*>(0x005d7c40);
    }

    /**
    * @brief Same result as rotateZ, rotateX, rotateY (each multiplied in from the left) and the final
    * translation multiply of the original: translate * rotY * rotX * rotZ.
    */
    glm::mat4 computeChunkMatrix(const DisaMapChunk& chunk) {
        glm::mat4 matrix = glm::translate(glm::mat4(1.f), glm::vec3(chunk.position));
        matrix = glm::rotate(matrix, chunk.rotation.y, glm::vec3(0.f, 1.f, 0.f));
        matrix = glm::rotate(matrix, chunk.rotation.x, glm::vec3(1.f, 0.f, 0.f));
        return glm::rotate(matrix, chunk.rotation.z, glm::vec3(0.f, 0.f, 1.f));
    }

    /**
    * @brief Wrapper that picks FUN_004f02e0 (batched) or FUN_004f1070 (one quad at a time) like the original does,
    * and calls it with every blend tag and without normal mapping.
    */
    void FUN_004f02e0_FUN_004f1070_submitFaces(DisaFaceList* faces, glm::mat4* modelMatrix) {
        const unsigned int function = getSettings()->isBatchedMapRendering() ? 0x004f02e0 : 0x004f1070;
        FP::callFastFarA<void>(function, faces, modelMatrix, 0, -1);
    }

    /**
    * @brief Wrapper of FUN_004fde70 that fills in the chunk's face list and the batching setting.
    * Records the callback into a new display list and queues the list once.
    * @param pass 0 for the opaque faces at depth 40000, 2 for the blended faces at depth 40005.
    */
    DisaCachedMesh* FUN_004fde70_createChunkMesh(DisaMapChunk& chunk, unsigned int callback, glm::mat4* modelMatrix, int pass) {
        const int batched = getSettings()->isBatchedMapRendering() ? 1 : 0;
        //the callback is passed a second time on the stack, the builder never reads it
        return FP::callFastFarA<DisaCachedMesh*>(0x004fde70, callback, &chunk.meshFaces, callback, modelMatrix, pass, batched);
    }

    /// Wrapper that calls FUN_004fe080 on both chunk meshes and clears the pointers.
    void FUN_004fe080_destroyChunkMeshes(DisaMapChunk& chunk) {
        if (chunk.opaqueMesh) {
            FP::callStdFastFarA<void>(0x004fe080, chunk.opaqueMesh);
            chunk.opaqueMesh = nullptr;
        }
        if (chunk.blendedMesh) {
            FP::callStdFastFarA<void>(0x004fe080, chunk.blendedMesh);
            chunk.blendedMesh = nullptr;
        }
    }

    void writeOverlayFace(DisaMapFace& face, const DisaMapChunk& chunk, const mpdfileTile& tile, glm::vec2 uvMin, glm::vec2 uvMax) {
        constexpr int tileSize = 12;
        face.blendTag = 'D';
        face.field0x04 = 0x80;
        face.textureEntry = &FP::getData<uint8_t>(0x0074bfd0);
        face.field0x0c = 0;
        face.slot = 11;

        const float x0 = static_cast<float>(tile.x * tileSize) - chunk.pivot.x;
        const float x1 = static_cast<float>((tile.x + 1) * tileSize) - chunk.pivot.x;
        const float z0 = static_cast<float>(tile.z * tileSize) - chunk.pivot.z;
        const float z1 = static_cast<float>((tile.z + 1) * tileSize) - chunk.pivot.z;
        //cornersTop is SW, SE, NW, NE. The 0.125 lifts the overlay above the tile, negative y is up
        auto height = [&tile](int corner) { return static_cast<float>(tile.cornersTop[corner]) - 0.125f; };

        face.vertices[0] = { { uvMin.x, uvMin.y }, { x0, height(2), z1 } };
        face.vertices[1] = { { uvMax.x, uvMin.y }, { x1, height(3), z1 } };
        face.vertices[2] = { { uvMin.x, uvMax.y }, { x0, height(0), z0 } };
        face.vertices[3] = { { uvMax.x, uvMax.y }, { x1, height(1), z0 } };
    }

    inline void setColor(DisaMapFace& face, uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
        face.r = r;
        face.g = g;
        face.b = b;
        face.a = a;
    }

    /// Tile byte 0x73, a runtime value gameplay writes every frame.
    inline uint8_t& highlightType(mpdfileTile& tile) {
        return tile.unk3[0];
    }

    /**
    * @brief The inner part of FUN_004a2af0. The line from corner through (x, z) is cut with the line through lineBegin
    * and lineEnd. Each point holds x, height and z.
    */
    float interpolateTileHeight(float x, float z, const glm::vec3& corner, const glm::vec3& lineBegin, const glm::vec3& lineEnd) {
        const auto distance = [](double deltaX, double deltaZ) { return std::sqrt(static_cast<float>(deltaX * deltaX + deltaZ * deltaZ)); };

        const double toCornerX = double(corner.x) - x;
        const double toCornerZ = double(corner.z) - z;
        const float slope = static_cast<float>(toCornerZ / toCornerX);
        const float offset = static_cast<float>(z - double(slope) * x);
        const float lineSlope = static_cast<float>((double(lineBegin.z) - lineEnd.z) / (double(lineBegin.x) - lineEnd.x));
        const float lineOffset = static_cast<float>(lineEnd.z - double(lineSlope) * lineEnd.x);

        const float cutX = static_cast<float>((double(lineOffset) - offset) / (double(slope) - lineSlope));
        const float cutZ = static_cast<float>(offset + double(slope) * cutX);

        const float toBegin = distance(double(lineBegin.x) - cutX, double(lineBegin.z) - cutZ);
        const float toEnd = distance(double(lineEnd.x) - cutX, double(lineEnd.z) - cutZ);
        const float lineLength = toEnd + toBegin;
        const float cutHeight = static_cast<float>(double(toBegin) * lineEnd.y / lineLength + double(toEnd) * lineBegin.y / lineLength);

        const float toCorner = distance(toCornerX, toCornerZ);
        const float toCut = distance(double(x) - cutX, double(z) - cutZ);
        const float length = toCut + toCorner;
        return static_cast<float>(double(toCorner) * cutHeight / length + double(toCut) * corner.y / length);
    }

} //namespace

DisaGeoModelSet* DisaGeoModelSet::get(int index) {
    return &(&FP::getData<DisaGeoModelSet>(0x00703480))[index];
}

DisaMapChunk* DisaMapChunk::get(int index) {
    return &(&FP::getData<DisaMapChunk>(0x007091c0))[index];
}

void DisaMapChunk::FUN_004a27f0_clearChunk(bool isFullClear) {
    isActive = false;
    position = glm::vec4(6.f, 0.f, 6.f, 1.f);
    rotation = glm::vec4(0.f, 0.f, 0.f, 1.f);
    pivot.x = 6.f;
    pivot.z = 6.f;

    for (mpdfileChunkEventTile& eventTile : eventTiles) {
        eventTile.index = 0;
    }
    for (mpdfileChunkObject& object : objects) {
        object.visible = 0;
    }

    if (isFullClear) {
        std::memset(tiles, 0, sizeof(tiles));
    } else {
        static_assert(offsetof(mpdfileTile, geo_color) == 0x78);
        for (mpdfileTile& tile : tiles) {
            std::memset(&tile, 0, offsetof(mpdfileTile, geo_color));
        }
    }
    std::memset(tileGrid, 0, sizeof(tileGrid));

    FUN_004fe080_destroyChunkMeshes(*this);

    //BEGIN custom code
    getCustomMapModels().erase(static_cast<int>(this - DisaMapChunk::get(0)));
    //END custom code
}

float DisaMapChunk::FUN_004a2af0_getHeight(float x, float z) const {
    constexpr float tileSize = 12.f;
    FUN_004e32a0_rotateXZ(static_cast<float>(rotation.y * double(57.29674f)), x - position.x, z - position.z);
    const float localX = pivot.x + FP::getData<float>(0x006b791c);
    const float localZ = pivot.z + FP::getData<float>(0x006b7a60);

    const int tileX = static_cast<int>(localX * double(1.f / tileSize));
    const int tileZ = static_cast<int>(localZ * double(1.f / tileSize));
    if (tileX < 0 || tileZ < 0 || tileX >= 40 || tileZ >= 40) return 0.f;

    const int tileIndex = tileGrid[tileX][tileZ] - 1;
    if (tileIndex < 0) return 0.f;
    const mpdfileTile& tile = tiles[tileIndex];
    if (!tile.visible) return 0.f;

    const float x0 = static_cast<float>(tile.x * 12);
    const float x1 = static_cast<float>(tile.x * 12 + 12);
    const float z0 = static_cast<float>(tile.z * 12);
    const float z1 = static_cast<float>(tile.z * 12 + 12);
    const glm::vec3 southWest(x0, static_cast<float>(tile.cornersTop[0]), z0);
    const glm::vec3 southEast(x1, static_cast<float>(tile.cornersTop[1]), z0);
    const glm::vec3 northWest(x0, static_cast<float>(tile.cornersTop[2]), z1);
    const glm::vec3 northEast(x1, static_cast<float>(tile.cornersTop[3]), z1);

    const auto alongEdge = [tileSize](const glm::vec3& from, const glm::vec3& to, float offset) {
        const float t = static_cast<float>(std::abs(offset) / double(tileSize));
        return static_cast<float>(from.y + double(to.y - from.y) * t);
    };
    if (std::abs(x0 - localX) < 0.01f) return alongEdge(southWest, northWest, localZ - z0);
    if (std::abs(x1 - localX) < 0.01f) return alongEdge(southEast, northEast, localZ - z0);
    if (std::abs(z0 - localZ) < 0.01f) return alongEdge(southWest, southEast, localX - x0);
    if (std::abs(z1 - localZ) < 0.01f) return alongEdge(northWest, northEast, localX - x0);

    const glm::vec3 point(localX, 0.f, localZ);
    if (FUN_0046e410_sideOfLine(&southEast, &northWest, &point) == 1) {
        return interpolateTileHeight(localX, localZ, southEast, northWest, northEast);
    }
    return interpolateTileHeight(localX, localZ, southWest, southEast, northWest);
}

//CUSTOM CODE START
std::optional<DisaMapChunk::CustomRayHit> DisaMapChunk::customIntersectRay(const glm::vec3& origin, const glm::vec3& direction) const {
    if (!faces || staticFaceCount <= 0) return std::nullopt;

    //the chunk matrix has no scale, so a distance in chunk space is the same distance in world space
    const glm::mat4 toWorld = computeChunkMatrix(*this);
    const glm::mat4 toChunk = glm::inverse(toWorld);
    const glm::vec3 localOrigin(toChunk * glm::vec4(origin, 1.f));
    const glm::vec3 localDirection(toChunk * glm::vec4(direction, 0.f));

    std::optional<CustomRayHit> nearest;
    const auto testTriangle = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        const std::optional<float> distance = PG_MathGeometry::intersectionTriangle(localOrigin, localDirection, a, b, c);
        if (!distance || (nearest && nearest->distance <= *distance)) return;
        const glm::vec3 normal = glm::normalize(glm::mat3(toWorld) * glm::cross(b - a, c - a));
        nearest = CustomRayHit{ *distance, normal };
    };
    for (const DisaMapFace& face : std::span(faces, staticFaceCount)) {
        const auto& v = face.vertices; //triangle strip
        testTriangle(v[0].pos, v[1].pos, v[2].pos);
        testTriangle(v[1].pos, v[3].pos, v[2].pos);
    }
    return nearest;
}
//CUSTOM CODE END

void FUN_004a4660_BuildTileOverlays(DisaMapChunk& chunk) {
    //one white texel of the system texture, used by the plain colored overlays
    const glm::vec2 solidUvMin(241.5f / 256.f, 1.5f / 256.f);
    const glm::vec2 solidUvMax(242.5f / 256.f, 2.5f / 256.f);

    DisaMapFace* face = chunk.faces + chunk.staticFaceCount;

    chunk.geoPanelFaceCount = 0;
    const bool showGeoPanels = FP::getData<int>(0x00664778) != 0;
    if (showGeoPanels && (FP::getData<uint32_t>(0x00664f04) & 4) == 0) {
        const uint8_t pulse = (&FP::getData<uint8_t>(0x00533498))[FP::getData<int>(0x006b7914) % 170];
        const uint32_t alpha = pulse + 20;
        for (const mpdfileTile& tile : chunk.tiles) {
            if (!tile.visible || tile.geo_mark == 0) continue;
            switch (tile.geo_color) { //colors 6 and up keep what the slot held last frame
                case 0: setColor(*face, 255, 56, 80, alpha); break;
                case 1: setColor(*face, 136, 255, 8, alpha); break;
                case 2: setColor(*face, 56, 96, 255, alpha); break;
                case 3: setColor(*face, 255, 200, 40, alpha); break;
                case 4: setColor(*face, 128, 32, 128, alpha); break;
                case 5: setColor(*face, 64, 255, 255, alpha); break;
            }
            writeOverlayFace(*face, chunk, tile, solidUvMin, solidUvMax);
            ++face;
            ++chunk.geoPanelFaceCount;
        }
    }

    const int frame60 = FP::getData<int>(0x006b7918); //0 to 59
    const uint32_t pulseAlpha = std::abs(frame60 - 30) * 4 + 60;
    const float spriteFrameU = (frame60 > 19 ? 8.f : 0.f) + (frame60 > 39 ? 8.f : 0.f);

    chunk.highlightFaceCount = 0;
    for (mpdfileTile& tile : chunk.tiles) {
        if (!tile.visible || highlightType(tile) == 0) continue;
        const uint8_t type = highlightType(tile);
        switch (type) { //5 to 9 and 24 and up keep what the slot held last frame
            case 1: setColor(*face, 255, 80, 80, pulseAlpha); break;
            case 2: setColor(*face, 255, 255, 80, pulseAlpha); break;
            case 3: setColor(*face, 80, 80, 255, pulseAlpha); break;
            case 4: setColor(*face, 0, 0, 255, pulseAlpha); break;
            default:
                if (type >= 10 && type <= 23) setColor(*face, 255, 255, 255, 255);
                break;
        }

        if (type < 10) {
            writeOverlayFace(*face, chunk, tile, solidUvMin, solidUvMax);
        } else {
            //7x7 sprites, one row per type, three animation frames 8 texels apart
            const glm::vec2 uvMin(spriteFrameU + 232.f, (type - 10) * 8.f + 96.f);
            writeOverlayFace(*face, chunk, tile, uvMin / 256.f, (uvMin + 7.f) / 256.f);
        }

        highlightType(tile) = 0;
        ++face;
        ++chunk.highlightFaceCount;
    }
}

void __fastcall FUN_004a4c60_SubmitMapChunk(DisaMapChunk* chunk, int unused, DisaCameraInfo* camera) {
    //the render queue keeps only a pointer to the matrix, so it has to outlive this call
    static glm::mat4 chunkMatrix(1.f);

    if (FP::getData<int>(0x0067728c) >= 0) return; //a fullscreen 2D image hides the map

    if (chunk->staticFaceCount != 0) {
        FUN_004a4660_BuildTileOverlays(*chunk);
        chunkMatrix = computeChunkMatrix(*chunk);

        const bool isBatched = getSettings()->isBatchedMapRendering();
        DisaFaceList overlays{ nullptr, 0 };
        DisaFaceList* toSubmit = &overlays;

        if (chunk->meshState == 1) {
            //uncached, all faces are submitted every frame
            chunk->meshFaces.faces = chunk->faces;
            chunk->meshFaces.count = chunk->staticFaceCount + chunk->highlightFaceCount + chunk->geoPanelFaceCount;
            toSubmit = &chunk->meshFaces;
        } else {
            if (chunk->builtWithBatching != static_cast<uint32_t>(isBatched)) FUN_004fe080_destroyChunkMeshes(*chunk);

            if (!chunk->opaqueMesh) {
                //building also queues the lists once, so they are not submitted again on this frame
                chunk->builtWithBatching = isBatched;
                chunk->meshFaces.faces = chunk->faces;
                chunk->meshFaces.count = chunk->staticFaceCount;
                chunk->opaqueMesh = FUN_004fde70_createChunkMesh(*chunk, chunk->isHDTexture ? 0x004f2600 : 0x004f25d0, &chunkMatrix, 0);
                chunk->blendedMesh = FUN_004fde70_createChunkMesh(*chunk, chunk->isHDTexture ? 0x004f2660 : 0x004f2630, &chunkMatrix, 2);
                glFlush();
            } else {
                if (chunk->meshState > 1) {
                    chunk->meshFaces.faces = chunk->faces;
                    chunk->meshFaces.count = chunk->staticFaceCount;
                    FP::callStdFastFarA<void>(0x004fdff0, chunk->opaqueMesh);
                    FP::callStdFastFarA<void>(0x004fdff0, chunk->blendedMesh);
                    ++chunk->meshState;
                    if (static_cast<uint32_t>(chunk->meshState) > 3) chunk->meshState = 0;
                }

                DisaRenderObjectManager* manager = DisaRenderObjectManager::get();
                pushRenderFlags().isFullyLit = 1;
                manager->FUN_00426200_submitDisplayList(40000.f, chunk->opaqueMesh->listId, 0xffffffff, &chunkMatrix, true, false, 0.f);
                manager->FUN_00426200_submitDisplayList(40005.f, chunk->blendedMesh->listId, 0xffffffff, &chunkMatrix, true, false, 0.f);
                popRenderFlags();
            }

            //the chunk buffer is rewritten next frame while the draw is deferred, so the overlays get a copy
            overlays.count = chunk->geoPanelFaceCount + chunk->highlightFaceCount;
            if (overlays.count >= 1) {
                const uint32_t bytes = overlays.count * sizeof(DisaMapFace);
                overlays.faces = static_cast<DisaMapFace*>(DisaRenderObjectManager::get()->FUN_00425bb0_allocFrameMemory(bytes));
                if (overlays.faces) std::memcpy(overlays.faces, chunk->faces + chunk->staticFaceCount, bytes);
                else toSubmit = nullptr; //the original copies into null and crashes
            } else {
                toSubmit = nullptr;
            }
        }

        if (toSubmit) FUN_004f02e0_FUN_004f1070_submitFaces(toSubmit, &chunkMatrix);
    }

    DisaRenderFlags& objectFlags = pushRenderFlags();
    objectFlags.isFullyLit = 1;
    objectFlags.field0x10 = 1;
    for (mpdfileChunkObject& object : chunk->objects) {
        if (!object.visible) continue;

        //BEGIN custom code
        if (object.isCustomModelsMarker()) {
            submitCustomMapModels(*chunk, chunkMatrix);
            continue;
        }
        //END custom code

        constexpr double degToRad = 0.017453;
        constexpr double percent = 0.01;
        DisaTransform transform;
        transform.position = glm::vec4(object.x, object.y, object.z, 1.f);
        transform.rotation = glm::vec4(glm::vec3(glm::dvec3(object.xRot, object.yRot, object.zRot) * degToRad), 1.f);
        transform.scale = glm::vec4(glm::vec3(glm::dvec3(object.xScale, object.yScale, object.zScale) * percent), 1.f);

        DisaGeoModel* model = &DisaGeoModelSet::get(chunk->modelSetIndex)->models[object.model_id];
        //the scroll hook lives on the shared model, so it is pointed at this object before every draw
        model->uvScroll.uvScrollX = &object.uvScrollX;
        model->uvScroll.uvScrollY = &object.uvScrollY;
        model->uvScroll.uvScrollSpeedX = reinterpret_cast<int8_t*>(&object.textureScrollX);
        model->uvScroll.uvScrollSpeedY = reinterpret_cast<int8_t*>(&object.textureScrollY);

        //only reaches the draw on the deferred instance path, as its polygon offset
        const int deferredPolygonOffset = static_cast<uint8_t>(static_cast<int64_t>(object.unk0x17 * 2.0));
        FP::callThisFarA<void>(fRender3DObject, model, static_cast<float>(object.animationFrame), camera, &transform, (float*)nullptr, &chunkMatrix,
            0.f, deferredPolygonOffset, 0.f);

        ++object.animationFrame;
        if (model->animationFrameMax <= object.animationFrame) object.animationFrame = 0;
    }
    popRenderFlags();
}

//BEGIN custom code
std::unordered_map<int, std::vector<DisaCustomMapModel>>& getCustomMapModels() {
    static std::unordered_map<int, std::vector<DisaCustomMapModel>> models;
    return models;
}

void submitCustomMapModels(const DisaMapChunk& chunk, const glm::mat4& chunkMatrix) {
    const auto findIt = getCustomMapModels().find(static_cast<int>(&chunk - DisaMapChunk::get(0)));
    if (findIt == getCustomMapModels().end()) return;

    DisaCustomRenderManager* renderManager = DisaCustomRenderManager::get();
    if (!renderManager->isEnabled()) return;

    constexpr double degToRad = 0.017453; //the factors of the vanilla object loop, so both land alike
    constexpr double percent = 0.01;
    /* The editor shows the map with x and y mirrored, which is the space the models are authored in.
     * That mirror is a half turn around z, so it folds into the rotation and the node keeps a plain scale. */
    const glm::mat3 editorToGame = glm::mat3(glm::rotate(glm::mat4(1.f), glm::pi<float>(), glm::vec3(0.f, 0.f, 1.f)));
    const glm::mat3 chunkRotation(chunkMatrix);

    for (const DisaCustomMapModel& custom : findIt->second) {
        const mpdfileChunkObject& object = custom.object;
        PG_DisaModelNode* node = renderManager->getNode<PG_DisaModelNode>(custom.nodeId, DisaCustomRenderManager::EGroup::World);
        if (node->getModel() != custom.model) node->setModel(custom.model);

        const glm::vec3 rotation(glm::dvec3(object.xRot, object.yRot, object.zRot) * degToRad);
        const glm::vec3 scale(glm::dvec3(object.xScale, object.yScale, object.zScale) * percent);
        //the order of computeChunkMatrix, assumed to be the one FUN_004e13a0 uses for the vanilla objects
        const glm::mat3 objectRotation(glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z));
        const glm::vec3 translation(chunkMatrix * glm::vec4(glm::vec3(object.x, object.y, object.z) - chunk.pivot, 1.f));

        node->setTransformation(translation, glm::quat_cast(chunkRotation * objectRotation * editorToGame), scale);
        //the event loop only runs at the start of the next frame, so the matrices are updated right away
        node->updateModelMatrix();
        node->updateModelMatrixWorld();

        //depthMaskEnabled is stored inverted by submitCustomModel, false keeps the depth writes on
        renderManager->submit(40002.f, node, -1, true, false, 0.f, 0, 0, false);
    }
}
//END custom code
