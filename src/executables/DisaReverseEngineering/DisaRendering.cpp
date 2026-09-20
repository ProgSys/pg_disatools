#include "DisaRendering.h"
#include <gl/glew.h>
#include <GL/glu.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <array>

#include <FunctionPatcher.h>
#include <Renderer/Command/PG_AddCommands.h>
#include <Managers/DisaCustomRenderManager.h> 


constexpr uint16_t g_customModelMarker = 9001;

bool FUN_004280a0_sortFunc(const DisaRenderObject* param_1, const DisaRenderObject* param_2) {
    assert(param_1 && param_2);
    if(!param_1 || !param_2) return param_1 > param_2;

    if (!glm::isnan(param_1->depth) && !glm::isnan(param_1->depth) && param_1->depth != param_2->depth)
        return  param_1->depth < param_2->depth;
    if (!glm::isnan(param_1->sortBias) && !glm::isnan(param_1->sortBias) && param_1->sortBias != param_2->sortBias)
        return  param_1->sortBias < param_2->sortBias;
    return param_1 < param_2; // tiebreaker by using raw pointer
}

void __fastcall FUN_00427da0_insertionSort(DisaRenderObject** begin, DisaRenderObject** end) {
    assert(begin <= end);
    //as compare function tests with the pointer as fallback the result is same as std::sort
    //std::stable_sort(begin, end, &FUN_004280a0);
    std::sort(begin, end, &FUN_004280a0_sortFunc);

    /* 
    glm::vec3** it;
    if ((begin == end) || (it = begin + 1, it == end))  return;
    
    int local_14 = (int)it - (int)end;
    do {
        glm::vec3* copy = *it;
        bool bVar5 = FUN_004280a0(*it, *begin);
        glm::vec3** ppVVar6 = it;
        if (bVar5) {
            //it is invalid here
            memmove(it + (1 - (local_14 >> 2)), begin, (local_14 >> 2) * 4);
            *begin = *it;
        }
        else {
            do {
                glm::vec3* pVVar4 = ppVVar6[-1];
                float fVar1 = copy->y; //curr
                float fVar2 = pVVar4->y; //prev
                if ((glm::isnan(fVar1) || glm::isnan(fVar2)) == (fVar1 == fVar2)) {
                    if (fVar2 <= fVar1) break;
                }
                else {
                    fVar1 = copy->z;
                    fVar2 = pVVar4->z;
                    if ((glm::isnan(fVar1) || glm::isnan(fVar2)) == (fVar1 == fVar2)) {
                        if (fVar2 <= fVar1) break;
                    }
                    else if (pVVar4 <= copy) break;
                }
                *ppVVar6 = pVVar4;
                --ppVVar6;
            } while (true);
            *ppVVar6 = copy;
        }
        local_14 += 4;
        ++it;
    } while (it != end);
   */
}

void __fastcall FUN_00427ff0_pushHeap(DisaRenderObject** begin, int index, int start, DisaRenderObject** insertValue) {
    int iVar1 = index - 1;
    if (start < index) {
        int iVar6;
        do {
            iVar6 = iVar1 / 2;
            auto copy = begin[iVar6];
            //same as copy < *insertValue
            if (!FUN_004280a0_sortFunc(copy, *insertValue)) {
                break;
            }

            iVar1 = iVar6 - 1;
            begin[index] = copy;
            index = iVar6; 
        } while (start < iVar6);
    }

    begin[index] = *insertValue;
   
}

void __fastcall FUN_00427c30_makeHeap(DisaRenderObject** begin, DisaRenderObject** end) {
    std::make_heap(begin, end, &FUN_004280a0_sortFunc); //create max-heap
}

using byte = std::uint8_t;


