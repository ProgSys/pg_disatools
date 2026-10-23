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

#include <atomic>
#include <functional>
#include <memory>
#include <Base/PG_Object.h>
#include <Resources/PG_ResourceSource.h>

/**
* @brief Lives under the resource manager beside a registered source and asks it once a second to let
* go of what it holds when idle.
* @details Created on whatever thread registered the source, so it moves itself to the managers thread
* before it touches the object tree. Deletes itself once the source sets released or is gone.
*
* Create it with new from PG_ResourceSource::onRegistered and keep only the released flag, the manager
* owns it once it attached.
*/
class PG_DisaSourceIdleCompanion : public PG_Object {
public:
	/// Called on the managers thread with the source still alive.
	using Check = std::function<void(PG_ResourceSource& source)>;

	PG_DisaSourceIdleCompanion(PG_ResourceManager* manager, PG_ResourceSource_WPtr source, std::shared_ptr<std::atomic_bool> released, Check check);

private:
	void attach();
	void check();

	PG_ResourceManager* m_manager;
	PG_ResourceSource_WPtr m_source;
	std::shared_ptr<std::atomic_bool> m_released;
	Check m_check;
};
