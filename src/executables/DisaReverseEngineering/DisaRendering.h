#pragma once
#include <glm/glm.hpp>
#include <ranges>
#include <array>

struct DisaRenderObject {
    uint32_t m_type; //0x0
    float m_depth; //0x4 or type 
    float m_field0x8; //0x8
    uint32_t m_stencilSetting; //0xc
    int32_t m_field0x10; //0x10 vertexCount
    int32_t* m_field0x14; //0x14 texture
    int32_t m_field0x18; //0x18 texture
    float* m_meshData; //0x1c
    glm::mat4* m_modelMatrix; //0x20 modelMatrix?
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
    uint32_t m_prevRenderObjectListSize; //0x18 stores the last m_renderObjectListSize
    std::array<DisaRenderObject, 4096> m_renderObjects; //0x24 Two blocks each max 2048 items, split at middle
    uint32_t m_renderObjectListSize; //0x74024
    std::array<DisaRenderObject*, 2048> m_renderObjectList; //0x74028 pointers to objects in renderObjects
    std::array<int, 2> m_renderBlockSize; //0x76028 number of items inside each block
    std::array<float*, 2> m_field0x76030; //0x76030 mesh memory cash begin pointer for each block
    std::array<int, 2> m_field0x76038; //0x76038 
    uint32_t m_renderBlockIndex; //0x76040 0 or 1
    float m_depthOffset; //0x76044
    float m_field0x76048; //0x76048
    bool m_skipGlSetup; //0x7604c
    bool m_field0x7604d; //0x7604d
    bool m_field0x7604e; //0x7604e
    bool m_field0x7604f; //0x7604f
    glm::mat4* m_modelMatrix; //0x76050
    glm::mat4* m_projectionMatrix; //0x76054
    glm::mat4* m_perspectiveMatrix; //0x76058 used to render the world
    glm::mat4 m_field0x7605c; //0x7605c viewMatrix?
    glm::mat4 m_field0x7609c; //0x7609c
    glm::mat4 m_field0x760dc; //0x760dc
    uint32_t m_field0x760e0; //0x760e0

    inline auto getRenderObjectList() {
        return std::ranges::subrange(m_renderObjectList.begin(), std::next(m_renderObjectList.begin(), m_renderObjectListSize));
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

void __stdcall FUN_00425df0_SubmitVertexBasic(float param_1, int* textureDataPtr, void* geometryData, int triangleCount, short sizeUniforms, int param_6, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled,
    float poligonOffset, int param_11, bool param_12, bool param_13);