/**
* @brief One 2D camera: an orthographic projection over a virtual 480x272 screen,
* plus the scroll, zoom and cull rectangle derived from it.
*
* Screen space is y down, x = 0 left, y = 0 top. Depth runs 0 to 65535.
* Created by FUN_00432d40, recomputed by FUN_00432b40 (0x00432b40) after any field above cullRect changes.
* Size: 0x78
*/
struct DisaProjection {
    float viewLeft;     // 0x00 - default 0
    float viewRight;    // 0x04 - default 480
    float viewTop;      // 0x08 - default 0
    float viewBottom;   // 0x0c - default 272
    float depthFar;     // 0x10 - default 65535
    float depthNear;    // 0x14 - default 0, also added to the matrix as a depth bias
    float zoom;         // 0x18 - default 1, never 0, recompute divides by it
    float cameraX;      // 0x1c - scroll in virtual pixels
    float cameraY;      // 0x20 - scroll in virtual pixels
    glm::ivec4 cullRect;      // 0x24 - derived from everything above (left, right, top, bottom)
    bool isMatrixDirty;         // 0x34 - set by recompute, cleared once the matrix is uploaded
    bool padding0x35;           // 0x35 - unused
    bool padding0x36;           // 0x36
    bool padding0x37;           // 0x37
    glm::mat4 projectionMatrix; // 0x38 - ortho * rotateY(pi) * scale(zoom), then scrolled
};

/**
* @brief Owns the projection slots and remembers which one OpenGL currently holds.
* Singleton at 0x005CBBC0, created at 0x00508073, destroyed by FUN_00432e60.
* Render objects pick their slot through DisaRenderObjectSettings::projIndex.
* Size: 0x14
*/
struct DisaProjectionManager {
    int32_t projectionCount;      // 0x00 - always 2, bounds the array and the destructor
    DisaProjection** projections; // 0x04 - heap array of m_projectionCount pointers
    DisaProjection* selected;     // 0x08 - slot OpenGL holds, null until the first select
    int32_t selectedIndex;        // 0x0c
    bool isEnabled;               // 0x10 - false makes selecting a slot do nothing
    bool isForceReload;           // 0x11 - upload the matrix even if the slot did not change
    uint16_t padding0x12;         // 0x12 - unused

    /// Instance located at 0x005CBBC0
    static DisaProjectionManager* get() { return FP::getData<DisaProjectionManager*>(0x005CBBC0);  }
};

void FUN_00421a30_glUseAndInitProgram(int param_1, int param_2) {
    if (FP::getData<bool>(0x006e8cfc)) return;
    const int shaderIndex = param_1 + param_2 * 16;
    if (FP::getData<int>(0x006e8cf8) == shaderIndex) return;
    FP::getData<int>(0x006e8cf8) = shaderIndex; //lastShaderIndex
    GLuint programId = (&FP::getData<GLuint>(0x006e8878))[shaderIndex];
    if (programId == 0) return; //in the original this check is missing
    glUseProgram(programId);
    
    int i = 0;
    for (const auto& name : { "tex0", "tex1", "tex2", "tex3", "tex4", "tex5", "tex6" }) {
        if (auto loc = glGetUniformLocation(programId, "tex0"); loc >= 0)
            glUniform1i(loc, i);
        ++i;
    }
}

/**
* @brief Will select and update gui projection by index
*/
void FUN_00432f50_SelectProjection(uint32_t projectionIndex){
    //not sure jet what they are cooking here
    //this seems to be a very complex way to manage multiple projection matrices
    DisaProjectionManager* projectionManager = DisaProjectionManager::get();
    if (!projectionManager->isEnabled) return;

    DisaProjection* selected = projectionManager->projections[projectionIndex];
    if (selected != projectionManager->selected || projectionManager->isForceReload || selected->isMatrixDirty) {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf((float*)&selected->projectionMatrix);
    }

    selected->isMatrixDirty = false;
    projectionManager->isForceReload = false;
    projectionManager->selectedIndex = projectionIndex;
    projectionManager->selected = selected;
}

void FUN_0042d3e0_SetProjectionMatrix(glm::mat4& thisMatrix, float fovDegrees, float width, float height, float nearVal, float farVal) {
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();

    // tan(fov / 2)
    const float tanHalfFov = std::tan(fovDegrees * 0.5f * (glm::pi<float>() / 180.0f));

    // Match exact original math
    const float leftRight = (nearVal * tanHalfFov * width * 1.764706f) / 480.0f;
    const float topBottom = (nearVal * tanHalfFov * height) / 272.0f;
    //we can just use glm function instead of using a open gl call :P
    thisMatrix = glm::frustum(-leftRight, leftRight, -topBottom, topBottom, nearVal, farVal);
    //glFrustum(-leftRight, leftRight, -topBottom, topBottom, nearVal, farVal);
    //glGetFloatv(GL_PROJECTION_MATRIX, (float*)&thisMatrix);
}

