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

#include <memory>
#include <string>
#include <Util/PG_File.h>

class PG_ConfigFile;

/**
* @brief The Disa values inside the application config. @see PG_Application::getConfig
* @details Without a config file one is started beside the executable, which the first change then creates.
* Must be constructed after the application.
*/
class PG_DisaConfig {
public:
	PG_DisaConfig();
	~PG_DisaConfig();

	bool isFancySplashScreen() const;
	std::string getFancySplashScreenText() const;
	std::string getSplashScreenText() const;
	std::string getDisaExePath() const;
	std::string getDisaExeVersion() const;

	void setExePath(const std::string& version, const PG_File& path);

private:
	PG_ConfigFile* m_config = nullptr; /// the application's, or m_ownConfig when it has none
	std::unique_ptr<PG_ConfigFile> m_ownConfig;
};