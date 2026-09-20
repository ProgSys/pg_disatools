/*
 * GNU Lesser General Public License (LGPL-3.0)
 *
 * Copyright (c) 2026 ProgSys
 *
 * This code is part of a free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * This code is distributed in the hope that it will be useful, without any
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 * or https://www.gnu.org/licenses/lgpl-3.0.md
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <glm/glm.hpp>

#include "DisaRendering.h"

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
    glm::ivec4 cullRect;        // 0x24 - derived from everything above (left, right, top, bottom)
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
    DisaProjection** projections; // 0x04 - heap array of projectionCount pointers
    DisaProjection* selected;     // 0x08 - slot OpenGL holds, null until the first select
    int32_t selectedIndex;        // 0x0c
    bool isEnabled;               // 0x10 - false makes selecting a slot do nothing
    bool isForceReload;           // 0x11 - upload the matrix even if the slot did not change
    uint16_t padding0x12;         // 0x12 - unused

    /// Instance located at 0x005CBBC0
    static DisaProjectionManager* get();
};

/**
* @brief Binds the blend state display list of a textured draw.
* A mode of -1 falls back to the global default at 0x005BAE6C.
*/
void FUN_0041fee0_glCallBlendList(int blendMode);
/**
* @brief Same as FUN_0041fee0_glCallBlendList, but for the list set an untextured draw uses.
*/
void FUN_0041ff30_glCallBlendListUntextured(int blendMode);
/**
* @brief Makes the unit the texture wants to live on the active one. The unit is the texture's
* own field0x18, so a render target always ends up on unit 3.
*/
void FUN_00428120_glActiveTexture(const DisaTextureSPtr* texture);
/**
* @brief Uploads the negated light direction as the "lightPos" uniform of the bound program.
*/
void FUN_00421bf0_glSetLightPosUniform(const glm::vec3& lightPos);
/**
* @brief Uploads the "invViewMtx" uniform of the bound program.
*/
void FUN_00421f40_glSetInvViewMtxUniform(const glm::mat4& invViewMatrix);
/**
* @brief Uploads the "img_Width" and "img_Height" uniforms. Always targets the program at
* 0x006E88B8, never the bound one.
*/
void FUN_00421ba0_glSetImageSizeUniform(float width, float height);

/**
* @brief Draws one render object: the OpenGL state it asks for, the program the pass wants,
* its textures, and finally the geometry.
* @param skipGlSetup The caller already applied stencil, depth and polygon offset, and picks the
* program itself. The submit functions pass true when they draw straight away.
*/
void FUN_00426fb0_RenderObject(DisaRenderObject* object, bool skipGlSetup, EDisaRenderPass pass);
