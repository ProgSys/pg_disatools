#include "DisaRendering.h"
#include <algorithm>
#include <bit>
#include <glm/gtc/matrix_transform.hpp>

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
    
    /*
    int iVar1;
    float fVar2;
    float fVar3;
    glm::vec3* pVVar4;
    glm::vec3* pVVar5;
    int iVar6;

    iVar1 = index + -1;
    if (start < index) {
        do {
            iVar6 = iVar1 / 2;
            pVVar4 = begin[iVar6];
            pVVar5 = *insertValue;
            fVar2 = pVVar4->y;
            fVar3 = pVVar5->y;
            if ((glm::isnan(fVar2) || glm::isnan(fVar3)) == (fVar2 == fVar3)) {
            LAB_0042806f:
                if (fVar3 <= fVar2) break;
            }
            else {
                fVar2 = pVVar4->z;
                fVar3 = pVVar5->z;
                if ((glm::isnan(fVar2) || glm::isnan(fVar3)) == (fVar2 == fVar3)) goto LAB_0042806f;
                if (pVVar5 <= pVVar4) break;
            }
            iVar1 = iVar6 + -1;
            begin[index] = pVVar4;
            index = iVar6;
        } while (start < iVar6);
    }
    begin[index] = *insertValue;
    */

}

void __fastcall FUN_00427c30_makeHeap(DisaRenderObject** begin, DisaRenderObject** end) {
    std::make_heap(begin, end, &FUN_004280a0_sortFunc); //create max-heap
    /*
    glm::vec3* pVVar1;
    glm::vec3* pVVar2;
    int iVar3;
    int iVar4;
    int iVar5;
    int iVar6;
    glm::vec3* local_1c;
    int local_18;
    int local_14;
    int local_10;
    float local_c;
    float local_8;

    iVar6 = (int)end - (int)begin >> 2;
    local_14 = iVar6 - ((int)end - (int)begin >> 0x1f) >> 1;
   if (0 < local_14) {
        local_10 = local_14 * 2 + 2;
        local_18 = iVar6;
        do {
            local_1c = begin[local_14 + -1];
            local_14 = local_14 + -1;
            local_10 = local_10 + -2;
            iVar3 = local_10;
            iVar5 = local_14;
            iVar4 = local_14;
            if (local_10 < iVar6) {
                do {
                    pVVar1 = begin[iVar3];
                    pVVar2 = begin[iVar3 + -1];
                    iVar4 = iVar3;
                    if (FUN_004280a0(pVVar1, pVVar2)) {
                        iVar4 = iVar3 - 1;
                    }
    
                    begin[iVar5] = begin[iVar4];
                    iVar3 = iVar4 * 2 + 2;
                    iVar5 = iVar4;
                    iVar6 = local_18;
                } while (iVar3 < local_18);
            }
            if (iVar3 == iVar6) {
                begin[iVar4] = begin[iVar6 + -1];
                iVar4 = iVar6 + -1;
            }
            FUN_00427ff0(begin, iVar4, local_14, &local_1c);
        } while (0 < local_14);
    }
   */

}

using byte = std::uint8_t;

// CONCAT44 is a concatenation operation for two 32-bit integers
static inline uint64_t CONCAT44(uint32_t x, uint32_t y) {
    return (static_cast<uint64_t>(x) << 32) | static_cast<uint64_t>(y);
}

// CONCAT31 is a concatenation operation for two 32-bit integers
static inline uint32_t CONCAT31(uint32_t x, uint32_t y) {
    return (x << 16) | (y & 0xFF);
}


#include <FunctionPatcher.h>
#include <gl/glew.h>
#include <GL/glu.h>
#include <iostream>
#include <array>

