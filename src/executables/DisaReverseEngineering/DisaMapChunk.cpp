#include "DisaMapChunk.h"

#include <gl/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <cstring>

#include <FunctionPatcher.h>
#include "DisaRendering.h"

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

    /// FUN_004f02e0 or FUN_004f1070, both with every blend tag and without normal mapping.
    void submitFaces(DisaFaceList* faces, glm::mat4* modelMatrix) {
        const unsigned int function = getSettings()->isBatchedMapRendering() ? 0x004f02e0 : 0x004f1070;
        FP::callFastFarA<void>(function, faces, modelMatrix, 0, -1);
    }

    /**
    * @brief FUN_004fde70: records the callback into a new display list and queues the list once.
    * @param pass 0 for the opaque faces at depth 40000, 2 for the blended faces at depth 40005.
    */
    DisaCachedMesh* createChunkMesh(DisaMapChunk& chunk, unsigned int callback, glm::mat4* modelMatrix, int pass) {
        const int batched = getSettings()->isBatchedMapRendering() ? 1 : 0;
        //the callback is passed a second time on the stack, the builder never reads it
        return FP::callFastFarA<DisaCachedMesh*>(0x004fde70, callback, &chunk.meshFaces, callback, modelMatrix, pass, batched);
    }

    void destroyChunkMeshes(DisaMapChunk& chunk) {
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

} //namespace

DisaGeoModelSet* DisaGeoModelSet::get(int index) {
    return &(&FP::getData<DisaGeoModelSet>(0x00703480))[index];
}

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
            if (chunk->builtWithBatching != static_cast<uint32_t>(isBatched)) destroyChunkMeshes(*chunk);

            if (!chunk->opaqueMesh) {
                //building also queues the lists once, so they are not submitted again on this frame
                chunk->builtWithBatching = isBatched;
                chunk->meshFaces.faces = chunk->faces;
                chunk->meshFaces.count = chunk->staticFaceCount;
                chunk->opaqueMesh = createChunkMesh(*chunk, chunk->isHDTexture ? 0x004f2600 : 0x004f25d0, &chunkMatrix, 0);
                chunk->blendedMesh = createChunkMesh(*chunk, chunk->isHDTexture ? 0x004f2660 : 0x004f2630, &chunkMatrix, 2);
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
                manager->submitDisplayList(40000.f, chunk->opaqueMesh->listId, 0xffffffff, &chunkMatrix, true, false, 0.f);
                manager->submitDisplayList(40005.f, chunk->blendedMesh->listId, 0xffffffff, &chunkMatrix, true, false, 0.f);
                popRenderFlags();
            }

            //the chunk buffer is rewritten next frame while the draw is deferred, so the overlays get a copy
            overlays.count = chunk->geoPanelFaceCount + chunk->highlightFaceCount;
            if (overlays.count >= 1) {
                const uint32_t bytes = overlays.count * sizeof(DisaMapFace);
                overlays.faces = static_cast<DisaMapFace*>(DisaRenderObjectManager::get()->allocFrameMemory(bytes));
                if (overlays.faces) std::memcpy(overlays.faces, chunk->faces + chunk->staticFaceCount, bytes);
                else toSubmit = nullptr; //the original copies into null and crashes
            } else {
                toSubmit = nullptr;
            }
        }

        if (toSubmit) submitFaces(toSubmit, &chunkMatrix);
    }

    DisaRenderFlags& objectFlags = pushRenderFlags();
    objectFlags.isFullyLit = 1;
    objectFlags.field0x10 = 1;
    for (mpdfileChunkObject& object : chunk->objects) {
        if (!object.visible) continue;

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
