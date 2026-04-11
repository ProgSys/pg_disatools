#include "DisaRendering.h"
#include <gl/glew.h>
#include <GL/glu.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <array>

#include <FunctionPatcher.h>


bool FUN_004280a0_sortFunc(const DisaRenderObject* param_1, const DisaRenderObject* param_2) {
    assert(param_1 && param_2);
    if (!glm::isnan(param_1->m_depth) && !glm::isnan(param_1->m_depth) && param_1->m_depth != param_2->m_depth)
        return  param_1->m_depth < param_2->m_depth;
    if (!glm::isnan(param_1->m_field0x8) && !glm::isnan(param_1->m_field0x8) && param_1->m_field0x8 != param_2->m_field0x8)
        return  param_1->m_field0x8 < param_2->m_field0x8;
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



struct DisaProjectionObject {
    std::array<float, 13> field0x0;
    bool field0x34; //0x34 isInit?
    bool field0x35;
    bool field0x36;
    bool field0x37;
    glm::mat4 projectionMatrix; //0x38
};

struct DisaUnknownObject3 {
    DisaProjectionObject* field0x0[2];
    unsigned int field0x8;
    unsigned int field0xc;
};

struct DisaUnknownObject2 {
    uint32_t field0x0; //0x0
    DisaUnknownObject3* field0x4; //0x4 option
    DisaProjectionObject* selectedProjectionObject; //0x8 selected option by param_1
    uint32_t selectedProjectionObjectIndex; //0xc is set by param_1 is 0 or 1
    bool field0x10_isEnabled; //0x10 isEnabled
    bool field0x11; //0x11
    unsigned short field0x12; //0x12
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
void FUN_00432f50_ProjectionUpdate(uint32_t param_1){
    //not sure jet what they are cooking here
    //this seems to be a very complex way to manage multiple projection matrices
    DisaUnknownObject2* unknownObject = FP::getData<DisaUnknownObject2*>(0x005cbbc0);
    if (!unknownObject->field0x10_isEnabled) return;

    DisaProjectionObject* selected = unknownObject->field0x4->field0x0[param_1];
    if (selected != unknownObject->selectedProjectionObject || unknownObject->field0x11 || selected->field0x34 ) {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf((float*)&selected->projectionMatrix);
    }

    selected->field0x34 = false;
    unknownObject->field0x11 = false;
    unknownObject->selectedProjectionObjectIndex = param_1;
    unknownObject->selectedProjectionObject = selected;
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
static inline bool isVisible(int param_1, unsigned int visibleBitMask, uint8_t type, int32_t* field0x14, float depth, int32_t field0x10, bool isPerspective) {
    if (bool(visibleBitMask & 4) != (field0x14 && field0x14[6] == 3))
        return false;
    if ((param_1 == 1) && ((type & 1) == 0))
        return false;
    if ((visibleBitMask & 0x40) && depth == 100.0f)
        return true;
    if ((visibleBitMask & 0x100) && depth == 40005.0f)
        return true;
    if ((visibleBitMask & 0x400) && depth == 45000.0f)
        return true;
    if ((visibleBitMask & 0x800) && depth == 45000.0f)
        return false;
    if ((visibleBitMask & 0x1000) && depth == 41000.0f && (type & 0x10))
        return true;
    if ((visibleBitMask & 0x2000) && depth == 41000.0f && (type & 0x10))
        return false;
    if ((visibleBitMask & 0x20) && depth == 50000.0f)
        return true;
    if ((visibleBitMask & 0x2000) && depth == 50000.0f)
        return false;
    if (!(visibleBitMask & 0x8) && (field0x10 != 0))
        return false;
    if (!(visibleBitMask & 0x10) && (field0x10 == 0))
        return false;
    if (!(visibleBitMask & 0x1) && (isPerspective != 0))
        return false;
    if (!(visibleBitMask & 0x2) && (isPerspective == 0) && (param_1 != 2 || depth != 100.0))
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


void _stdcall FUN_00426770(int param_1, unsigned int visibleBitMask, bool depthTestEnabled) {
    //@todo move this to main
    static bool openGLInit = false;
    if (!openGLInit) {
        glewExperimental = GL_TRUE; // recommended
        if (glewInit() != GLEW_OK) {
            std::cout << "Failed to init GLEW!" << std::endl;
        }
        openGLInit = true;
    }

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
   

    DisaUnknownObject2* unknownObject = FP::getData<DisaUnknownObject2*>(0x005cbbc0);
    unknownObject->field0x10_isEnabled = true;
    unknownObject->field0x11 = true;
    
    if (memoryRenderManager->m_renderBlockSize[memoryRenderManager->m_renderBlockIndex] <= 0) {
        //invalidate list
        range = std::ranges::subrange(memoryRenderManager->m_renderObjectList.begin(), memoryRenderManager->m_renderObjectList.begin());
    }

    for (DisaRenderObject* object : range) {
        if (!isVisible(param_1, visibleBitMask, object->m_type, object->m_field0x14, object->m_depth, object->m_field0x10, object->m_isPerspective))
            continue;

        const bool stencilBitSet = object->m_stencilSetting >> 0xb & 1; //seams never to be true
        FUN_00432f50_ProjectionUpdate(stencilBitSet); //FP::callStdFarA(0x00432f50, stencilBitSet); // somekind of matrix setup

        //projection matrix
        glMatrixMode(GL_PROJECTION); 
        glm::mat4* projectionMat = object->m_projectionMatrix; // pfVar8
        if (projectionMat == nullptr) {
            if (object->m_isPerspective == false) {
                if (param_1 == 1) continue;
                //ui orthogonal matrix
                projectionMat = &unknownObject->field0x4->field0x0[stencilBitSet]->projectionMatrix;
            }
            else if (param_1 == 1) {
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
        glLoadMatrixf((GLfloat*)projectionMat);

        //modelview matrix
        glMatrixMode(GL_MODELVIEW);  
        if (object->m_isPerspective == false) {
            glLoadIdentity();
        }
        else {
            if (param_1 == 1 && memoryRenderManager->m_field0x0 && memoryRenderManager->m_center && memoryRenderManager->m_field0x8) {
                glLoadIdentity();
                assert(false);

                //why is it not the same as DisaCameraInfo?
                const glm::vec3 center = *memoryRenderManager->m_center;
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
        if (object->m_modelMatrix != nullptr) {
            glMultMatrixf((GLfloat*)object->m_modelMatrix);
        }

        //update some matrix? maybe modelMatrix, maybe multiply with parent?
        if (object->m_isPerspective == false || object->m_modelMatrix == nullptr) {
            object->m_field0x34 = FP::getData<glm::mat4>(0x005b2670);
        }
        else {
            //mutiply view matrix with model matrix
            const glm::mat4 temp = memoryRenderManager->m_field0x7605c; //renderWorld + 0x1d817
            object->m_field0x34 = *object->m_modelMatrix * temp; //  FP::callFastFarA(0x0042ced0, &object->m_field0x34, object->m_field0x20, &temp);
        }

        //update stencil
        const int stencilRefTemp = int(object->m_stencilSetting & 0xff);
        const int stencilModeTemp = (int)(object->m_stencilSetting << 0x15) >> 0x1d;
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
        if (depthTestEnabled != object->m_depthTestEnabled) {
            depthTestEnabled = object->m_depthTestEnabled;
            if (depthTestEnabled) {
                glEnable(GL_DEPTH_TEST);
            }
            else {
                glDisable(GL_DEPTH_TEST);
            }
        }

        //update depth mask
        if (depthMaskEnabled != object->m_depthMaskEnabled) {
            depthMaskEnabled = object->m_depthMaskEnabled;
            glDepthMask(depthMaskEnabled);
        }

        //update Polygon Offset
        if (!glm::isnan(object->m_polygonOffset) && object->m_polygonOffset != polygonOffsetValue) {
            polygonOffsetValue = object->m_polygonOffset;
            const float polygonOffsetScale = FP::getData<float>(0x005ac924);
            glPolygonOffset(0.0, polygonOffsetScale * polygonOffsetValue);
        }

        FP::callThisFarA(0x00426fb0, object, false, param_1); //render object
        glPopMatrix();
    }

    unknownObject->field0x10_isEnabled = 0;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void __stdcall FUN_00425df0_SubmitVertexBasic(float param_1, int* textureDataPtr, void* geometryData, int triangleCount, short sizeUniforms, int param_6, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled,
    float poligonOffset, int param_11, bool param_12, bool param_13){
    DisaRenderObjectManager* _this;
    DisaUnknownObject2* pDVar2;
    int* ptr;
    void* _Src;
    float* _Dst;
    int iVar3;
    DisaRenderObject* renderObject;
    size_t _Size;
    void* local_10;
    char* puStack_c;
    int local_8;

    iVar3 = triangleCount;
    _Src = geometryData;
    pDVar2 = FP::getData<DisaUnknownObject2*>(0x005cbbc0);
    _this = FP::getData<DisaRenderObjectManager*>(0x005bb02c);
    //puStack_c = &LAB_00524f48;
    //local_10 = ExceptionList;
    //ExceptionList = &local_10;
    local_8 = 0;

    const unsigned int someObjPtr = FP::getData<unsigned int>(0x005bae70);
    const bool isEnabled = *(bool*)(someObjPtr + 0x19) && !*(bool*)(someObjPtr + 0x59) && !*(bool*)(someObjPtr + 0x60);


    if ((*(bool*)&_this->m_field0xc == false) && isEnabled) {

        if (_this->m_renderBlockSize[_this->m_renderBlockIndex] < 2048) {
            renderObject = &_this->m_renderObjects[_this->m_renderBlockIndex * 2048 + _this->m_renderBlockSize[_this->m_renderBlockIndex]];

            renderObject->m_depth = _this->m_depthOffset + param_1;
            renderObject->m_field0x8 = _this->m_field0x76048;
            renderObject->m_stencilSetting = renderObject->m_stencilSetting ^ (pDVar2->selectedProjectionObjectIndex << 0xb ^ renderObject->m_stencilSetting) & 0x800;

            FP::callThisFarA<void>(0x0041e560, &renderObject->m_field0x14, &textureDataPtr);
            FP::callThisFarA<void>(0x004281f0, textureDataPtr, (short*)&renderObject->m_field0x18);

            renderObject->m_stencilSetting = renderObject->m_stencilSetting ^ (param_6 << 0xc ^ renderObject->m_stencilSetting) & 0xf000;
            *(short*)((int)&renderObject->m_stencilSetting + 2) = sizeUniforms;
            renderObject->m_modelMatrix = modelMatrix;
            renderObject->m_stencilSetting = renderObject->m_stencilSetting ^ (param_11 << 8 ^ renderObject->m_stencilSetting) & 0x700;
            *(bool*)&renderObject->m_stencilSetting = param_12;
            renderObject->m_isPerspective = isPerspective;
            renderObject->m_depthTestEnabled = FP::getData<bool>(0x005ac978);
            renderObject->m_depthMaskEnabled = depthMaskEnabled == false;
            renderObject->m_field0x29 = (bool)param_13;
            renderObject->m_field0x10 = iVar3;
            renderObject->m_polygonOffset = poligonOffset;

            const int typeIndex = FP::getData<int>(0x005bb030);
            renderObject->m_type = (&FP::getData<unsigned int>(0x005bb038))[typeIndex];

            if (_this->m_modelMatrix) {
                renderObject->m_modelMatrix = (glm::mat4*)_this->m_modelMatrix;
            }
            if (_this->m_projectionMatrix) {
                renderObject->m_projectionMatrix = _this->m_projectionMatrix;
            }

            if ((_Src < _this->m_field0x76030[_this->m_renderBlockIndex]) || (_this->m_field0x76030[_this->m_renderBlockIndex] + 0x40000 <= _Src)) {
                _Size = iVar3 * 24;
                _Dst = (float*)FP::callThisFarA<int>(0x00425bb0, _this, _Size);
                memcpy(_Dst, _Src, _Size);
                renderObject->m_meshData = _Dst;
            }
            else {
                renderObject->m_meshData = (float*)_Src;
            }

            if (!_this->m_skipGlSetup) {
                if (_this->m_renderObjectListSize < 2048) {
                    if (_this->m_renderObjectList[_this->m_renderObjectListSize] != nullptr) {
                        _this->m_renderObjectList[_this->m_renderObjectListSize] = renderObject;
                    }
                    ++_this->m_renderObjectListSize;
                }
                ++_this->m_renderBlockSize[_this->m_renderBlockIndex];
            }
            else {
                if (param_6 + 1U < 4) {
                    FP::callFastFarA(0x00426f10, param_6 + 1U); 
                }
                FP::callThisFarA(0x00426fb0, renderObject, _this->m_skipGlSetup, 0); //render
            }
        }
    }

    ptr = textureDataPtr;
    if (((textureDataPtr != (int*)0x0) && ((int)textureDataPtr - 0x10001U < 0xeffeffff)) && ((*textureDataPtr != 0 && (iVar3 = *textureDataPtr + -1, *textureDataPtr = iVar3, iVar3 == 0)))) {
        FP::callFastFarA(0x004243e0, (int)textureDataPtr);
        FP::callFastFarA(0x00433380, (byte*)ptr);
    }
    //ExceptionList = local_10;
    return;
}