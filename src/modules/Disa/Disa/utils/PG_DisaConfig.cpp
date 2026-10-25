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

#include "PG_DisaConfig.h"
#include <Base/PG_Application.h>
#include <Script/PG_ConfigFile.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_Exception.h>

PG_DisaConfig::PG_DisaConfig() {
	m_config = PG_Application::config();
	if (m_config) return;

	m_ownConfig = std::make_unique<PG_ConfigFile>();
	try {
		m_ownConfig->open(PG_File::getExecutable().getDir() + "/config.ini");
	}
	catch (...) {} //expected, there is no file yet, but the path is kept for the save
	m_config = m_ownConfig.get();
}

PG_DisaConfig::~PG_DisaConfig() = default;

bool PG_DisaConfig::isFancySplashScreen() const {
	return m_config->getValue<bool>("fancySplashScreen", false);
}
std::string PG_DisaConfig::getFancySplashScreenText() const {
	return m_config->getValue<std::string>("fancySplashScreenText", std::string("I forgot what to say~"));
}
std::string PG_DisaConfig::getSplashScreenText() const {
	return m_config->getValue<std::string>("splashScreenText", std::string("Failed to read \"splashScreenText\" in config.ini!"));
}
std::string PG_DisaConfig::getDisaExePath() const {
	return m_config->getValue<std::string>("disaExe", std::string());
}
std::string PG_DisaConfig::getDisaExeVersion() const {
	return m_config->getValue<std::string>("disaExeVersion", std::string());
}

void PG_DisaConfig::setExePath(const std::string& version, const PG_File& path) {
	bool change = m_config->setValueChange<std::string>("disaExeVersion", version);
	change |= m_config->setValueChange<std::string>("disaExe", path);
	if (!change) return;

	try {
		m_config->save();
	}
	catch (const PG_Exception& ex) {
		OUT_ERROR("Failed to save \"" << m_config->getFile() << "\": " << ex.what());
	}
}