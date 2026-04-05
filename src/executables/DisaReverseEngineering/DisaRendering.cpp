#include "DisaRendering.h"
#include <algorithm>
#include <bit>

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

void _stdcall FUN_00426770(int param_1, unsigned int param_2, bool depthTestEnabled) {
    static bool openGLInit = false;
    if (!openGLInit) {
        glewExperimental = GL_TRUE; // recommended
        if (glewInit() != GLEW_OK) {
            auto i = 0;
        }
        openGLInit = true;
    }
    byte bVar1;
    bool bVar4;
    float* pfVar5;
    float* pfVar8;
    float* pfVar10;
    char* pcVar11;
    unsigned long long uVar12;
    float local_64[3];
    float local_58[3];
    unsigned int local_4c;
    unsigned int local_48;
    float local_44;
    float local_40;
    float local_3c;
    int local_38;
    float local_30;
    float local_2c;
    float local_28;
    float* local_20;
    byte local_11;
    float depth = 0.f;

    int* renderWorld = FP::getData<int*>(0x005bb02c); //piVar9
    float polygonOffsetValue = 1.192093e-07f;

    const unsigned int someObjPtr = FP::getData<unsigned int>(0x005bae70);
    if ( !*(bool*)(someObjPtr + 0x19) || *(bool*)(someObjPtr + 0x59) || *(bool*)(someObjPtr + 0x60))
        return;

    unsigned int sizeCurr = FP::getData<unsigned int*>(0x005bb02c)[0x1d009];
    unsigned int& sizeOld = FP::getData<unsigned int*>(0x005bb02c)[8];
    DisaRenderObject** begin = (DisaRenderObject**)(renderWorld + 0x1d00a);
    DisaRenderObject** end = begin + sizeCurr;
    assert(begin <= end);

    if (sizeCurr != sizeOld) {
        std::sort(begin, end, &FUN_004280a0_sortFunc);
        //original function of a custom build sorting algorithm, however std sort does not only the same is even faster
        //unsigned int centerIndex = (FP::getData<unsigned int>(0x005bb02c) + FP::getData<unsigned int*>(0x005bb02c)[0x1d009] + 0x1d00a) - (int)(FP::getData<unsigned int>(0x005bb02c) + 0x1d00a) >> 2;
        //FUN_004275f0_sort(begin, end, centerIndex);
        sizeOld = sizeCurr;
    }

    FP::getData<int>(0x005bb020) = 0;
    FP::getData<int>(0x005bb024) = 0;
    FP::getData<int>(0x005ac928) = -1;
    FP::getData<int>(0x005ac92c) = -1;
    FP::getData<int>(0x005ac920) = -1;

    int stencilModeValue = 0;
    int stencilRefValue = -1;

    if (depthTestEnabled == 0) {
        glDisable(0xb71);
    }
    else {
        glEnable(0xb71);
    }
    bool depthMaskEnabled = depthTestEnabled;
    glDepthMask(depthTestEnabled);

    local_20 = (float*)0x34000000;
    glEnable(0xbc0);
    glAlphaFunc(0x204, 0.01f);
    FP::callStdFarA(0x00421a30, 10, 17); //glUseAndInitProgram; will call glUseProgram and then setup needed uniforms
    *(unsigned short*)(FP::getData<unsigned int>(0x005cbbc0) + 0x10) = 257;



    for (DisaRenderObject* object : std::ranges::subrange(begin, end)) {
        const float depth = object->m_depth; //or type

        bVar4 = object->m_field0x14 && object->m_field0x14[6] == 3;
        if (bool(param_2 & 4) != bVar4) continue;

        if ((param_1 == 1) && ((object->m_type & 1) == 0))
            continue;
        if ((param_2 & 0x40) && depth == 100.0f)
            goto pathYes; //yes
        if ((param_2 & 0x100) && depth == 40005.0f)
            goto pathYes; //yes  
        if ((param_2 & 0x400) && depth == 45000.0f)
            goto pathYes; //yes  
        if ((param_2 & 0x800) && depth == 45000.0f)
            continue; //no  
        if ((param_2 & 0x1000) && depth == 41000.0f && (object->m_type & 0x10))
            goto pathYes; //yes  
        if ((param_2 & 0x2000) && depth == 41000.0f && (object->m_type & 0x10))
            continue; //no  
        if ((param_2 & 0x20) && depth == 50000.0f)
            goto pathYes; //yes  
        if ((param_2 & 0x2000) && depth == 50000.0f)
            continue; //no 
        if (!(param_2 & 0x8) && (object->m_field0x10 != 0))
            continue; // no
        if (!(param_2 & 0x10) && (object->m_field0x10 == 0))
            continue; // no
        if (!(param_2 & 0x1) && (object->m_field0x28 != 0))
            continue; // no


        if (!(param_2 & 0x2)) {
            bool local_xx = object->m_field0x28;
            if (local_xx == 0 && param_1 == 2 && depth == 100.0) {
                bVar4 = true;
            }
            else {
                bVar4 = false;
            }
            if ((local_xx == 0) && (!bVar4)) continue;//continue; 
        }

        if (param_2 & 0x20 && depth == 100.0)
            continue; //no 
        if (-1 >= (int)param_2 && depth == 40005.0)
            continue; //no 
    pathYes:
        FP::callStdFarA(0x00432f50, object->m_field0xc >> 0xb & 1); // somekind of matrix setup

        glMatrixMode(0x1701);
        glm::mat4* targetMat = object->m_field0x24; // pfVar8
        if (object->m_field0x24 == nullptr) {
            if (object->m_field0x28 == false) {
                if (param_1 == 1) continue;
                targetMat = (glm::mat4*)(*(int*)(*(int*)(FP::getData<unsigned int>(0x005cbbc0) + 4) + (object->m_field0xc >> 0xb & 1) * 4) + 0x38);
            }
            else if (param_1 == 1) {
                glLoadIdentity();
                targetMat = &FP::getData<glm::mat4>(0x008191c0);

                glm::mat4* currMat = (glm::mat4*)(renderWorld + 0x1d837);
                *currMat = *targetMat;
                FP::callThisFarA(0x0042d3e0, currMat, 0x40d9999a, 256.0, 256.0, 320.0, 11600.0);
            }
            else {
                targetMat = (glm::mat4*)renderWorld[0x1d816];
            }
        }
        glLoadMatrixf((GLfloat*)targetMat);
        glMatrixMode(0x1700);
        if (object->m_field0x28 == false) {
            glLoadIdentity();
        }
        else {
            if (((*renderWorld == 0) || (renderWorld[1] == 0)) || (renderWorld[2] == 0)) {
                bVar4 = false;
            }
            else {
                bVar4 = true;
            }
            if ((param_1 == 1) && (bVar4)) {
                glLoadIdentity();
                local_44 = (float)(renderWorld[4] ^ 0x80000000);
                local_40 = (float)(renderWorld[5] ^ 0x80000000);
                local_3c = (float)(renderWorld[6] ^ 0x80000000);
                // is same as "glm::vec3 local_64 = local_44 * 2000.0", returns first float (x) address of local_64
                // means pfVar5 is same as &local_64 afterwards
                pfVar5 = FP::callThisFarA<float*>(0x004281b0, &local_44, local_64, 2000.0);

                pfVar8 = (float*)renderWorld[1];
                local_44 = *pfVar5;
                local_40 = pfVar5[1];
                local_3c = pfVar5[2];
                local_30 = *pfVar8;
                local_2c = pfVar8[1];
                local_28 = pfVar8[2];
                // is same as "glm::vec3 local_58 = local_30 + local_44" (component addition), returns first float (x) address of local_58
                // means pfVar5 is same as &local_58 afterwards
                pfVar5 = FP::callThisFarA<float*>(0x00428170, &local_30, local_58, &local_44);
                local_30 = *pfVar5;
                local_2c = pfVar5[1];
                local_28 = pfVar5[2];
                gluLookAt((double)local_30, (double)local_2c, (double)local_28, (double)*pfVar8, (double)pfVar8[1], (double)pfVar8[2], 0, 0xbff0000000000000, 0);
                glGetFloatv(0xba6, (float*)(renderWorld + 0x1d827));
                glLoadIdentity();
                glLoadMatrixf((float*)(renderWorld + 0x1d827));
                glMatrixMode(0x1701);
                glGetFloatv(0xba7, (float*)(renderWorld + 0x1d837));
                glMatrixMode(0x1700);
            }
            else {
                glLoadMatrixf((float*)(renderWorld + 0x1d817));
            }
        }
        glPushMatrix();
        if (object->m_field0x20 != nullptr) {
            glMultMatrixf((GLfloat*)object->m_field0x20);
        }

        if (object->m_field0x28 == false || object->m_field0x20 == nullptr) {
            object->m_field0x34 = FP::getData<glm::mat4>(0x005b2670);
        }
        else {
            glm::mat4 temp = *(glm::mat4*)(renderWorld + 0x1d817);
            object->m_field0x34 = *object->m_field0x20 * temp; //  FP::callFastFarA(0x0042ced0, &object->m_field0x34, object->m_field0x20, &temp);
        }

        const int stencilRefTemp = int(object->m_field0xc & 0xff);
        const int stencilModeTemp = (int)(object->m_field0xc << 0x15) >> 0x1d;
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

        if (depthTestEnabled != object->m_depthTestEnabled) {
            depthTestEnabled = object->m_depthTestEnabled;
            if (depthTestEnabled) {
                glEnable(GL_DEPTH_TEST);
            }
            else {
                glDisable(GL_DEPTH_TEST);
            }
        }

        if (depthMaskEnabled != object->m_depthMaskEnabled) {
            depthMaskEnabled = object->m_depthMaskEnabled;
            glDepthMask(depthMaskEnabled);
        }

        if (!glm::isnan(object->m_polygonOffset) && object->m_polygonOffset != polygonOffsetValue) {
            polygonOffsetValue = object->m_polygonOffset;
            const float polygonOffsetScale = FP::getData<float>(0x005ac924);
            glPolygonOffset(0.0, polygonOffsetScale * polygonOffsetValue);
        }

        FP::callThisFarA(0x00426fb0, object, '\0', param_1); //render object
        glPopMatrix();

    }

    /*
    if ((0 < piVar9[piVar9[0x1d810] + 0x1d80a]) && (ppbVar7 = (byte**)(piVar9 + 0x1d00a), local_18 = ppbVar7, ppbVar7 != (byte**)(piVar9 + piVar9[0x1d009] + 0x1d00a))) {
        local_4c = param_2 & 4;

        do {
            pbVar2 = *ppbVar7;
            depth = *(float*)(pbVar2 + 4);
            bVar4 = (*(int*)(pbVar2 + 0x14) != 0) && (*(int*)(*(int*)(pbVar2 + 0x14) + 0x18) == 3);
            if ((local_4c != 0) != bVar4) goto LAB_00426eba;


            if ((param_2 & 0x40) && depth == 100.0f) 
                goto LAB_00426a74; //yes
            if ((param_2 & 0x100) && depth == 40005.0f)
                goto LAB_00426a74; //yes  
            if ((param_2 & 0x400) && depth == 45000.0f)
                goto LAB_00426a74; //yes  
            if ((param_2 & 0x800) && depth == 45000.0f)
                goto LAB_00426eba; //no  
            if ((param_2 & 0x1000) && depth == 41000.0f && (*pbVar2 & 0x10))
                goto LAB_00426a74; //yes  
            if ((param_2 & 0x2000) && depth == 41000.0f && (*pbVar2 & 0x10))
               goto LAB_00426eba; //no  
            if ((param_2 & 0x20) && depth == 50000.0f)
                goto LAB_00426a74; //yes  
            if ((param_2 & 0x2000) && depth == 50000.0f) 
                goto LAB_00426eba; //no 
            if (!(param_2 & 0x8) && (*(int*)(pbVar2 + 0x10) != 0)) 
                goto LAB_00426eba; // no
            if (!(param_2 & 0x10) && (*(int*)(pbVar2 + 0x10) == 0)) 
                goto LAB_00426eba; // no
            if (!(param_2 & 0x1) && (pbVar2[0x28] != 0)) 
                goto LAB_00426eba; // no
            

            if (!(param_2 & 0x2)) {
                local_11 = pbVar2[0x28];
                if (local_11 == 0 && param_1 == 2 && depth == 100.0) {
                    bVar4 = true;
                }
                else {
                    bVar4 = false;
                }
                if ((local_11 == 0) && (!bVar4)) goto LAB_00426eba;//continue; 
            }

            if (param_2 & 0x20 && depth == 100.0) 
                goto LAB_00426eba; //no 
            if (-1 >= (int)param_2 && depth == 40005.0)
                goto LAB_00426eba; //no 
 
        LAB_00426a74:
            if ((param_1 == 1) && ((*pbVar2 & 1) == 0)) goto LAB_00426eba; //continue; 
            local_18 = ppbVar7;
            FP::callStdFarA(0x00432f50, (*(unsigned int*)(pbVar2 + 0xc) >> 0xb & 1)); // somekind of matrix setup
            glMatrixMode(0x1701);
            pbVar2 = *ppbVar7;
            pfVar8 = *(float**)(pbVar2 + 0x24);
            if (*(float**)(pbVar2 + 0x24) == (float*)0x0) {
                if (pbVar2[0x28] == 0) {
                    if (param_1 == 1) goto LAB_00426eba;
                    pfVar8 = (float*)(*(int*)(*(int*)(FP::getData<unsigned int>(0x005cbbc0) + 4) + (*(unsigned int*)(pbVar2 + 0xc) >> 0xb & 1) * 4) + 0x38);
                }
                else if (param_1 == 1) {
                    glLoadIdentity();
                    pfVar10 = (float*)(local_24 + 0x1d837);
                    local_10 = pfVar10;
                    pfVar8 = (float*)&FP::getData<unsigned int>(0x008191c0);
                    pfVar5 = local_e4;
                    //memcpy 4x4 matrix 
                    for (iVar6 = 0x10; iVar6 != 0; iVar6 = iVar6 + -1) {
                        *pfVar5 = *pfVar8;
                        pfVar8 = pfVar8 + 1;
                        pfVar5 = pfVar5 + 1;
                    }
                    pfVar8 = local_e4;
                    pfVar5 = pfVar10;
                    // memcpy 4x4 matrix
                    for (iVar6 = 0x10; iVar6 != 0; iVar6 = iVar6 + -1) {
                        *pfVar5 = *pfVar8;
                        pfVar8 = pfVar8 + 1;
                        pfVar5 = pfVar5 + 1;
                    }
                    FP::callThisFarA(0x0042d3e0, pfVar10, 0x40d9999a, 256.0, 256.0, 320.0, 11600.0);
                    pfVar8 = local_10;
                    ppbVar7 = local_18;
                    piVar9 = local_24;
                }
                else {
                    pfVar8 = (float*)piVar9[0x1d816];
                }
            }
            glLoadMatrixf(pfVar8);
            glMatrixMode(0x1700);
            if ((*ppbVar7)[0x28] == 0) {
                glLoadIdentity();
            }
            else {
                if (((*piVar9 == 0) || (piVar9[1] == 0)) || (piVar9[2] == 0)) {
                    bVar4 = false;
                }
                else {
                    bVar4 = true;
                }
                if ((param_1 == 1) && (bVar4)) {
                    glLoadIdentity();
                    local_44 = (float)(piVar9[4] ^ 0x80000000);
                    local_40 = (float)(piVar9[5] ^ 0x80000000);
                    local_3c = (float)(piVar9[6] ^ 0x80000000);
                    // is same as "glm::vec3 local_64 = local_44 * 2000.0", returns first float (x) address of local_64
                    // means pfVar5 is same as &local_64 afterwards
                    pfVar5 = FP::callThisFarA<float*>(0x004281b0, &local_44, local_64, 2000.0); 
                    pfVar8 = (float*)piVar9[1];
                    local_44 = *pfVar5;
                    local_40 = pfVar5[1];
                    local_3c = pfVar5[2];
                    local_30 = *pfVar8;
                    local_2c = pfVar8[1];
                    local_28 = pfVar8[2];
                    // is same as "glm::vec3 local_58 = local_30 + local_44" (component addition), returns first float (x) address of local_58
                    // means pfVar5 is same as &local_58 afterwards
                    pfVar5 = FP::callThisFarA<float*>(0x00428170, &local_30, local_58, &local_44);
                    local_30 = *pfVar5;
                    local_2c = pfVar5[1];
                    local_28 = pfVar5[2];
                    gluLookAt((double)local_30, (double)local_2c, (double)local_28, (double)*pfVar8, (double)pfVar8[1], (double)pfVar8[2], 0, 0xbff0000000000000, 0);
                    glGetFloatv(0xba6, (float*)(piVar9 + 0x1d827));
                    glLoadIdentity();
                    glLoadMatrixf((float*)(piVar9 + 0x1d827));
                    glMatrixMode(0x1701);
                    glGetFloatv(0xba7, (float*)(piVar9 + 0x1d837));
                    glMatrixMode(0x1700);
                    ppbVar7 = local_18;
                }
                else {
                    glLoadMatrixf((float*)(piVar9 + 0x1d817));
                }
            }
            glPushMatrix();
            if (*(float**)(*ppbVar7 + 0x20) != (float*)0x0) {
                glMultMatrixf(*(float**)(*ppbVar7 + 0x20));
            }
            local_10 = (float*)*ppbVar7;
            if ((*(byte*)((int)local_10 + 0x28) == 0) || (pfVar8 = *(float**)((int)local_10 + 0x20), pfVar8 == (float*)0x0)) {
                pfVar8 = (float*)&FP::getData<unsigned int>(0x005b2670);
            }
            else {
                pfVar5 = (float*)(piVar9 + 0x1d817);
                pfVar10 = local_a4;
                for (iVar6 = 0x10; iVar6 != 0; iVar6 = iVar6 + -1) {
                    *pfVar10 = *pfVar5;
                    pfVar5 = pfVar5 + 1;
                    pfVar10 = pfVar10 + 1;
                }
                FP::callFastFarA(0x0042ced0, local_a4, pfVar8, local_a4);
                pfVar8 = local_a4;
            }
            ppbVar7 = local_18;
            pfVar5 = (float*)((int)local_10 + 0x34);
            for (iVar6 = 0x10; iVar6 != 0; iVar6 = iVar6 + -1) {
                *pfVar5 = *pfVar8;
                pfVar8 = pfVar8 + 1;
                pfVar5 = pfVar5 + 1;
            }
            uVar3 = *(unsigned int*)(*local_18 + 0xc);
            iVar6 = (int)(uVar3 << 0x15) >> 0x1d;
            if ((local_34 == iVar6) && (local_1c == (uVar3 & 0xff))) goto switchD_00426d6c_caseD_4;
            local_1c = uVar3 & 0xff;
            local_34 = iVar6;
            switch (iVar6) {
            case 0:
                glDisable(GL_STENCIL_TEST);
                glStencilFunc(0x207, 0, 0);
                FP::callCdeclFarA(0x00420e50, 0); //calls glStencilOp(0x1e00,0x1e00,*(undefined4 *)(&DAT_00596e0c + param_1 * 4));
                break;
            case 1:
                glEnable(GL_STENCIL_TEST);
                func = 0x207;
                goto LAB_00426de9;
            case 2:
                glEnable(GL_STENCIL_TEST);
                uVar12 = CONCAT44(0xff, uVar3) & 0xffffffff000000ff;
                glStencilFunc(0x205, (int)uVar12, (unsigned int)(uVar12 >> 0x20));
                iVar6 = 0;
                goto LAB_00426df1;
            case 3:
                glEnable(GL_STENCIL_TEST);
                func = 0x205;
            LAB_00426de9:
                uVar12 = CONCAT44(0xff, uVar3) & 0xffffffff000000ff;
                glStencilFunc(func, (int)uVar12, (unsigned int)(uVar12 >> 0x20));
                iVar6 = 2;
            LAB_00426df1:
                FP::callCdeclFarA(0x00420e50, iVar6);
            }
        switchD_00426d6c_caseD_4:
            bVar1 = (*ppbVar7)[0x2a];
            if (depthTestEnabled != bVar1) {
                depthTestEnabled = bVar1;
                if (bVar1 == 0) {
                    glDisable(GL_DEPTH_TEST);
                }
                else {
                    glEnable(GL_DEPTH_TEST);
                }
            }
            bVar1 = (*ppbVar7)[0x2b];
            if (local_9 != bVar1) {
                //local_38 = CONCAT31(local_38 << 8, bVar1); // why is it here its never used?!
                local_9 = bVar1;
                glDepthMask(bVar1);
            }
            pfVar8 = *(float**)(*ppbVar7 + 0x2c);
            local_10 = pfVar8;
            if ((glm::isnan(std::bit_cast<float>(local_20)) || glm::isnan((std::bit_cast<float>(pfVar8))) == (std::bit_cast<float>(local_20) == std::bit_cast<float>(pfVar8)))) {
                local_10 = std::bit_cast<float*>(FP::getData<int>(0x005ac924) * std::bit_cast<float>(pfVar8));
                local_20 = pfVar8;
                float f = std::bit_cast<float>(local_10);
                glPolygonOffset(0.0, f);
            }
            FP::callThisFarA(0x00426fb0, *ppbVar7, '\0', param_1); //render object
            glPopMatrix();
            piVar9 = local_24;
        LAB_00426eba:
            ++ppbVar7;
            local_18 = ppbVar7;
        } while (ppbVar7 != (byte**)(piVar9 + piVar9[0x1d009] + 0x1d00a));
    }
    *(std::uint8_t*)(FP::getData<unsigned int>(0x005cbbc0) + 0x10) = 0;
    */
    glMatrixMode(0x1700);
    glLoadIdentity();
}


void _stdcall FUN_00426770_Dummy(int param_1, unsigned int param_2, unsigned char depthTestEnabled) {
    int i = 0;
}