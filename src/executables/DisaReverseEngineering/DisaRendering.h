#pragma once
#include <glm/glm.hpp>
#include <ranges>
#include <array>
#include <Resources/PG_Model.h>

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

    /// Pointers outside 0x10001 to 0xEFFFFFFF are marker values, the game never counts or frees them.
    static bool isCounted(const DisaTextureSPtr* texture);
    /**
    * @brief Drops one reference and frees the texture when it was the last one.
    * Original function: FUN_0041E5C0
    */
    static void FUN_0041e5c0_release(DisaTextureSPtr* texture);
    /**
    * @brief Takes a reference to source, stores it in target and releases what target held before.
    * Original function: FUN_0041E560
    */
    static void FUN_0041e560_assign(DisaTextureSPtr*& target, DisaTextureSPtr* source);
    /**
    * @brief Releases what target holds and sets it to nullptr.
    * Original function: FUN_004275B0
    */
    static void FUN_004275b0_reset(DisaTextureSPtr*& target);
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
    /*
    * The submit functions below fill the next free slot of the current render block and append it to the render
    * list, or draw it right away while m_skipGlSetup is set. Nothing is queued while the scene does not draw or
    * the block holds 2048 objects. Fields a submit does not write keep the value of the slot, which is zero after
    * the per frame reset (FUN_00425CC0).
    *
    * Every member that has a free stdcall original also has a static Std version with the exact original stack
    * layout. Only those are used to replace call sites of the game. Textures are borrowed by the members, the
    * reference the originals take by value is released by the Std version.
    *
    * depthMaskDisabled is true for translucent objects, they are drawn without writing depth.
    */

    /**
    * @brief Queues VertexBasic mesh data with one texture.
    * Original function: FUN_00425df0
    * @param vertices Copied into the current render block unless they already live there.
    */
    void FUN_00425df0_submitVertexBasic(float depth, DisaTextureSPtr* texture, VertexBasic* vertices, int vertexCount, short primitiveType, int blendMode, glm::mat4* modelMatrix,
        bool isPerspective, bool depthMaskDisabled, float polygonOffset, int stencilMode, uint8_t stencilRef, bool useColorTint);
    /// Call site replacement for FUN_00425df0. Releases the texture reference the original takes by value.
    static void __stdcall FUN_00425df0_submitVertexBasicStd(float depth, DisaTextureSPtr* texture, VertexBasic* vertices, int vertexCount, short primitiveType, int blendMode,
        glm::mat4* modelMatrix, bool isPerspective, bool depthMaskDisabled, float polygonOffset, int stencilMode, uint8_t stencilRef, bool useColorTint);
    /**
    * @brief FUN_00425df0_submitVertexBasic for mesh data owned by the caller, not part of the game.
    * The vertices are used in place and never copied, so they have to stay alive until the frame is rendered.
    */
    void submitVertexBasicCustom(float depth, DisaTextureSPtr* texture, VertexBasic* vertices, int vertexCount, short primitiveType, int blendMode, glm::mat4* modelMatrix,
        bool isPerspective, bool depthMaskDisabled, float polygonOffset, int stencilMode, uint8_t stencilRef, bool useColorTint);

    /**
    * @brief Queues untextured VertexSolidColor mesh data.
    * Original function: FUN_00426040. Always writes depth, clears the stencil settings and leaves useColorTint as the slot holds it.
    */
    void FUN_00426040_submitSolidColor(float depth, VertexSolidColor* vertices, int vertexCount, short primitiveType, int blendMode, glm::mat4* modelMatrix, bool isPerspective,
        float polygonOffset);
    /**
    * @brief Call site replacement for FUN_00426040.
    * The unused slots keep the parameter list of FUN_00425df0. Callers reserve them but never write them, so they hold garbage.
    */
    static void __stdcall FUN_00426040_submitSolidColorStd(float depth, VertexSolidColor* vertices, int vertexCount, short primitiveType, int blendMode, glm::mat4* modelMatrix,
        bool isPerspective, int unusedDepthMask, float polygonOffset, int unusedStencilMode, int unusedStencilRef);

    /**
    * @brief Queues an object that calls a compiled GL display list. Draws it right away while m_skipGlSetup is set.
    * Original function: FUN_00426200. Leaves texture, blend mode and primitive type as the slot holds them.
    * @param color Handed to glColor4ubv before the list is called.
    */
    void FUN_00426200_submitDisplayList(float depth, uint32_t listId, uint32_t color, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskDisabled, float polygonOffset);
    /// Call site replacement for FUN_00426200. The last two slots are never read.
    static void __stdcall FUN_00426200_submitDisplayListStd(float depth, uint32_t listId, uint32_t color, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskDisabled,
        float polygonOffset, int unusedArg8, int unusedArg9);

    /**
    * @brief Queues VertexBasic mesh data with a color texture and a normal map.
    * Original function: FUN_00426350. Always perspective, writes depth, no color tint and no stencil.
    * The vertices are not copied while m_skipGlSetup is set.
    * @param textures [0] color texture, [1] normal map. Borrowed, the original takes no reference to the array.
    */
    void FUN_00426350_submitVertexBasicNormalMap(float depth, DisaTextureSPtr* const* textures, VertexBasic* vertices, int vertexCount, short primitiveType, int blendMode,
        glm::mat4* modelMatrix, float polygonOffset);
    /// Call site replacement for FUN_00426350. The unused slots keep the parameter list of FUN_00425df0.
    static void __stdcall FUN_00426350_submitVertexBasicNormalMapStd(float depth, DisaTextureSPtr* const* textures, VertexBasic* vertices, int vertexCount, short primitiveType,
        int blendMode, glm::mat4* modelMatrix, int unusedPerspective, int unusedDepthMask, float polygonOffset, int unusedStencilMode, int unusedStencilRef, int unusedColorTint);

    /**
    * @brief Queues VertexWithNormal mesh data with a color texture and a normal map.
    * Original function: FUN_00426530, a thiscall, so this member replaces call sites directly.
    * The vertices are not copied while m_skipGlSetup is set, and the immediate draw computes modelViewMatrix itself.
    * @param textures [0] color texture, [1] normal map. Borrowed.
    */
    void FUN_00426530_submitVertexWithNormal(float depth, DisaTextureSPtr* const* textures, VertexWithNormal* vertices, int vertexCount, short primitiveType, int blendMode,
        glm::mat4* modelMatrix, bool isPerspective, bool depthMaskDisabled, float polygonOffset, int stencilMode, uint8_t stencilRef, bool useColorTint);

    /**
    * @brief FUN_00426530_submitVertexWithNormal with the two textures given one by one.
    * Original function: FUN_004f3450. Holds an own reference to both textures for the duration of the call.
    */
    void FUN_004f3450_submitVertexWithNormalRef(float depth, DisaTextureSPtr* texture, DisaTextureSPtr* normalMap, VertexWithNormal* vertices, int vertexCount, short primitiveType,
        int blendMode, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskDisabled, float polygonOffset, int stencilMode, uint8_t stencilRef, bool useColorTint);
    /// Call site replacement for FUN_004f3450. Releases both texture references the original takes by value.
    static void __stdcall FUN_004f3450_submitVertexWithNormalRefStd(float depth, DisaTextureSPtr* texture, DisaTextureSPtr* normalMap, VertexWithNormal* vertices,
        int vertexCount, short primitiveType, int blendMode, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskDisabled, float polygonOffset, int stencilMode,
        uint8_t stencilRef, bool useColorTint);

    void submitCustomModel(float depth, int modelIndex, int blendMode, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled, float poligonOffset, int stencilMode, unsigned char stencilRef, bool useColorTint);
    /**
    * @brief Bump allocator in the 1 MiB block of the current render block, freed when the block is reset.
    * Original function: FUN_00425bb0
    * @return nullptr when the block is full.
    */
    void* FUN_00425bb0_allocFrameMemory(uint32_t bytes);

