/*
 * GNU Lesser General Public License (LGPL-3.0)
 *
 * Copyright (c) 2025 ProgSys
 *
 * This code is part of a free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * This code is distributed in the hope that it will be useful,
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

#define DISA_ISNTALL_APP_VERSION "0.3.3"

#include <Scene/Gui/PG_GuiItem.h>
#include <Util/PG_File.h>

class PG_DisaInstaller : public PG_GuiItem {
	PG_OBJECT(PG_DisaInstaller)
	PG_PARENT(PG_GuiItem)
signals:
	void done(std::string version, const PG_File& exeFile);
public:
	using PG_GuiItem::PG_GuiItem;

	void runInstall(std::string version, PG_File exePath);

private:
	void runFreshInstall();
	std::string execInstall(PG_File exePath, bool doUnzip);
};