/**
* Needs validation
*/
static inline bool isVisible(EDisaRenderPass pass, unsigned int visibleBitMask, DisaRenderFlags renderFlags, const DisaTextureSPtr* texture, float depth, int32_t vertexCount, bool isPerspective) {
    if (bool(visibleBitMask & 4) != (texture && texture->m_field0x18 == 3))
        return false;
    if ((pass == RenderPass_ShadowDepth) && !renderFlags.castsShadow)
        return false;
    if ((visibleBitMask & 0x40) && depth == 100.0f)
        return true;
    if ((visibleBitMask & 0x100) && depth == 40005.0f)
        return true;
    if ((visibleBitMask & 0x400) && depth == 45000.0f)
        return true;
    if ((visibleBitMask & 0x800) && depth == 45000.0f)
        return false;
    if ((visibleBitMask & 0x1000) && depth == 41000.0f && renderFlags.field0x10)
        return true;
    if ((visibleBitMask & 0x2000) && depth == 41000.0f && renderFlags.field0x10)
        return false;
    if ((visibleBitMask & 0x20) && depth == 50000.0f)
        return true;
    if ((visibleBitMask & 0x2000) && depth == 50000.0f)
        return false;
    if (!(visibleBitMask & 0x8) && (vertexCount != 0))
        return false;
    if (!(visibleBitMask & 0x10) && (vertexCount == 0))
        return false;
    if (!(visibleBitMask & 0x1) && (isPerspective != 0))
        return false;
    if (!(visibleBitMask & 0x2) && (isPerspective == 0) && (pass != RenderPass_Scene || depth != 100.0))
        return false;
    if (visibleBitMask & 0x20 && depth == 100.0)
        return false;
    if (-1 < (int)visibleBitMask && depth == 40005.0)
        return false;
    return true;
}

static inline void FUN_00420e50_glStencilOp(int dppassTypeIndex) {
    //INT_ARRAY_00596e0c
    static const std::array<GLenum, 8> dppassTypes = { GL_KEEP , GL_ZERO, GL_REPLACE, GL_INVERT, GL_INCR, GL_INCR_WRAP, GL_DECR, GL_DECR_WRAP };
    glStencilOp(GL_KEEP, GL_KEEP, dppassTypes[dppassTypeIndex]);
}

static PG_Object g_dummy;
static PG_CommandRendererJob g_job;
static PG_CommandRenderPass g_pass;

static inline glm::mat4* getProjectionMatrix(DisaRenderObjectManager* memoryRenderManager, DisaRenderObject* object, DisaProjectionManager* projectionManager, EDisaRenderPass pass) {
    const bool projectionIndex = object->renderSettingsBitMask.projIndex; //seams never to be true
    FUN_00432f50_SelectProjection(projectionIndex); //FP::callStdFarA(0x00432f50, stencilBitSet); 

    glm::mat4* projectionMat = object->projectionMatrix; // pfVar8
    if (!projectionMat) {
        if (object->isPerspective == false) {
            if (pass == RenderPass_ShadowDepth) return nullptr;
            //ui orthogonal matrix
            projectionMat = &projectionManager->projections[projectionIndex]->projectionMatrix;// &unknownObject->field0x4->field0x0[projectionIndex]->projectionMatrix;
        }
        else if (pass == RenderPass_ShadowDepth) {
            //glLoadIdentity(); //is not needed at all but present in original
            projectionMat = &memoryRenderManager->m_field0x760dc;
            const glm::mat4& staticMat = FP::getData<glm::mat4>(0x008191c0);
            *projectionMat = staticMat; //I am not sure what the point of this is, as SetProjectionMatrix will override it?!
            FUN_0042d3e0_SetProjectionMatrix(*projectionMat, 6.8f, 256.0f, 256.0f, 320.0f, 11600.0f); //FP::callThisFarA(0x0042d3e0, currMat, 0x40d9999a, 256.0, 256.0, 320.0, 11600.0);
        }
        else {
            projectionMat = memoryRenderManager->m_perspectiveMatrix;// (glm::mat4*)renderWorld[0x1d816];
        }
    }
    return projectionMat;
}