private:
    /**
    * @brief Gate and slot shared by every submit. Writes depth, sortBias and the projection selector.
    * @return nullptr when the scene does not draw or the current block is full.
    */
    DisaRenderObject* acquireRenderObject(float depth);
    /// Render flags of the current scope, the global depth test, and the matrix overrides of the manager.
    void inheritScope(DisaRenderObject* object) const;
    /// The vertices when they already live in the current render block, otherwise a copy in frame memory.
    float* keepMeshData(void* vertices, uint32_t bytes);
    /// Appends the object to the render list and counts it in its block.
    void queueRenderObject(DisaRenderObject* object);
    /// Immediate draw while m_skipGlSetup is set. The slot is not counted, so the next submit reuses it.
    void drawRenderObjectNow(DisaRenderObject* object, int blendMode);
    void submitVertexBasic(float depth, DisaTextureSPtr* texture, VertexBasic* vertices, int vertexCount, short primitiveType, int blendMode, glm::mat4* modelMatrix,
        bool isPerspective, bool depthMaskDisabled, float polygonOffset, int stencilMode, uint8_t stencilRef, bool useColorTint, bool copyVertices);
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
* @brief An off screen render target, the class the game calls CGraRenderMap.
* The scene (0x008B7DC8) and effect (0x008B7DCC) targets use this layout. The shadow map
* (0x008B7DC4) is its own class with the same vtable slots, size 0xF0, and keeps its view,
* projection and light modelview matrices at 0x30, 0x70 and 0xB0.
* Total size: 0x30
*/
struct DisaRenderTarget {
    void** m_vtable;              // 0x00
    uint32_t m_colorTexture;      // 0x04
    int m_textureUnit;            // 0x08 - index into the GL_TEXTUREi table at 0x00531698
    int m_width;                  // 0x0c
    int m_height;                 // 0x10
    int m_bindMode;               // 0x14 - mode of the last bind, 0 when unbound
    uint32_t m_framebuffer;       // 0x18
    uint32_t m_depthRenderbuffer; // 0x1c
    uint32_t m_depthTexture;      // 0x20 - bound to GL_TEXTURE4
    float m_focusZ;               // 0x24 - average centre depth, written by FUN_004231B0
    uint32_t m_colorTexture1;     // 0x28 - second draw buffer, bound to GL_TEXTURE5
    uint32_t m_colorTexture2;     // 0x2c - third draw buffer, bound to GL_TEXTURE6

