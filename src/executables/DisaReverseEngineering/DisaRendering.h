#pragma once
#include <glm/glm.hpp>

struct DisaRenderObject {
    uint8_t m_type; //0x0
    uint8_t m_field0x1; //0x1
    uint8_t m_field0x2; //0x2
    uint8_t m_field0x3; //0x3
    float m_depth; //0x4 or type 
    float m_field0x8; //0x8
    uint32_t m_field0xc; //0xc
    int32_t m_field0x10; //0x10
    int32_t* m_field0x14; //0x14
    int32_t m_field0x18; //0x18
    glm::mat4* m_field0x1c; //0x1c
    glm::mat4* m_field0x20; //0x20
    glm::mat4* m_field0x24; //0x24
    bool m_field0x28; //0x28
    bool m_field0x29; //0x29
    bool m_depthTestEnabled; //0x2a
    bool m_depthMaskEnabled; //0x2b
    float m_polygonOffset; //0x2c
    int32_t m_field0x30; //0x30
    glm::mat4 m_field0x34; //0x34
};

/**
* @brief Compares most likly two vec3 values by y and z. If both are queal or nan then the pointer is used as fallback.
*/
bool FUN_004280a0_sortFunc(const DisaRenderObject* param_1, const DisaRenderObject* param_2);

//all replaced by std::sort
void __fastcall FUN_00427da0_insertionSort(DisaRenderObject** param_1, DisaRenderObject** param_2);
void __fastcall FUN_00427ff0_pushHeap(DisaRenderObject** begin, int index, int param_3, DisaRenderObject** insertValue);
void __fastcall FUN_00427c30_makeHeap(DisaRenderObject** begin, DisaRenderObject** end);

void _stdcall FUN_00426770(int param_1, unsigned int param_2, bool depthTestEnabled);
void _stdcall FUN_00426770_Dummy(int param_1, unsigned int param_2, unsigned char depthMask);