void _stdcall FUN_00426770(EDisaRenderPass pass, unsigned int visibleBitMask, bool depthTestEnabled) {

    DisaRenderObjectManager* memoryRenderManager = FP::getData<DisaRenderObjectManager*>(0x005bb02c);

    const unsigned int someObjPtr = FP::getData<unsigned int>(0x005bae70);
    if ( !*(bool*)(someObjPtr + 0x19) || *(bool*)(someObjPtr + 0x59) || *(bool*)(someObjPtr + 0x60))
        return;

    auto range = memoryRenderManager->getRenderObjectList();
    //sort if number of item has changed
    if (memoryRenderManager->m_renderObjectListSize != memoryRenderManager->m_prevRenderObjectListSize) {
        std::ranges::sort(range, &FUN_004280a0_sortFunc);
        //original function of a custom build sorting algorithm, however std sort does not only the same is even faster
        //unsigned int centerIndex = (FP::getData<unsigned int>(0x005bb02c) + FP::getData<unsigned int*>(0x005bb02c)[0x1d009] + 0x1d00a) - (int)(FP::getData<unsigned int>(0x005bb02c) + 0x1d00a) >> 2;
        //FUN_004275f0_sort(begin, end, centerIndex);
        memoryRenderManager->m_prevRenderObjectListSize = memoryRenderManager->m_renderObjectListSize;
    }

    //int staring values
    FP::getData<int>(0x005bb020) = 0;
    FP::getData<int>(0x005bb024) = 0;
    FP::getData<int>(0x005ac928) = -1;
    FP::getData<int>(0x005ac92c) = -1;
    FP::getData<int>(0x005ac920) = -1;

    int stencilModeValue = 0;
    int stencilRefValue = -1;

    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }

    bool depthMaskEnabled = depthTestEnabled;
    glDepthMask(depthTestEnabled);
    float polygonOffsetValue = 1.192093e-07f;
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.01f);
    FUN_00421a30_glUseAndInitProgram(10, 17); //FP::callStdFarA(0x00421a30, 10, 17); //glUseAndInitProgram; will call glUseProgram and then setup needed uniforms
   
    const auto* customDisaRenderManger = DisaCustomRenderManager::get();
    bool isInCustomState = false;
    g_pass.renderData.size = glm::ivec2(512);
    g_pass.renderData.cameraPos = memoryRenderManager->m_cameraCenter ? *memoryRenderManager->m_cameraCenter : glm::vec3(0.0F);
    g_pass.renderData.deltaTime = customDisaRenderManger->getDeltaTimer().getDeltaTime();
    g_pass.renderData.ticks = customDisaRenderManger->getDeltaTimer().getTotalTime();
    g_job.errorTexture = customDisaRenderManger->getErrorTexture();
    g_job.loadTexture = customDisaRenderManger->getLoadTexture();

    DisaProjectionManager* projectionManager = DisaProjectionManager::get();
    projectionManager->isEnabled = true;
    projectionManager->isForceReload = true;
    
    if (memoryRenderManager->m_renderBlockSize[memoryRenderManager->m_renderBlockIndex] <= 0) {
        //invalidate list
        range = std::ranges::subrange(memoryRenderManager->m_renderObjectList.begin(), memoryRenderManager->m_renderObjectList.begin());
    }

    for (DisaRenderObject* object : range) {
        assert(object);
        if (!object) return;

        if (!isVisible(pass, visibleBitMask, object->renderFlags, object->texture, object->depth, object->vertexCount, object->isPerspective))
            continue;

        glm::mat4* projectionMat = getProjectionMatrix(memoryRenderManager, object, projectionManager, pass);

        //render custom object
        if (object->primitiveType == g_customModelMarker) {
            if (!customDisaRenderManger->isEnabled()) continue;
            const auto& models = customDisaRenderManger->getModels();
            if (object->vertexCount >= models.size()) continue;
            const PG_DisaModel& model = models[object->vertexCount];
            if (!model.isLoaded()) continue;
            if (!model.getMaterial(0).getShader().isLoaded()) continue;

            g_pass.renderData.projectionMatrix = *projectionMat;
            g_pass.renderData.viewMatrix = glm::mat4(1.0F);

            if (!isInCustomState) {
                DisaCustomRenderManager::resetDisaOpenGLStates();
                PG_Context::runCreate();
                isInCustomState = true;
            }

            const glm::mat4 modelMatrix = object->modelMatrix ? *object->modelMatrix : glm::mat4(1.0F);
            addResModel(model, &g_dummy, modelMatrix, g_job, g_pass, model.additionalProperties);

            glDisable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            g_pass.runCurrentAndClearAll();
            continue;
        }
        if (isInCustomState) {
            glPopAttrib();
            isInCustomState = false;
        }

        //projection matrix
        glMatrixMode(GL_PROJECTION); 
        glLoadMatrixf((GLfloat*)projectionMat);

        //modelview matrix
        glMatrixMode(GL_MODELVIEW);  
        if (object->isPerspective == false) {
            glLoadIdentity();
        }
        else {
            if (pass == RenderPass_ShadowDepth && memoryRenderManager->m_cameraEye && memoryRenderManager->m_cameraCenter && memoryRenderManager->m_cameraUp) {
                glLoadIdentity();
                assert(false);

                //why is it not the same as DisaCameraInfo?
                const glm::vec3 center = *memoryRenderManager->m_cameraCenter;
                const glm::vec3 dir = memoryRenderManager->m_lookDir * -2000.0f;
                const glm::vec3 eye = center + dir;

                // pfVar5 = FP::callThisFarA<float*>(0x004281b0, &local_44, local_64, 2000.0); //local_64 = local_44 * 2000.0 -> address of local_64
                // pfVar5 = FP::callThisFarA<float*>(0x00428170, &local_30, local_58, &local_44); //local_58 = local_30 + local_44 -> address of local_58

                gluLookAt((double)eye.x, (double)eye.y, (double)eye.z, (double)center.x, (double)center.y, (double)center.z, 0.0, -1.0, 0.0);
                glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&memoryRenderManager->m_field0x7609c);
                glLoadIdentity();
                glLoadMatrixf((float*)&memoryRenderManager->m_field0x7609c);
                glMatrixMode(GL_PROJECTION);
                glGetFloatv(GL_PROJECTION_MATRIX, (float*)&memoryRenderManager->m_field0x760dc);
                glMatrixMode(GL_MODELVIEW);
            }
            else {
                glLoadMatrixf((float*)&memoryRenderManager->m_field0x7605c);//(float*)(renderWorld + 0x1d817)
            }
        }

        glPushMatrix();
        if (object->modelMatrix != nullptr) {
            glMultMatrixf((GLfloat*)object->modelMatrix);
        }

        //update some matrix? maybe modelMatrix, maybe multiply with parent?
        if (object->isPerspective == false || object->modelMatrix == nullptr) {
            object->modelViewMatrix = FP::getData<glm::mat4>(0x005b2670);
        }
        else {
            //mutiply view matrix with model matrix
            const glm::mat4 temp = memoryRenderManager->m_field0x7605c; //renderWorld + 0x1d817
            object->modelViewMatrix = *object->modelMatrix * temp; //original does this through FUN_0042CED0
        }

        //update stencil
        const int stencilRefTemp = object->renderSettingsBitMask.stencilRef;
        const int stencilModeTemp = object->renderSettingsBitMask.stencilMode;
        if (stencilModeValue != stencilModeTemp || stencilRefValue != stencilRefTemp) {
            stencilRefValue = stencilRefTemp;
            stencilModeValue = stencilModeTemp;
            switch (stencilModeValue) {
            case 0:
                glDisable(GL_STENCIL_TEST);
                glStencilFunc(GL_ALWAYS, 0, 0);
                FUN_00420e50_glStencilOp(0); //0x00420e50 calls glStencilOp(0x1e00,0x1e00,*(undefined4 *)(&DAT_00596e0c + param_1 * 4));
                break;
            case 1:
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_ALWAYS, stencilRefValue, 0xff);
                FUN_00420e50_glStencilOp(2); //glStencilOp
                break;
            case 2:
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_NOTEQUAL, stencilRefValue, 0xff);
                FUN_00420e50_glStencilOp(0); //glStencilOp
                break;
            case 3:
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_NOTEQUAL, stencilRefValue, 0xff);
                FUN_00420e50_glStencilOp(2); //glStencilOp
                break;
            }
        }

        //update depth test
        if (depthTestEnabled != object->depthTestEnabled) {
            depthTestEnabled = object->depthTestEnabled;
            if (depthTestEnabled) {
                glEnable(GL_DEPTH_TEST);
            }
            else {
                glDisable(GL_DEPTH_TEST);
            }
        }

        //update depth mask
        if (depthMaskEnabled != object->depthMaskEnabled) {
            depthMaskEnabled = object->depthMaskEnabled;
            glDepthMask(depthMaskEnabled);
        }

        //update Polygon Offset
        if (!glm::isnan(object->polygonOffset) && object->polygonOffset != polygonOffsetValue) {
            polygonOffsetValue = object->polygonOffset;
            const float polygonOffsetScale = FP::getData<float>(0x005ac924);
            glPolygonOffset(0.0, polygonOffsetScale * polygonOffsetValue);
        }

        FP::callThisFarA(0x00426fb0, object, false, pass); //render object
        glPopMatrix();
    }
    //reset back to normal
    if (isInCustomState) glPopAttrib();
    
    projectionManager->isEnabled = false;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

