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
 * @brief Texture state captured when the object was submitted, or a vertex color.
 * Which one applies depends on DisaRenderObject::vertexCount. Zero means the object is a
 * display list, and then these four bytes are the color handed to glColor4ubv.
 * Total size: 0x04
 */
union DisaRenderObjectTexParameter {
    struct {
        uint16_t paletteIndex; // 0x00 - copied off the texture at submit time
        uint8_t magFilter;     // 0x02 - GL_NEAREST or GL_LINEAR
        uint8_t wrapMode;      // 0x03 - GL_CLAMP, or GL_REPEAT when 2
    };
    uint32_t color;            // packed RGBA, display list objects only
};
/**
 * @brief Bitfield-based render state configuration.
 * Packed into a single 16-bit value. The signed fields carry -1 to keep the global default.
 */
struct DisaRenderObjectSettings {
    uint16_t stencilRef : 8;  // 0x00 [0 to 7]   - Stencil reference value
    int16_t stencilMode : 3;  // 0x00 [8 to 10]  - 0 to 3 pick a glStencilOp, anything else disables it
    uint16_t projIndex : 1;   // 0x00 [11]       - Projection selector
    int16_t blendMode : 4;    // 0x00 [12 to 15] - Index of a blend state call list
};

/**
 * @brief What an object inherits from the scope it was submitted in.
 * The game keeps a small stack of these at 0x005BB038 with the depth at 0x005BB030. Code
 * brackets a group of draws with a push that copies the parent word down, edits a bit, and
 * pops again. A fresh stack starts as castsShadow, useTexture and useLighting.
 */
struct DisaRenderFlags {
    uint32_t castsShadow : 1; // 0x01 - drawn in the shadow pass
    uint32_t isFullyLit : 1;  // 0x02 - picks the lit program that also samples the normal map
    uint32_t useTexture : 1;  // 0x04 - when clear the object draws with the flat color program
    uint32_t useLighting : 1; // 0x08 - only read while useTexture is set
    uint32_t field0x10 : 1;   // 0x10 - tested by the visibility mask together with depth 41000
    uint32_t field0x20 : 27;  // 0x20 and above, no bit up here is ever read
};

/**
* @brief A reference counted texture. The count is the first member, so the game passes these
* around as a raw pointer and keeps them alive through FUN_0041E560.
*/
struct DisaTextureSPtr {
    uint32_t m_useCount;  //0x0
    uint32_t* m_field0x4; //0x4 - image parameters, 0x34 holds the wrap mode and 0x38 the mag filter
    uint32_t* m_field0x8; //0x8
    uint16_t m_field0xc;  //0xc - palette index, selects the color table the image uploads with
    uint16_t m_field0xe;  //0xe
    uint32_t m_field0x10; //0x10
    uint32_t m_field0x14; //0x14
    uint32_t m_field0x18; //0x18 - 3 marks a render target, which is never bound as a normal texture

    /**
    * Will extract settings for that will be used for glTexParameteri
    * @see FUN_004245e0
    */
    void FUN_004281f0(DisaRenderObjectTexParameter* texParamters) const {
        texParamters->paletteIndex = m_field0xc;
        if (m_field0x4) {
            texParamters->magFilter = (uint8_t)m_field0x4[0xe];
            texParamters->wrapMode = (uint8_t)m_field0x4[0xd];
        }
        else {
            texParamters->magFilter = 0;
            texParamters->wrapMode = 1; //1 is never used and will be GL_CLAMP
        }
    }
};

