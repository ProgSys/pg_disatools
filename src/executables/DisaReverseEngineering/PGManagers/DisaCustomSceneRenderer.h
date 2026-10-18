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

#include <Renderer/PG_Renderer.h>
#include <Resources/PG_Material.h>
#include <Resources/PG_Geometry.h>
#include <Resources/PG_Texture.h>
#include <memory>

class PG_GuiMain;

/**
* @brief Collects the gui of the custom scene into a render job, which the game's UI pass runs later.
* @details The job is built on every PG_Event_Render, during the event loop of the frame start. It is stored
* and not rendered, because the game owns the GL context and the framebuffer: FUN_00426770_RenderObjects calls
* renderStored at the end of the UI pass, which draws into the window at its full resolution.
*
* Only the gui is collected. The world and screen groups are queued in the game's own render list by
* DisaCustomRenderManager::submit, since the game decides their depth order.
*/
class DisaCustomSceneRenderer : public PG_Renderer
{
	PG_OBJECT(DisaCustomSceneRenderer);
	PG_PARENT(PG_Renderer);
public:
	using PG_Renderer::PG_Renderer;
	~DisaCustomSceneRenderer() override;

	/// Loads the clip and error resources and checks for a stencil buffer. Needs the game's context to be current.
	void init();

	inline PG_GuiMain* getGuiMain() const { return m_guiMain; }
	/// The gui that is collected, it has to live in the scene of this renderer
	inline void setGuiMain(PG_GuiMain* guiMain) { m_guiMain = guiMain; }

	std::pair<PG_RenderJobEvent*, PG_GarbageJobEvent*> createJob(PG_RenderEvent* e) override;
	/**
	* @brief Runs the stored jobs into the currently bound framebuffer, then releases them.
	* @details Only call from the game thread while the game renders. Saves and restores the game's GL state.
	*/
	void renderStored();

	void render(const PG_RenderJob_SPtr& job);
	/// Keeps the creation commands of a job that was replaced before it was drawn
	void merge(PG_RenderJob_SPtr& genericJob, PG_RenderJob_SPtr& genericJobOld);
	void mergeGarbage(PG_RenderJob_SPtr& genericJob, PG_RenderJob_SPtr& genericJobOld);
	void destroyGarbage(const PG_RenderJob_SPtr& job);
protected:
	bool isReady() const override;
	/// A newer job replaces one that was never drawn and takes over its creation commands and garbage
	void submitJobs(PG_RenderJobEvent* renderJob, PG_GarbageJobEvent* garbageJob) override;
private:
	PG_GuiMain* m_guiMain = nullptr;
	std::unique_ptr<PG_RenderJobEvent> m_job;
	std::unique_ptr<PG_GarbageJobEvent> m_garbage;
	bool m_hasStencil = false;

	PG_Material m_clipMat;
	PG_Geometry m_clipGeo;
	PG_Texture m_errorTexture;
	PG_Material m_errorMaterial;
};
