#pragma once
#include <glm/glm.hpp>
#include <ranges>
#include <array>

struct DisaRenderObject {
    uint8_t m_type; //0x0
    uint8_t m_field0x1; //0x1
    uint8_t m_field0x2; //0x2
    uint8_t m_field0x3; //0x3
    float m_depth; //0x4 or type 
    float m_field0x8; //0x8
    uint32_t m_stencilSetting; //0xc
    int32_t m_field0x10; //0x10 vertexCount
    int32_t* m_field0x14; //0x14 texture
    int32_t m_field0x18; //0x18 texture
    float* m_meshData; //0x1c
    glm::mat4* m_field0x20; //0x20 modelMatrix?
    glm::mat4* m_projectionMatrix; //0x24 optional custom projection matrix (is mostly null)
    bool m_isPerspective; //0x28 world object or Ui (orthogonal matrix)
    bool m_field0x29; //0x29
    bool m_depthTestEnabled; //0x2a
    bool m_depthMaskEnabled; //0x2b
    float m_polygonOffset; //0x2c
    int32_t* m_field0x30; //0x30 
    glm::mat4 m_field0x34; //0x34
};

/**
*
* Created at 00507fc1 with total size of 0x76120, init function is FUN_00425970.
*/
struct DisaRenderObjectManager {
    glm::vec3* m_field0x0; //0x0
    glm::vec3* m_center; //0x4 //camera center
    glm::vec3* m_field0x8; //0x8
    glm::vec3* m_field0xc; //0xc
    glm::vec3 m_lookDir; //0x10
    uint32_t m_field0x14; //0x14
    uint32_t m_prevRenderSize; //0x18 stores the last m_renderSize
    std::array<DisaRenderObject, 4096> m_renderObjects; //0x24
    uint32_t m_renderSize; //0x74024
    std::array<DisaRenderObject*, 2048> m_renderObjectList; //0x74028
    std::array<int, 6> m_field0x76028; //0x76028
    uint32_t m_field0x76040; //0x76040
    std::array<int, 5> m_field0x76044; //0x76044
    glm::mat4* m_perspectiveMatrix; //0x76058 used to render the world
    glm::mat4 m_field0x7605c; //0x7605c viewMatrix?
    glm::mat4 m_field0x7609c; //0x7609c
    glm::mat4 m_field0x760dc; //0x760dc
    uint32_t m_field0x760e0; //0x760e0

    inline auto getRenderObjectList() {
        return std::ranges::subrange(m_renderObjectList.begin(), std::next(m_renderObjectList.begin(), m_renderSize));
    }
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