/**
* @brief One entry of the render list. Filled by the submit functions of
* DisaRenderObjectManager and drawn by FUN_00426FB0.
* Total size: 0x74
*/
struct DisaRenderObject {
    DisaRenderFlags renderFlags; // 0x00 - inherited from the global flag stack at 0x005BB038
    float depth;              // 0x04 - primary sort key, 0 to 65535, smaller is drawn first
    float sortBias;           // 0x08 - tie break when two objects share a depth
    DisaRenderObjectSettings renderSettingsBitMask; // 0x0c - Packed render state
    int16_t primitiveType;    // 0x0e - index into the GL mode table at 0x00596EA0
    int32_t vertexCount;      // 0x10 - 0 means meshData is a display list id
    DisaTextureSPtr* texture; // 0x14
    DisaRenderObjectTexParameter textureInfo; // 0x18
    float* meshData;          // 0x1c - VertexSolidColor, VertexBasic or VertexWithNormal, or a display list id
    glm::mat4* modelMatrix;   // 0x20 - modelMatrix
    glm::mat4* projectionMatrix; // 0x24 - optional custom projection matrix (is mostly null)
    bool isPerspective;       // 0x28 - True: 3D, False: UI (orthographic)
    bool useColorTint;        // 0x29 - multiply and add split of the vertex color
    bool depthTestEnabled;    // 0x2a - Depth test enable, taken from the global at 0x005AC978
    bool depthMaskEnabled;    // 0x2b - Depth write enable
    float polygonOffset;      // 0x2c - glPolygonOffset units, scaled by the global at 0x005AC924
    DisaTextureSPtr* normalMap; // 0x30 - when set, the tangent attributes and a normal mapped program are used
    glm::mat4 modelViewMatrix;  // 0x34 - view times model, only filled on the immediate draw path
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
    glm::vec3* m_cameraEye;     // 0x00 (not fully confirmed (high))
    glm::vec3* m_cameraCenter;  // 0x04 - camera center, used to compute gluLookAt
    glm::vec3* m_cameraUp;      // 0x08 (not fully confirmed (high))
    glm::vec3* m_field0xc;      // 0x0c
    glm::vec3 m_lookDir;        // 0x10 - camera forward direction
    uint32_t m_field0x1c;       // 0x1c
    uint32_t m_prevRenderObjectListSize; // 0x20 - stores the last m_renderObjectListSize
    std::array<DisaRenderObject, 4096> m_renderObjects; //0x24 - Two blocks each max 2048 items, split at middle
    uint32_t m_renderObjectListSize; // 0x74024
    std::array<DisaRenderObject*, 2048> m_renderObjectList; // 0x74028 - pointers to objects in renderObjects
    std::array<int, 2> m_renderBlockSize; // 0x76028 - number of items inside each block
    std::array<float*, 2> m_field0x76030; // 0x76030 - mesh memory cash begin pointer for each block
    std::array<int, 2> m_field0x76038; // 0x76038 
    uint32_t m_renderBlockIndex; // 0x76040 - 0 or 1
    float m_depthOffset;        // 0x76044 - added to the depth of every object submitted
    float m_sortBias;           // 0x76048 - copied into the sortBias of every object submitted
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
        float poligonOffset, int stencilMode, unsigned char stencilRef, bool useColorTint, bool customData);

    void submitCustomModel(float depth, int modelIndex, int blendMode, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled, float poligonOffset, int stencilMode, unsigned char stencilRef, bool useColorTint);
    /**
    * @brief Queues an object that calls a compiled GL display list. Draws it right away while m_skipGlSetup is set.
    * Original function: FUN_00426200, a stdcall with two more stack arguments that are never read.
    * @param color Handed to glColor4ubv before the list is called.
    */
    void FUN_00426200_submitDisplayList(float depth, uint32_t listId, uint32_t color, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskDisabled, float polygonOffset);
    /**
    * @brief Bump allocator in the 1 MiB block of the current render block, freed when the block is reset.
    * Original function: FUN_00425bb0
    * @return nullptr when the block is full.
    */
    void* FUN_00425bb0_allocFrameMemory(uint32_t bytes);
};

/**
* @brief Render pass of FUN_00426770. Selects the shader every object is drawn with, and for the shadow pass also the camera the scene is rendered from.
* (NC)
*/
enum EDisaRenderPass : int {
    RenderPass_Default,      // 0 - plain textured or unlit, also used for the UI
    RenderPass_ShadowDepth,  // 1 - casters only, from the light, into the shadow map
    RenderPass_Scene,        // 2 - lit scene with shadow lookup and normal mapping
    RenderPass_SSAO,         // 3 - builds the occlusion buffer
    RenderPass_SSAOComposite,// 4 - multiplies the occlusion buffer into the scene
    RenderPass_Blur,         // 5 - uniform blur, used on the occlusion buffer
    RenderPass_BlurPosition, // 6 - blur strength from screen position
    RenderPass_DepthOfField, // 7 - blur strength from depth, uses focusZ
    RenderPass_BlurAndDof,   // 8 - position and depth combined
};

/**
* @brief Binds the program linked from the given shader pair and points its tex0 to tex7 samplers
* at the matching texture units. The pair (10, 17) has no shaders, so binding it returns to fixed
* function.
*/
void FUN_00421a30_glUseAndInitProgram(int vertexShaderIndex, int fragmentShaderIndex);

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
void _stdcall FUN_00426770(EDisaRenderPass pass, unsigned int param_2, bool depthTestEnabled);
/**
* @brief Submit render data to render queue
*/
void __stdcall FUN_00425df0_SubmitVertexBasic(float param_1, int* textureDataPtr, void* geometryData, int triangleCount, short sizeUniforms, int param_6, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled,
    float poligonOffset, int param_11, bool param_12, bool useColorTint);