static inline bool isVisible(int param_1, unsigned int visibleBitMask, uint8_t type, int32_t* field0x14, float depth, int32_t field0x10, bool field0x28) {
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
    if (!(visibleBitMask & 0x1) && (field0x28 != 0))
        return false;
    if (!(visibleBitMask & 0x2) && (field0x28 == 0) && (param_1 != 2 || depth != 100.0))
        return false;
    if (visibleBitMask & 0x20 && depth == 100.0)
        return false;
    if (-1 >= (int)visibleBitMask && depth == 40005.0)
        return false;
    return true;
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

    int* renderWorld = FP::getData<int*>(0x005bb02c); //piVar9 DisaMemeoryManager?
    
    const unsigned int someObjPtr = FP::getData<unsigned int>(0x005bae70);
    if ( !*(bool*)(someObjPtr + 0x19) || *(bool*)(someObjPtr + 0x59) || *(bool*)(someObjPtr + 0x60))
        return;

    int sizeCurr = renderWorld[0x1d009];
    int& sizeOld = renderWorld[8];
    DisaRenderObject** begin = (DisaRenderObject**)(renderWorld + 0x1d00a);
    DisaRenderObject** end = begin + sizeCurr;
    assert(begin <= end);
    const auto range = std::ranges::subrange(begin, end);

    //sort if number of item has changed
    if (sizeCurr != sizeOld) {
        std::ranges::sort(range, &FUN_004280a0_sortFunc);
        //original function of a custom build sorting algorithm, however std sort does not only the same is even faster
        //unsigned int centerIndex = (FP::getData<unsigned int>(0x005bb02c) + FP::getData<unsigned int*>(0x005bb02c)[0x1d009] + 0x1d00a) - (int)(FP::getData<unsigned int>(0x005bb02c) + 0x1d00a) >> 2;
        //FUN_004275f0_sort(begin, end, centerIndex);
        sizeOld = sizeCurr;
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
    FP::callStdFarA(0x00421a30, 10, 17); //glUseAndInitProgram; will call glUseProgram and then setup needed uniforms
    
    struct UnknownObject3 {
        std::array<float, 14> field0x0;
        glm::mat4 projectionMatrix; //0x38
    };

    struct UnknownObject {
        uint32_t field0x0; //0x0
        UnknownObject3** field0x4[2];//0x4 - 0x8
        uint32_t field0x10; //0x12
        unsigned short field0x14; //0x14
        unsigned short field0x16; //0x16
    };

    UnknownObject* unknownObject = FP::getData<UnknownObject*>(0x005cbbc0);
    unknownObject->field0x14 = 257;

    for (DisaRenderObject* object : range) {

        if (!isVisible(param_1, visibleBitMask, object->m_type, object->m_field0x14, object->m_depth, object->m_field0x10, object->m_field0x28))
            continue;

        bool stencilBitSet = object->m_stencilSetting >> 0xb & 1;
        FP::callStdFarA(0x00432f50, stencilBitSet); // somekind of matrix setup

        //projection matrix
        glMatrixMode(GL_PROJECTION); 
        glm::mat4* projectionMat = object->m_projectionMatrix; // pfVar8
        if (projectionMat == nullptr) {
            if (object->m_field0x28 == false) {
                if (param_1 == 1) continue;
                projectionMat = &(*unknownObject->field0x4[stencilBitSet])->projectionMatrix;
            }
            else if (param_1 == 1) {
                glLoadIdentity();
                projectionMat = &FP::getData<glm::mat4>(0x008191c0);

                glm::mat4* currMat = (glm::mat4*)(renderWorld + 0x1d837);
                *currMat = *projectionMat;
                FP::callThisFarA(0x0042d3e0, currMat, 0x40d9999a, 256.0, 256.0, 320.0, 11600.0);
            }
            else {
                projectionMat = (glm::mat4*)renderWorld[0x1d816];
            }
        }
        glLoadMatrixf((GLfloat*)projectionMat);

        //modelview matrix
        glMatrixMode(GL_MODELVIEW);  
        if (object->m_field0x28 == false) {
            glLoadIdentity();
        }
        else {
            struct MemoryManagerTemp {
                glm::vec3* m_field0x0;
                glm::vec3* m_field0x4;
                glm::vec3* m_field0x8;
                glm::vec3* m_field0xc;
                glm::vec3 m_field0x10;
            };
            MemoryManagerTemp* temp = (MemoryManagerTemp*)renderWorld;

            if (param_1 == 1 && temp->m_field0x0 && temp->m_field0x4 && temp->m_field0x8) {
                glLoadIdentity();
                assert(false);

                //why is it not the same as DisaCameraInfo?
                const glm::vec3 center = *temp->m_field0x4;
                const glm::vec3 dir = temp->m_field0x10 * -2000.0f;
                const glm::vec3 eye = center + dir;

                // pfVar5 = FP::callThisFarA<float*>(0x004281b0, &local_44, local_64, 2000.0); //local_64 = local_44 * 2000.0 -> address of local_64
                // pfVar5 = FP::callThisFarA<float*>(0x00428170, &local_30, local_58, &local_44); //local_58 = local_30 + local_44 -> address of local_58

                gluLookAt((double)eye.x, (double)eye.y, (double)eye.z, (double)center.x, (double)center.y, (double)center.z, 0.0, -1.0, 0.0);
                glGetFloatv(GL_MODELVIEW_MATRIX, (float*)(renderWorld + 0x1d827));
                glLoadIdentity();
                glLoadMatrixf((float*)(renderWorld + 0x1d827));
                glMatrixMode(GL_PROJECTION);
                glGetFloatv(GL_PROJECTION_MATRIX, (float*)(renderWorld + 0x1d837));
                glMatrixMode(GL_MODELVIEW);
            }
            else {
                glLoadMatrixf((float*)(renderWorld + 0x1d817));
            }
        }

        glPushMatrix();
        if (object->m_field0x20 != nullptr) {
            glMultMatrixf((GLfloat*)object->m_field0x20);
        }

        //update some object matrix, maybe modelMatrix
        if (object->m_field0x28 == false || object->m_field0x20 == nullptr) {
            object->m_field0x34 = FP::getData<glm::mat4>(0x005b2670);
        }
        else {
            glm::mat4 temp = *(glm::mat4*)(renderWorld + 0x1d817);
            object->m_field0x34 = *object->m_field0x20 * temp; //  FP::callFastFarA(0x0042ced0, &object->m_field0x34, object->m_field0x20, &temp);
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
                FP::callCdeclFarA(0x00420e50, 0); //calls glStencilOp(0x1e00,0x1e00,*(undefined4 *)(&DAT_00596e0c + param_1 * 4));
                break;
            case 1:
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_ALWAYS, stencilRefValue, 0xff);
                FP::callCdeclFarA(0x00420e50, 2); //glStencilOp
                break;
            case 2:
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_NOTEQUAL, stencilRefValue, 0xff);
                FP::callCdeclFarA(0x00420e50, 0); //glStencilOp
                break;
            case 3:
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_NOTEQUAL, stencilRefValue, 0xff);
                FP::callCdeclFarA(0x00420e50, 2); //glStencilOp
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
    unknownObject->field0x14 = 0;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void _stdcall FUN_00426770_Dummy(int param_1, unsigned int param_2, unsigned char depthTestEnabled) {
    int i = 0;
}