DisaRenderObjectManager* DisaRenderObjectManager::get() {
    return FP::getData<DisaRenderObjectManager*>(0x005bb02c);
}

void DisaRenderObjectManager::submitVertexBasic(float depth, int* textureDataPtr, void* geometryData, int triangleCount, short primitiveType, int blendMode, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled,
    float poligonOffset, int stencilMode, unsigned char stencilRef, bool useColorTint, bool customData) {

    //char* puStack_c;
    //void* local_10;
    //puStack_c = &LAB_00524f48;
    //local_10 = ExceptionList;
    //ExceptionList = &local_10;

    const unsigned int someObjPtr = FP::getData<unsigned int>(0x005bae70);
    const bool isEnabled = *(bool*)(someObjPtr + 0x19) && !*(bool*)(someObjPtr + 0x59) && !*(bool*)(someObjPtr + 0x60);


    if ((*(bool*)&m_field0xc == false) && isEnabled) {

        if (m_renderBlockSize[m_renderBlockIndex] < 2048) {
            DisaRenderObject* renderObject = &m_renderObjects[m_renderBlockIndex * 2048 + m_renderBlockSize[m_renderBlockIndex]];

            renderObject->depth = m_depthOffset + depth;
            renderObject->sortBias = m_sortBias;

            FP::callThisFarA<void>(0x0041e560, &renderObject->texture, &textureDataPtr);
            ((DisaTextureSPtr*)textureDataPtr)->FUN_004281f0(&renderObject->textureInfo);
            //FP::callThisFarA<void>(0x004281f0, textureDataPtr, (short*)&renderObject->m_field0x18);

            renderObject->renderSettingsBitMask.projIndex = DisaProjectionManager::get()->selectedIndex;
            renderObject->renderSettingsBitMask.blendMode = blendMode;
            renderObject->renderSettingsBitMask.stencilMode = stencilMode;
            renderObject->renderSettingsBitMask.stencilRef = stencilRef;

            renderObject->primitiveType = primitiveType;
            renderObject->modelMatrix = modelMatrix;
            renderObject->isPerspective = isPerspective;
            renderObject->depthTestEnabled = FP::getData<bool>(0x005ac978);
            renderObject->depthMaskEnabled = depthMaskEnabled == false;
            renderObject->useColorTint = useColorTint;
            renderObject->vertexCount = triangleCount;
            renderObject->polygonOffset = poligonOffset;

            const int flagStackDepth = FP::getData<int>(0x005bb030);
            renderObject->renderFlags = (&FP::getData<DisaRenderFlags>(0x005bb038))[flagStackDepth];

            if (m_modelMatrix) {
                renderObject->modelMatrix = m_modelMatrix;
            }
            if (m_projectionMatrix) {
                renderObject->projectionMatrix = m_projectionMatrix;
            }

            void* _Src = geometryData;
            //CUSTOM memory assignment to support big geometry 
            if (customData) {
                renderObject->meshData = (float*)_Src;
            }
            else {
                if ((_Src < m_field0x76030[m_renderBlockIndex]) || (m_field0x76030[m_renderBlockIndex] + 0x40000 <= _Src)) {
                    size_t _Size = triangleCount * 24;
                    float* _Dst = (float*)FP::callThisFarA<int>(0x00425bb0, this, _Size);
                    memcpy(_Dst, _Src, _Size);
                    renderObject->meshData = _Dst;
                }
                else {
                    renderObject->meshData = (float*)_Src;
                }
            }

            if (!m_skipGlSetup) {
                if (m_renderObjectListSize < 2048) {
                    // The original only sets the value if the slot is full
                    // But then we would leave null slots (nullptr in redering)
                    // so I don't get the point of this
                    //if (m_renderObjectList[m_renderObjectListSize]) 
                        m_renderObjectList[m_renderObjectListSize] = renderObject;
                    ++m_renderObjectListSize;
                }
                ++m_renderBlockSize[m_renderBlockIndex];
            }
            else {
                if (blendMode + 1U < 4) {
                    FP::callFastFarA(0x00426f10, blendMode + 1U); //setup of glBlendFunc
                }
                FP::callThisFarA(0x00426fb0, renderObject, m_skipGlSetup, 0); //render
            }
        }
    }

    int iVar3;
    int* ptr = textureDataPtr;
    if (((textureDataPtr != (int*)0x0) && ((int)textureDataPtr - 0x10001U < 0xeffeffff)) && ((*textureDataPtr != 0 && (iVar3 = *textureDataPtr - 1, *textureDataPtr = iVar3, iVar3 == 0)))) {
        FP::callFastFarA(0x004243e0, (int)textureDataPtr);
        FP::callFastFarA(0x00433380, (byte*)ptr);
    }
    //ExceptionList = local_10;
    return;
}

