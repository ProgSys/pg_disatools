#pragma once
#include <glm/glm.hpp>
#include <ranges>
#include <array>
#include <Resources/PG_DisaModel.h>

/**
 * @brief Vertex containing only position and packed color.
 * Total size: 0x10 (16 bytes)
 */
struct VertexSolidColor {
    uint32_t  color;   // 0x00 - RGBA packed color
    glm::vec3 pos;     // 0x04 - Position (x, y, z)
}; 
/**
 * @brief Basic textured vertex with color.
 * Total size: 0x18 (24 bytes)
 */
struct VertexBasic {
    glm::vec2 uv;      // 0x00 - Texture coordinates
    uint32_t  color;   // 0x08 - RGBA packed color
    glm::vec3 pos;     // 0x0C - Position (x, y, z)
}; 
/**
 * @brief Vertex with normal mapping support (tangent space).
 * Includes tangent and bitangent vectors.
 * Total size: 0x30 (48 bytes)
 */
struct VertexWithNormal { 
    glm::vec2 uv;        // 0x00 - Texture coordinates
    uint32_t  color;     // 0x08 - RGBA packed color
    glm::vec3 pos;       // 0x0C - Position
    glm::vec3 tangent;   // 0x18 - Tangent vector
    glm::vec3 bitangent; // 0x24 - Bitangent vector 
}; // 48 bytes

/**
 * @brief Texture sampling parameters.
 * Likely maps directly to OpenGL sampler state.
 * Total size: 0x04
 */
struct DisaRenderObjectTexParameter {
    uint16_t m_field0x0; //0x00
    uint8_t m_magFilter; //0x02 GL_NEAREST or GL_LINEAR (could be bool)
    uint8_t m_warpMode;  //0x03 GL_CLAMP or GL_REPEAT == 2
};
/**
 * @brief Bitfield-based render state configuration.
 * Packed into a single 16-bit value.
 */
struct DisaRenderObjectSettings {
    uint16_t stencilRef : 8;  // 0x00 [0–7]   - Stencil reference value
    uint16_t stencilMode : 3; // 0x00 [8–10]  - Stencil operation mode
    uint16_t projIndex : 1;   // 0x00 [11]    - Projection selector
    uint16_t blendMode : 4;   // 0x00 [12–15] - Blending mode
};

/**
* @brief Represents a single renderable object.
* Managed by DisaRenderObjectManager.
*/
struct DisaRenderObject {
    uint32_t m_type;            // 0x00
    float m_depth;              // 0x04 - sortorder or type? 
    float m_field0x8;           // 0x08
    DisaRenderObjectSettings m_renderSettingsBitMask; // 0x0c - Packed render state
    uint16_t m_uniformSize;     // 0x0e
    int32_t m_field0x10;        // 0x10 - vertexCount
    int32_t* m_field0x14;       // 0x14 - texture
    DisaRenderObjectTexParameter m_textureInfo; // 0x18 - texture
    float* m_meshData;          // 0x1c - data to a list of VertexSolidColor, VertexBasic or VertexWithNormal
    glm::mat4* m_modelMatrix;   // 0x20 - modelMatrix
    glm::mat4* m_projectionMatrix; // 0x24 - optional custom projection matrix (is mostly null)
    bool m_isPerspective;       // 0x28 - True: 3D, False: UI (orthographic)
    bool m_field0x29;           // 0x29
    bool m_depthTestEnabled;    // 0x2a - Depth test enable
    bool m_depthMaskEnabled;    // 0x2b - Depth write enable
    float m_polygonOffset;      // 0x2c - Depth bias / polygon offset
    int32_t* m_field0x30;       // 0x30 
    glm::mat4 m_field0x34;      // 0x34
};