    /// Creates the GL names. Vtable slot 0x08, FUN_00422A00 for the scene target.
    void generate();
    /// Deletes the GL names and zeroes them. Vtable slot 0x0C, FUN_00422A50 for the scene target.
    void release();
    /**
    * @brief Allocates the textures and attaches them to the framebuffer.
    * Vtable slot 0x10, FUN_00422C60 for the scene target.
    */
    void allocate(int width, int height);
    /**
    * @brief Binds the framebuffer and sets the viewport. Vtable slot 0x14, FUN_00422F90 for the scene target.
    * @param mode 1 uses the target size and clears color and depth, 2 uses the window size, 3 uses the target size and keeps the content.
    */
    void bind(int mode);
    /// Returns to the default framebuffer with a window sized viewport. Vtable slot 0x18, FUN_00423040.
    void unbind();
    /**
    * @brief Binds the target textures for sampling. Vtable slot 0x1C, FUN_00423120 for the scene target.
    * @param unit Texture unit of the color texture, -1 uses m_textureUnit.
    */
    void bindTextures(int unit);
};

/**
* @brief Renders one frame through the render targets: shadow map, lit scene, SSAO, blur or depth of field, then the UI.
* Only called while the render targets exist (0x00889A23), otherwise the game renders pass 0 directly.
* Original function: FUN_00509300
*/
void FUN_00509300_RenderFrame();

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
void _stdcall FUN_00426770_RenderObjects(EDisaRenderPass pass, unsigned int param_2, bool depthTestEnabled);