void DisaRenderObjectManager::submitCustomModel(float depth, int modelIndex, int blendMode, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled, float poligonOffset, int stencilMode, unsigned char stencilRef, bool useColorTint) {
    const unsigned int someObjPtr = FP::getData<unsigned int>(0x005bae70);
    const bool isEnabled = *(bool*)(someObjPtr + 0x19) && !*(bool*)(someObjPtr + 0x59) && !*(bool*)(someObjPtr + 0x60);
    if (!(*(bool*)&m_field0xc == false) || !isEnabled) return;

    if (m_renderBlockSize[m_renderBlockIndex] >= 2048) return; //render queue full
    DisaRenderObject* renderObject = &m_renderObjects[m_renderBlockIndex * 2048 + m_renderBlockSize[m_renderBlockIndex]];

    renderObject->depth = m_depthOffset + depth;
    renderObject->sortBias = m_sortBias;
    renderObject->texture = nullptr; //no texture
    //mark as special
    renderObject->primitiveType = g_customModelMarker;
    renderObject->vertexCount = modelIndex; //vertexCount

    renderObject->renderSettingsBitMask.projIndex = DisaProjectionManager::get()->selectedIndex;
    renderObject->renderSettingsBitMask.blendMode = blendMode;
    renderObject->renderSettingsBitMask.stencilMode = stencilMode;
    renderObject->renderSettingsBitMask.stencilRef = stencilRef;

    renderObject->modelMatrix = modelMatrix;
    renderObject->isPerspective = isPerspective;
    renderObject->depthTestEnabled = FP::getData<bool>(0x005ac978);
    renderObject->depthMaskEnabled = depthMaskEnabled == false;
    renderObject->useColorTint = useColorTint;
    renderObject->polygonOffset = poligonOffset;

    const int flagStackDepth = FP::getData<int>(0x005bb030);
    renderObject->renderFlags = (&FP::getData<DisaRenderFlags>(0x005bb038))[flagStackDepth];

    if (m_modelMatrix) {
        renderObject->modelMatrix = m_modelMatrix;
    }
    if (m_projectionMatrix) {
        renderObject->projectionMatrix = m_projectionMatrix;
    }

    renderObject->meshData = nullptr;

    if (m_skipGlSetup) return;

    if (m_renderObjectListSize < 2048) {
        // if (m_renderObjectList[m_renderObjectListSize] != nullptr) {
        m_renderObjectList[m_renderObjectListSize] = renderObject;
        //}
        ++m_renderObjectListSize;
    }
    ++m_renderBlockSize[m_renderBlockIndex];
}