/**
 * @brief Central manager for all renderable objects.
 * Handles batching, sorting, and submission.
 *
 * Created at: 0x00507FC1
 * Size: 0x76120
 * Init function: FUN_00425970
*/
struct DisaRenderObjectManager {
    glm::vec3* m_field0x0;      // 0x00
    glm::vec3* m_center;        // 0x04 - camera center
    glm::vec3* m_field0x8;      // 0x08
    glm::vec3* m_field0xc;      // 0x0c
    glm::vec3 m_lookDir;        // 0x10 - camera forward direction
    uint32_t m_field0x14;       // 0x14
    uint32_t m_prevRenderObjectListSize; // 0x18 - stores the last m_renderObjectListSize
    std::array<DisaRenderObject, 4096> m_renderObjects; //0x24 - Two blocks each max 2048 items, split at middle
    uint32_t m_renderObjectListSize; // 0x74024
    std::array<DisaRenderObject*, 2048> m_renderObjectList; // 0x74028 - pointers to objects in renderObjects
    std::array<int, 2> m_renderBlockSize; // 0x76028 - number of items inside each block
    std::array<float*, 2> m_field0x76030; // 0x76030 - mesh memory cash begin pointer for each block
    std::array<int, 2> m_field0x76038; // 0x76038 
    uint32_t m_renderBlockIndex; // 0x76040 - 0 or 1
    float m_basePolygonOffset;  // 0x76044 - additional offset added to each object when submitted
    float m_field0x76048;       // 0x76048
    bool m_skipGlSetup;         // 0x7604c - When bulk rendered its is set to true as openGL setup is done beforehand
    bool m_field0x7604d;        // 0x7604d
    bool m_field0x7604e;        // 0x7604e
    bool m_field0x7604f;        // 0x7604f
    glm::mat4* m_modelMatrix;   // 0x76050
    glm::mat4* m_projectionMatrix; // 0x76054
    glm::mat4* m_perspectiveMatrix; // 0x76058 - used to render the world
    glm::mat4 m_field0x7605c;   // 0x7605c - viewMatrix?
    glm::mat4 m_field0x7609c;   // 0x7609c
    glm::mat4 m_field0x760dc;   // 0x760dc
    uint32_t m_field0x760e0;    // 0x760e0

    /**
    * @brief Singleton accessor.
    * @return Instance located at 0x005BB02C
    */
    static DisaRenderObjectManager* get();
    /**
    * @brief Returns the active render object list as a range.
    */
    inline auto getRenderObjectList() {
        return std::ranges::subrange(m_renderObjectList.begin(), std::next(m_renderObjectList.begin(), m_renderObjectListSize));
    }
    /**
    * @brief Will add a new render object that uses VertexBasic as its mesh type
    * Original function: FUN_00425df0
    * @param customData If set to true then the mesh data will directly be send to renderer wihtout checking if it needs to be moved into game buffer 
    */
    void submitVertexBasic(float depth, int* textureDataPtr, void* geometryData, int triangleCount, short sizeUniforms, int blendMode, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled,
        float poligonOffset, int stencilMode, unsigned char stencilRef, bool param_13, bool customData);

    void submitCustomModel(float depth, int modelIndex, int blendMode, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled, float poligonOffset, int stencilMode, unsigned char stencilRef, bool param_13);
};

/**
* @brief Compares two DisaRenderObjects using m_depth and m_field0x8. If both are equal or nan then the pointer is used as fallback.
*/
bool FUN_004280a0_sortFunc(const DisaRenderObject* param_1, const DisaRenderObject* param_2);

//all replaced by std::sort
void __fastcall FUN_00427da0_insertionSort(DisaRenderObject** param_1, DisaRenderObject** param_2);
void __fastcall FUN_00427ff0_pushHeap(DisaRenderObject** begin, int index, int param_3, DisaRenderObject** insertValue);
void __fastcall FUN_00427c30_makeHeap(DisaRenderObject** begin, DisaRenderObject** end);

/**
* @brief Render objects
*/
void _stdcall FUN_00426770(int param_1, unsigned int param_2, bool depthTestEnabled);
/**
* @brief Submit render data to render queue
*/
void __stdcall FUN_00425df0_SubmitVertexBasic(float param_1, int* textureDataPtr, void* geometryData, int triangleCount, short sizeUniforms, int param_6, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled,
    float poligonOffset, int param_11, bool param_12, bool param_13);