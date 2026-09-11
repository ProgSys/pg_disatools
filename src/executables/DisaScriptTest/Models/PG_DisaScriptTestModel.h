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

#include <Disa/models/PG_DisaMainModel.h>
#include <Disa/PG_DisaMapModel.h>
#include <Util/PG_File.h>
#include <memory>
#include <string>

class PG_PSPFS;
class PG_DisaLooseFileContainer;
class PG_DisaCharactersModel;
class PG_DisaMapScene;

/**
* @brief Owns the map that is shown, and everything needed to read one
* @details Holds the PG_DisaMapModel the view is bound to. Opening another map refills that same
* model, so nothing has to be rebound. This is where the script commands will live.
*/
class PG_DisaScriptTestModel : public PG_DisaMainModel {
	PG_OBJECT(PG_DisaScriptTestModel);
	PG_PARENT(PG_DisaMainModel);
public:
	using PG_DisaMainModel::PG_DisaMainModel;

	void init() override;

	/**
	* @brief Opens the game archives the map view reads from
	* @details Without it there are no maps, and without its character data actors show as cubes.
	*/
	bool openGameFiles();

	/**
	* @brief Opens a map out of DATA.DAT
	* @param mapName With or without the extension, so "MP00701" and "MP00701.MPD" both work
	*/
	bool openMap(const std::string& mapName);
	/**
	* @brief Opens a loose mpd or mpp
	* @details The partner files are taken from the same folder, an mpp resolves to its mpd.
	*/
	bool openMap(const PG_File& file);

	PG_DisaMapModel* getMapModel() const { return m_mapModel; }

	PG_DisaMapScene* getScene() const { return m_scene; }
	void setScene(PG_DisaMapScene* scene) { m_scene = scene; }

private:
	PG_DisaMapModel* m_mapModel = nullptr;
	PG_DisaMapScene* m_scene = nullptr;

	/* The map reads its textures through these, so they have to outlive the map. Holding the
	 * archives is what a PG_DisaMPDContainer would do, and none is built here. */
	std::shared_ptr<PG_PSPFS> m_dataDat;
	std::shared_ptr<PG_PSPFS> m_subdata;
	std::shared_ptr<PG_DisaLooseFileContainer> m_looseFiles;

	PG_DisaCharactersModel* m_charactersModel = nullptr;
};