void __stdcall FUN_00425df0_SubmitVertexBasic(float depth, int* textureDataPtr, void* geometryData, int triangleCount, short primitiveType, int blendMode, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled,
    float poligonOffset, int param_11, bool param_12, bool useColorTint){

    DisaProjectionManager* projectionManager = DisaProjectionManager::get();
    DisaRenderObjectManager* roManager = FP::getData<DisaRenderObjectManager*>(0x005bb02c);

    //char* puStack_c;
    //void* local_10;
    //puStack_c = &LAB_00524f48;
    //local_10 = ExceptionList;
    //ExceptionList = &local_10;

    const unsigned int someObjPtr = FP::getData<unsigned int>(0x005bae70);
    const bool isEnabled = *(bool*)(someObjPtr + 0x19) && !*(bool*)(someObjPtr + 0x59) && !*(bool*)(someObjPtr + 0x60);


    DisaRenderObject* renderObject;
    if ((*(bool*)&roManager->m_field0xc == false) && isEnabled) {

        if (roManager->m_renderBlockSize[roManager->m_renderBlockIndex] < 2048) {
            renderObject = &roManager->m_renderObjects[roManager->m_renderBlockIndex * 2048 + roManager->m_renderBlockSize[roManager->m_renderBlockIndex]];

            renderObject->depth = roManager->m_depthOffset + depth;
            renderObject->sortBias = roManager->m_sortBias;

            FP::callThisFarA<void>(0x0041e560, &renderObject->texture, &textureDataPtr);
            FP::callThisFarA<void>(0x004281f0, textureDataPtr, &renderObject->textureInfo);

            renderObject->primitiveType = primitiveType;
            renderObject->modelMatrix = modelMatrix;
            int& mask = *((int*)&renderObject->renderSettingsBitMask);
            mask = mask ^ (projectionManager->selectedIndex << 0xb ^ mask) & 0x800;
            mask = mask ^ (blendMode << 0xc ^ mask) & 0xf000;
            mask = mask ^ (param_11 << 8 ^ mask) & 0x700;
            *(bool*)&renderObject->renderSettingsBitMask = param_12;

            renderObject->isPerspective = isPerspective;
            renderObject->depthTestEnabled = FP::getData<bool>(0x005ac978);
            renderObject->depthMaskEnabled = depthMaskEnabled == false;
            renderObject->useColorTint = useColorTint;
            renderObject->vertexCount = triangleCount;
            renderObject->polygonOffset = poligonOffset;

            const int flagStackDepth = FP::getData<int>(0x005bb030);
            renderObject->renderFlags = (&FP::getData<DisaRenderFlags>(0x005bb038))[flagStackDepth];

            if (roManager->m_modelMatrix) {
                renderObject->modelMatrix = (glm::mat4*)roManager->m_modelMatrix;
            }
            if (roManager->m_projectionMatrix) {
                renderObject->projectionMatrix = roManager->m_projectionMatrix;
            }

            void* _Src = geometryData;
            if ((_Src < roManager->m_field0x76030[roManager->m_renderBlockIndex]) || (roManager->m_field0x76030[roManager->m_renderBlockIndex] + 0x40000 <= _Src)) {
                size_t _Size = triangleCount * 24;
                float* _Dst = (float*)FP::callThisFarA<int>(0x00425bb0, roManager, _Size);
                memcpy(_Dst, _Src, _Size);
                renderObject->meshData = _Dst;
            }
            else {
                renderObject->meshData = (float*)_Src;
            }

            if (!roManager->m_skipGlSetup) {
                if (roManager->m_renderObjectListSize < 2048) {
                    if (roManager->m_renderObjectList[roManager->m_renderObjectListSize] != nullptr) {
                        roManager->m_renderObjectList[roManager->m_renderObjectListSize] = renderObject;
                    }
                    ++roManager->m_renderObjectListSize;
                }
                ++roManager->m_renderBlockSize[roManager->m_renderBlockIndex];
            }
            else {
                if (blendMode + 1U < 4) {
                    FP::callFastFarA(0x00426f10, blendMode + 1U); //setup of glBlendFunc
                }
                FP::callThisFarA(0x00426fb0, renderObject, roManager->m_skipGlSetup, 0); //render
            }
        }
    }

    int iVar3;
    int* ptr = textureDataPtr;
    if (((textureDataPtr != (int*)0x0) && ((int)textureDataPtr - 0x10001U < 0xeffeffff)) && ((*textureDataPtr != 0 && (iVar3 = *textureDataPtr - 1, *textureDataPtr = iVar3, iVar3 == 0)))) {
        FP::callFastFarA(0x004243e0, (int)textureDataPtr);
        FP::callFastFarA(0x00433380, (byte*)ptr);
    }
    //ExceptionList = local_10;
    return;
}