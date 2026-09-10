/*
 * GNU Lesser General Public License (LGPL-3.0)
 *
 * Copyright (c) 2020 ProgSys
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

#include "RootScene.h"

#include <PG_DisaDefines.h>
#include <Base/PG_Application.h>
#include <Base/Events/PG_TickEvent.h>
#include <Base/Events/PG_KeyEvent.h>
#include <Scene/Gui/PG_GuiMain.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiToggleButton.h>
#include <Scene/Gui/PG_GuiRenderItem.h>
#include <Scene/Gui/PG_GuiGridView.h>
#include <Scene/Gui/PG_GuiScrollBar.h>
#include <Scene/Gui/PG_GuiModel.h>
#include <Scene/Gui/PG_GuiTooltip.h>
#include <Scene/Gui/PG_GuiTextField.h>
#include <Scene/Gui/PG_GuiWindowPopup.h>
#include <Scene/Gui/PG_GuiRow.h>
#include <Scene/Gui/PG_GuiColumn.h>
#include <Scene/Gui/PG_GuiPopup.h>
#include <Scene/Gui/PG_GuiTextEdit.h>
#include <Scene/Gui/PG_GuiMenuPopup.h>
#include <Scene/Gui/PG_GuiCreateSelectionElement.h>
#include <Scene/Gui/PG_GuiLines.h>
#include <Scene/Gui/PG_GuiFlowView.h>
#include <Scene/PG_CameraNode.h>
#include <Scene/PG_ModelNode.h>
#include <CustomGui/PG_DisaItemSelector.h>
#include <CustomGui/PG_DisaCustomCampaign.h>
#include <CustomGui/PG_DisaAddDungeonDialog.h>
#include <Dialog/PG_GuiDialog.h>
#include <Renderer/PG_SceneRenderer.h>
#include <Resources/PG_ResourceManager.h>
#include <Models/PG_CategoryFilterModel.h>

#include <Disa/fileformats/PG_PSPFS.h>
#include <Disa/fileformats/PG_ZIP.h>
#include <Disa/PG_DisaAssetsModel.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/utils/PG_DisaConfig.h>
#include <Disa/utils/PG_DisaInstaller.h>
#include <Disa/CustomGui/PG_PupUpDialog.h>
#include <Disa/CustomGui/PG_DisaSearchField.h>

#include <PG_DisaGridTile.h>
#include <PG_DisaMapListModel.h>
#include <PG_DisaAssetsWindow.h>
#include <Models/PG_DisaMapEditorMainModel.h>
#include <Scene/Model/PG_ListModelFilter.h>

#include <Util/PG_Dir.h>
#include <Util/PG_MD5.h>
#include <Util/Color/ColorDefined.h>
#include <Util/Debug/PG_DebugOut.h>

#include <Disa/PG_DisaMapScene.h>
#include <Disa/map/PG_DisaEditorModule.h>
#include <SDLApp/PG_SDLWindow.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Inspector.h>
#include <nfd.h>

constexpr glm::vec3 backgroundColor = glm::vec3(40.f / 255.f);
constexpr glm::vec3 backgroundDecoColor = glm::vec3(45.f / 255.f);
constexpr glm::vec3 backgroundDeco2Color = glm::vec3(30.f / 255.f);
constexpr glm::vec3 backgroundDarkColor = glm::vec3(20.f / 255.f);
constexpr glm::vec3 offWeightColor = glm::vec3(200.f / 255.f);
constexpr glm::vec3 blueColor = glm::vec3(133.f, 208.f, 255.f) / 255.f;
constexpr glm::vec3 bobbleTextColor = glm::vec3(76.f, 111.f, 199.f) / 255.f;

static inline PG_DisaMapScene* getGameScene(PG_GuiItem* center) {
	if (auto item = dynamic_cast<PG_GuiRenderItem*>(center)) {
		return dynamic_cast<PG_DisaMapScene*>(item->findFirstInTree(PG_Search::byName("GameScene")));
	}
	return nullptr;
}

void RootScene::clearCenter() {
	if (!m_center) return;
	emit cleanUp();
	m_center->deleteLater();
	m_center = nullptr;
}

void RootScene::init() {
	PG_Scene::init();

	if (!m_mainModel) m_mainModel = PG_new<PG_DisaMapEditorMainModel>("PG_DisaMapEditorMainModel", this);
	auto& config = m_mainModel->getConfig();

	auto guiMain = PG_new<PG_GuiMain>("guiMain", getRoot());
	rootItem = PG_new<PG_GuiItem>("rootItem", glm::vec2(0.f), glm::vec2(300.f, 400.f), guiMain);
	{
		auto cam = PG_new<PG_CameraNode>("N_CameraNode", PG_CameraNode::Type_Orthographic, getRoot());
		cam->setTranslation(glm::vec3(-5.0f, 0.0f, 0.0f));
		cam->setZoom(120.0f);
		setActiveCamera(cam);
	}
	{//map editor toolbar

		toolbar = PG_new<PG_GuiRectangle>("toolbar", glm::vec2(0.f), glm::vec2(10.f, 50.f), rootItem);
		toolbar->setColor(backgroundColor);
		toolbar->getAnchor(PG_Anchor::Top)->setAnchor(rootItem->getAnchor(PG_Anchor::Top));
		toolbar->getAnchor(PG_Anchor::Left)->setAnchor(rootItem->getAnchor(PG_Anchor::Left));
		toolbar->getAnchor(PG_Anchor::Right)->setAnchor(rootItem->getAnchor(PG_Anchor::Right));

		/*
		fpsItem = PG_new<PG_GuiText>("FPS", glm::vec2(0.f, -5.f), glm::vec2(110.f, 30.f), "Fps: 0", toolbar);
		fpsItem->setWrapMode(PG_GuiText::None);
		fpsItem->setTranslationZ(5.f);
		fpsItem->setFontSize(5.f);
		fpsItem->setColor(blueColor);
		fpsItem->setHorizontalAlignment(PG_Anchor::Left);
		fpsItem->setVerticalAlignment(PG_Anchor::VerticalCenter);
		fpsItem->setBaselineScale(0.7f);
		fpsItem->getAnchor(PG_Anchor::Top)->setAnchor(toolbar->getAnchor(PG_Anchor::Top));
		fpsItem->getAnchor(PG_Anchor::Bottom)->setAnchor(toolbar->getAnchor(PG_Anchor::Bottom));
		fpsItem->getAnchor(PG_Anchor::Right)->setAnchor(toolbar->getAnchor(PG_Anchor::Right));
		*/

		toolbarRow = PG_new<PG_GuiRow>("ToolbarRow", glm::vec2(0.f, 10.f), glm::vec2(39.f, 30.f), toolbar);
		toolbarRow->setSpacing(3.f);
		toolbarRow->setMargin(PG_Anchor::Left, 10.f);
		toolbarRow->setMargin(PG_Anchor::Right, 10.f);
		toolbarRow->fill(toolbar);
	}

	{ //default center
		PG_GuiItem* defaultWrapper = PG_new<PG_GuiItem>("defaultWrapper", rootItem);
		PG_GuiItem::anchor(defaultWrapper, PG_Anchor::Top, toolbar, PG_Anchor::Bottom);
		PG_GuiItem::anchor(defaultWrapper, PG_Anchor::Left, rootItem, PG_Anchor::Left);
		PG_GuiItem::anchor(defaultWrapper, PG_Anchor::Right, rootItem, PG_Anchor::Right);
		PG_GuiItem::anchor(defaultWrapper, PG_Anchor::Bottom, rootItem, PG_Anchor::Bottom);

		m_center = defaultWrapper;

		if (config.isFancySplashScreen()) {
			auto dialogFrame = PG_new<PG_GuiDialog>("N_DialogFrame", defaultWrapper);
			dialogFrame->setText(config.getFancySplashScreenText());
			dialogFrame->fill(defaultWrapper);
		}
		else {
			auto dialogFrame = PG_new<PG_GuiText>("N_DialogFrame", defaultWrapper);
			dialogFrame->setFontSize(20.f);
			dialogFrame->setColor(blueColor);
			dialogFrame->setHorizontalAlignment(PG_GuiText::HorizontalLeftCenter);
			dialogFrame->setVerticalAlignment(PG_Anchor::VerticalCenter);

			dialogFrame->fill(defaultWrapper);
			dialogFrame->setText(config.getSplashScreenText());
			defaultWrapper->moveToFront();
		}
	}

	{ //tooltip
		guiMain->setTooltipItem(PG_new<PG_GuiTooltip>("Tooltip", rootItem));
	}

	connect(m_mainModel, &PG_DisaMapEditorMainModel::uiStateChanged, this, [this](PG_UiStateBaseModel::UiState state) {
		if (auto window = findFirstChild<PG_GuiWindowPopup>("AssetsWindow"); window && !window->isDeleteLater()) {
			window->close();
		}

		switch (state)
		{
		case PG_UiStateBaseModel::Empty: {
			clearCenter();
		}break;
		case PG_UiStateBaseModel::Overview:
		case PG_UiStateBaseModel::GridFileView:
			setupGridView();
			setupToolbarGeneric();
			break;
		case PG_UiStateBaseModel::MapView:
			setupMapView();
			setupToolbarMapView();
			break;
		case PG_UiStateBaseModel::ModelView:
			break;
		case PG_UiStateBaseModel::CustomCampaignSetting:
			setupCustomCampaignSettingView();
			setupToolbarGeneric();
			break;
		case PG_UiStateBaseModel::CustomCampaignPlan:
			setupCustomCampaignPlanView();
			setupToolbarGeneric();
			break;
		default:
			break;
		}
	});

	connect(m_mainModel, &PG_DisaMapEditorMainModel::displayProgress, this, [this](const std::string& text, float progress) {
		PG_PopUpDialog::openProgress(rootItem,
			"Please wait",
			text,
			progress
		);
		});


	postCallEvent(&RootScene::openInit, config.getDisaExeVersion(), PG_File(config.getDisaExePath()));
}

void RootScene::openInit(std::string version, PG_File exePath) {
	PG_DisaInstaller* install = PG_new<PG_DisaInstaller>(rootItem);
	connect(install, &PG_DisaInstaller::done, this, [this](std::string newVersion, const PG_File& newExeFile) {
		m_mainModel->getConfig().setExePath(newVersion, newExeFile);
		openEXE(newExeFile);
	});

	install->runInstall(version, exePath);
}


static inline std::pair<PG_GuiRenderItem*, PG_DisaMapScene*> createMapScene(PG_GuiItem* parent) {
	auto renderWindow = PG_new<PG_GuiRenderItem>("GridRenderItem", parent);
	auto sceneRenderer = PG_new<PG_SceneRenderer>("GameSceneRenderer", true, 2, renderWindow);
	auto gameScene = PG_new<PG_DisaMapScene>("GameScene", sceneRenderer);
	return { renderWindow, gameScene };
}

void RootScene::reset() {
	if (m_center) m_center->deleteLater();
	//if (m_mainModel) delete m_mainModel;
	//m_mainModel = nullptr;
}

bool RootScene::event(PG_Event* e) {
	switch (e->getType())
	{
		case PG_Event_Key: {
			auto key = static_cast<PG_KeyEvent*>(e);
			if (key->getKey() == PG_Key_F5 && !key->isRepeat() && key->isUp()) {
				PG_Shader::reloadAll();
				key->accept();
				return true;
			}
		}break;
	}

	return PG_Scene::event(e);
}

void RootScene::openClicked() {
	PG_Application::grabMouse((PG_Object*)PG_Object::caller());
	nfdchar_t* outPath = nullptr;
	nfdresult_t result = NFD_OpenDialog("exe,geo", nullptr, &outPath);

	PG_EventLoop::doRun();
	PG_Application::grabMouse(nullptr);

	std::string path;
	if (outPath) {
		path = outPath;
		delete[] outPath;
	}

	if (result == NFD_OKAY) {
		open(PG_File(path));
	}
	else if (result != NFD_CANCEL) {
		OUT_ERROR("OpenDialog error: " << NFD_GetError());

		if (auto window = findParent<PG_SDLWindow>())
			window->setTitle(APP_NAME);
	}

	//remove all events that happen while dialog was open
	/*
	SDL_PumpEvents();
	SDL_FlushEvents(SDL_MOUSEMOTION, SDL_MOUSEWHEEL);
	SDL_FlushEvent(SDL_KEYUP);
	SDL_FlushEvent(SDL_KEYDOWN);
	*/
}

void RootScene::open(const PG_File& file) {
	const std::string ext = file.getFileExtensionFormated();
	if (ext == "mpd" || ext == "json")
		openMPD(file);
	else if (ext == "geo")
		openGEO(file);
	else if (ext == "exe")
		openEXE(file);

	if (auto window = findParent<PG_SDLWindow>())
		window->setTitle(APP_NAME " - " + file.getPath());
}

void RootScene::openGEO(const PG_File& file) {
	reset();
	auto rootItem = dynamic_cast<PG_GuiItem*>(getRoot()->findFirstInTree(PG_Search::byName("rootItem")));
	auto toolbar = dynamic_cast<PG_GuiItem*>(getRoot()->findFirstInTree(PG_Search::byName("toolbar")));
	if (!rootItem || !toolbar) return;

	auto [item, gameScene] = createMapScene(rootItem);
	item->getAnchor(PG_Anchor::Top)->setAnchor(toolbar->getAnchor(PG_Anchor::Bottom));
	item->getAnchor(PG_Anchor::Left)->setAnchor(rootItem->getAnchor(PG_Anchor::Left));
	item->getAnchor(PG_Anchor::Right)->setAnchor(rootItem->getAnchor(PG_Anchor::Right));
	item->getAnchor(PG_Anchor::Bottom)->setAnchor(rootItem->getAnchor(PG_Anchor::Bottom));
	/*
	auto mapNode = dynamic_cast<PG_DisaMapNode*>(gameScene->findFirstInTree(PG_Search::byName("N_DisaMapNode")));
	if (const auto geo = PG_DisaFileInfo(file))
		mapNode->openGEO(geo);
	*/
	m_center = item;
}

void RootScene::openMPD(const PG_File& file) {
	reset();
	auto rootItem = dynamic_cast<PG_GuiItem*>(getRoot()->findFirstInTree(PG_Search::byName("rootItem")));
	auto toolbar = dynamic_cast<PG_GuiItem*>(getRoot()->findFirstInTree(PG_Search::byName("toolbar")));
	if (!rootItem || !toolbar) return;


	auto [item, gameScene] = createMapScene(rootItem);
	item->getAnchor(PG_Anchor::Top)->setAnchor(toolbar->getAnchor(PG_Anchor::Bottom));
	item->getAnchor(PG_Anchor::Left)->setAnchor(rootItem->getAnchor(PG_Anchor::Left));
	item->getAnchor(PG_Anchor::Right)->setAnchor(rootItem->getAnchor(PG_Anchor::Right));
	item->getAnchor(PG_Anchor::Bottom)->setAnchor(rootItem->getAnchor(PG_Anchor::Bottom));
	/*
	auto mapNode = dynamic_cast<PG_DisaMapNode*>(gameScene->findFirstInTree(PG_Search::byName("N_DisaMapNode")));

	const auto mpd = PG_DisaFileInfo(file);
	const auto mppFile = file.replaceFileExtension("mpp");
	const auto mppSD = PG_DisaFileInfo(mppFile);
	const auto mppHD = PG_DisaFileInfo(mppFile.replaceFileName("NEW_" + file.getName()));
	if (mpd && mppSD) {
		mapNode->openMPD(mpd, mppSD, mppHD);
	}
	*/
	m_center = item;
}

void RootScene::setupMapView() {
	clearCenter();

	auto item = PG_new<PG_GuiRenderItem>("GridRenderItem", rootItem);
	m_center = item;
	auto sceneRenderer = PG_new<PG_SceneRenderer>("GameSceneRenderer", true, 2, item);
	auto gameScene = PG_new<PG_DisaMapScene>("GameScene", sceneRenderer);

	gameScene->setModel(std::dynamic_pointer_cast<PG_DisaMPDContainer>(m_mainModel->getCurrent()));
	connect(this, &RootScene::cleanUp, gameScene, [gameScene] {
		gameScene->setModel(nullptr);
		});

	/* The asset window and the inspector announce these to nobody in particular, the scene showing the
	 * map is who they mean. Object mode is what used to own the two, so they stay off outside it. */
	const auto inObjectMode = [gameScene] {
		auto* editor = gameScene->getModule<PG_DisaEditorModule>();
		return editor && editor->getMode() == PG_DisaEditorModule::Mode_Object;
		};
	connect(m_mainModel, &PG_DisaMapEditorMainModel::addModelPressed, gameScene, [gameScene, inObjectMode](int index) {
		if (inObjectMode()) gameScene->addModelAtViewCenter(index);
		});
	connect(m_mainModel, &PG_DisaMapEditorMainModel::dublicateModelPressed, gameScene, [gameScene, inObjectMode](int index) {
		if (inObjectMode()) gameScene->duplicateObject(index);
		});

	Inspector::setup(gameScene, item);

	PG_GuiItem::anchor(item, PG_Anchor::Top, toolbar, PG_Anchor::Bottom);
	PG_GuiItem::anchor(item, PG_Anchor::Left, rootItem, PG_Anchor::Left);
	PG_GuiItem::anchor(item, PG_Anchor::Right, rootItem, PG_Anchor::Right);
	PG_GuiItem::anchor(item, PG_Anchor::Bottom, rootItem, PG_Anchor::Bottom);
}

void RootScene::setupGridView() {
	clearCenter();

	//second scene
	constexpr glm::ivec2 smallGrid = glm::ivec2(200, 140);
	constexpr glm::ivec2 normalGrid = glm::ivec2(400, 270);
	constexpr glm::ivec2 bigGrid = glm::ivec2(800, 540);
	PG_GuiItem* gridWrapper = PG_new<PG_GuiItem>("grid", rootItem);
	PG_GuiItem::anchor(gridWrapper, PG_Anchor::Top, toolbar, PG_Anchor::Bottom);
	PG_GuiItem::anchor(gridWrapper, PG_Anchor::Left, rootItem, PG_Anchor::Left);
	PG_GuiItem::anchor(gridWrapper, PG_Anchor::Right, rootItem, PG_Anchor::Right);
	PG_GuiItem::anchor(gridWrapper, PG_Anchor::Bottom, rootItem, PG_Anchor::Bottom);
	gridWrapper->moveFront();
	m_center = gridWrapper;

	auto gridView = PG_new<PG_GuiGridView>("grid", gridWrapper);

	gridView->fill(gridWrapper);
	gridView->setClip(true);
	gridView->setItem(PG_GuiGridView::getDefaultCreateFunc<PG_DisaGridTile>());
	gridView->setItemUpdate(PG_GuiGridView::getDefaultUpdateFunc<PG_DisaGridTile>());
	gridView->setCellSize(normalGrid);

	gridView->setModel(m_mainModel->getGridModel());

	PG_GuiScrollBar* scroll = PG_new<PG_GuiScrollBar>("ScrollBar", gridWrapper);
	scroll->getAnchor(PG_Anchor::Top)->setAnchor(gridWrapper->getAnchor(PG_Anchor::Top));
	scroll->getAnchor(PG_Anchor::Bottom)->setAnchor(gridWrapper->getAnchor(PG_Anchor::Bottom));
	scroll->getAnchor(PG_Anchor::Right)->setAnchor(gridWrapper->getAnchor(PG_Anchor::Right));
	scroll->setSize(glm::vec2(10.0f, 0.0f));
	scroll->setScrollBarColors(PG_Gui::ColorSet{ blueColor, blueColor * 1.1f , blueColor * 1.1f });
	if (auto rec = dynamic_cast<PG_GuiRectangle*>(scroll->getHandle())) {
		rec->setMaterial("M_GuiRoundedRectangleAll5");
	}

	gridView->setupVerticalScrollBar(scroll);

	PG_GuiItem* container = PG_new<PG_GuiItem>("NavWrapper", gridWrapper);
	container->setPosition(glm::vec2(0, -20));
	container->setSize(glm::vec2(39.f * 3.0f, 30.f));
	container->getAnchor(PG_Anchor::HorizontalCenter)->setAnchor(gridView->getAnchor(PG_Anchor::HorizontalCenter));
	container->getAnchor(PG_Anchor::Bottom)->setAnchor(gridView->getAnchor(PG_Anchor::Bottom));


	auto small = PG_new<PG_GuiToggleButton>("ToggleSmallButton", glm::vec2(0.f, 0.f), glm::vec2(39.f, 30.f), container);
	small->setToggled(false);
	small->setIcon("gui/icon_smallTiles.png");
	small->setBackground("M_GuiToolButtonStart");
	small->getAnchor(PG_Anchor::Top)->setAnchor(container->getAnchor(PG_Anchor::Top));
	small->getAnchor(PG_Anchor::Left)->setAnchor(container->getAnchor(PG_Anchor::Left));
	small->setTooltipText("Small");
	small->setAutoToggle(false);

	auto normal = PG_new<PG_GuiToggleButton>("ToggleNormalButton", glm::vec2(0.f, 0.f), glm::vec2(39.f, 30.f), container);
	normal->setToggled(true);
	normal->setIcon("gui/icon_normalTiles.png");
	normal->setBackground("M_GuiToolButton");
	normal->getAnchor(PG_Anchor::Top)->setAnchor(container->getAnchor(PG_Anchor::Top));
	normal->getAnchor(PG_Anchor::Left)->setAnchor(small->getAnchor(PG_Anchor::Right));
	normal->setTooltipText("Normal");
	normal->setAutoToggle(false);

	auto big = PG_new<PG_GuiToggleButton>("ToggleBigButton", glm::vec2(0.f, 0.f), glm::vec2(39.f, 30.f), container);
	big->setToggled(false);
	big->setIcon("gui/icon_bigTiles.png");
	big->setBackground("M_GuiToolButtonEnd");
	big->getAnchor(PG_Anchor::Top)->setAnchor(container->getAnchor(PG_Anchor::Top));
	big->getAnchor(PG_Anchor::Right)->setAnchor(container->getAnchor(PG_Anchor::Right));
	big->setTooltipText("Big");
	big->setAutoToggle(false);

	connect(small, &PG_GuiToggleButton::clicked, gridView, [small, normal, big, grid = gridView, smallGrid] {
		small->setToggled(true);
		normal->setToggled(false);
		big->setToggled(false);
		grid->setCellSize(smallGrid);
		});

	connect(normal, &PG_GuiToggleButton::clicked, gridView, [small, normal, big, grid = gridView, normalGrid] {
		small->setToggled(false);
		normal->setToggled(true);
		big->setToggled(false);
		grid->setCellSize(normalGrid);
		});

	connect(big, &PG_GuiToggleButton::clicked, gridView, [small, normal, big, grid = gridView, bigGrid] {
		small->setToggled(false);
		normal->setToggled(false);
		big->setToggled(true);
		grid->setCellSize(bigGrid);

		});
}

void RootScene::setupCustomCampaignSettingView() {
	clearCenter();

	PG_Object* currObject = m_mainModel->getCurrentObject().get();
	if (!currObject) return;
	if (auto prop = currObject->getPropertyValue("campaignJson")) {
		currObject = prop.get<PG_Object*>();
	}
	else return;
	

	PG_GuiColumn* column = PG_new<PG_GuiColumn>("PG_GuiColumn", rootItem);
	PG_GuiItem::anchor(column, PG_Anchor::Top, toolbar, PG_Anchor::Bottom);
	PG_GuiItem::anchor(column, PG_Anchor::Left, rootItem, PG_Anchor::Left);
	PG_GuiItem::anchor(column, PG_Anchor::Right, rootItem, PG_Anchor::Right);
	PG_GuiItem::anchor(column, PG_Anchor::Bottom, rootItem, PG_Anchor::Bottom);
	column->setMargin({ 5.f, 0.f, 22.f, 22.f });

	connect(column, &PG_GuiItem::afterGuiUpdate, column, [column] {
		glm::vec2 size = column->getParent<PG_GuiItem>()->getSize();
		float scale = std::min(size.x, size.y) / 800.f;
		scale = glm::clamp(scale, 1.f, 2.f);
		column->setScale(glm::vec3(scale));
		}, PG_ConnectionType_Event);

	m_center = column;

	auto textCreator = [](const std::string& name, PG_GuiItem* root, float fontSize = 17.f) {
		auto item = PG_new<PG_GuiText>("Text", glm::vec2(0.f), glm::vec2(323.f, fontSize), root);
		item->setText(name);
		item->setFontSize(fontSize);
		item->setColor(blueColor);
		return item;
		};

	auto textFieldCreator = [](const std::string& name, PG_GuiItem* root) {
		auto item = PG_new<PG_GuiTextField>(name, glm::vec2(0.f), glm::vec2(323.f, 35.f), root);
		item->setPlaceholderText(name);
		return item;
		};

	textCreator("Settings", column, 32.f);
	textCreator("Manage your general mod settings", column);

	auto* devider = PG_new<PG_GuiLines>("Devider", glm::vec2(0.f), glm::vec2(0.f, 10.f), column);
	devider->makeLineX();
	PG_GuiItem::anchor(devider, PG_Anchor::Left, column, PG_Anchor::Left);
	PG_GuiItem::anchor(devider, PG_Anchor::Right, column, PG_Anchor::Right);
	devider->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 12.f);
	devider->setColor(backgroundDecoColor);


	auto* row = PG_new<PG_GuiRow>("Row", glm::vec2(0.f), glm::vec2(323.f, 380.f), column);
	PG_GuiItem::anchor(row, PG_Anchor::Left, column, PG_Anchor::Left);
	PG_GuiItem::anchor(row, PG_Anchor::Right, column, PG_Anchor::Right);

	auto* column1 = PG_new<PG_GuiColumn>("column1", glm::vec2(0.f), glm::vec2(323.f, 380.f), row);
	auto* column2 = PG_new<PG_GuiColumn>("column2", glm::vec2(0.f), glm::vec2(323.f, 380.f), row);

	textCreator("Name", column1);
	auto* item = PG_Gui::createGuiElementV2<PG_GuiTextField>(column1, currObject, "Name", "name", "text", textFieldCreator);
	item->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 22.f);

	textCreator("Author", column1);
	item = PG_Gui::createGuiElementV2<PG_GuiTextField>(column1, currObject, "Author", "author", "text", textFieldCreator);
	item->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 22.f);

	textCreator("Description", column1);
	item = PG_Gui::createGuiElementV2<PG_GuiTextField>(column1, currObject, "Description", "description", "text", textFieldCreator);
	item->setHeight(170.f);
	item->makeMultiline();
	item->getTextItem()->setMaxChars(512);
	item->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 22.f);


	auto selectorCreator = [currObject](const std::string& name, PG_GuiItem* root) {
		if (auto prop = currObject->getPropertyValue("rootModel")) {
			return PG_DisaItemSelector::createImageSelector(name, root, prop.get<PG_ListModelBase*>());
		}
		return PG_DisaItemSelector::createImageSelector(name, root, nullptr);
	};

	textCreator("Preview Image", column2);
	auto* itemSelector = PG_Gui::createGuiElementV2<PG_DisaItemSelector>(column2, currObject, "PreviewSelector", "previewImgPath", "name", selectorCreator);
	itemSelector->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 22.f);

	textCreator("Background Image", column2);
	itemSelector = PG_Gui::createGuiElementV2<PG_DisaItemSelector>(column2, currObject, "BackgroundSelector", "backgroundImgPath", "name", selectorCreator);
}

void RootScene::setupCustomCampaignPlanView() {
	clearCenter();

	PG_Object* currObject = m_mainModel->getCurrentObject().get();
	if (!currObject) return;
	if (auto prop = currObject->getPropertyValue("campaignJson")) {
		currObject = prop.get<PG_Object*>();
	}
	else return;


	PG_GuiColumn* column = PG_new<PG_GuiColumn>("GuiColumnCampaignPlan", rootItem);
	PG_GuiItem::anchor(column, PG_Anchor::Top, toolbar, PG_Anchor::Bottom);
	PG_GuiItem::anchor(column, PG_Anchor::Left, rootItem, PG_Anchor::Left);
	PG_GuiItem::anchor(column, PG_Anchor::Right, rootItem, PG_Anchor::Right);
	PG_GuiItem::anchor(column, PG_Anchor::Bottom, rootItem, PG_Anchor::Bottom);
	column->setMargin({ 5.f, 0.f, 22.f, 22.f });

	connect(column, &PG_GuiItem::afterGuiUpdate, column, [column] {
		glm::vec2 size = column->getParent<PG_GuiItem>()->getSize();
		float scale = std::min(size.x, size.y) / 800.f;
		scale = glm::clamp(scale, 1.f, 2.f);
		column->setScale(glm::vec3(scale));
		}, PG_ConnectionType_Event);

	m_center = column;

	auto textCreator = [](const std::string& name, PG_GuiItem* root, float fontSize = 17.f) {
		auto item = PG_new<PG_GuiText>("Text", glm::vec2(0.f), glm::vec2(323.f, fontSize), root);
		item->setText(name);
		item->setFontSize(fontSize);
		item->setColor(blueColor);
		return item;
		};

	textCreator("Campaign", column, 32.f);
	auto* text = textCreator("Manage your playable areas and dungeons", column);
	text->setWidth(400.f);


	auto* devider = PG_new<PG_GuiLines>("Devider", glm::vec2(0.f), glm::vec2(0.f, 10.f), column);
	devider->makeLineX();
	PG_GuiItem::anchor(devider, PG_Anchor::Left, column, PG_Anchor::Left);
	PG_GuiItem::anchor(devider, PG_Anchor::Right, column, PG_Anchor::Right);
	devider->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 12.f);
	devider->setLayoutProperty(PG_GuiItem::LayoutProperty_MinSize, glm::vec2(770.f, 10.f));
	devider->setLayoutProperty(PG_GuiItem::LayoutProperty_MaxSize, glm::vec2(970.f, 10.f));
	devider->setColor(backgroundDecoColor);

	auto flowView = PG_new<PG_GuiFlowView>("GuiFlowViewAreas", column);
	PG_GuiItem::anchor(flowView, PG_Anchor::Left, column, PG_Anchor::Left);
	PG_GuiItem::anchor(flowView, PG_Anchor::Right, column, PG_Anchor::Right);

	flowView->setItem(PG_GuiFlowView::getDefaultCreateFunc<PG_DisaCustomCampaignArea>());
	flowView->setItemUpdate(PG_GuiFlowView::getDefaultUpdateFunc<PG_DisaCustomCampaignArea>());
	flowView->setFlowMode(PG_GuiFlowView::ColumnFill);
	flowView->setSpacing(15.f);
	flowView->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	flowView->setLayoutProperty(PG_GuiItem::LayoutProperty_MinSize, glm::vec2(770.f, 0));
	flowView->setLayoutProperty(PG_GuiItem::LayoutProperty_MaxSize, glm::vec2(970.f, std::numeric_limits<float>::max()));
	flowView->setUpdateOnItemChanged(true);
	flowView->setClip(true);


	auto addButton = PG_new<PG_GuiButton>("AddButton", glm::vec2(-5.0f, 10.f), glm::vec2(39.f, 30.f), column);
	addButton->setIcon("gui/icon_add.png");
	addButton->setBackground("M_GuiRoundedRectangleAll5");
	addButton->setTooltipText("Add Area");
	addButton->setPreventCulling(true);
	addButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Ignore, true);
	PG_GuiItem::anchor(addButton, PG_Anchor::Right, flowView, PG_Anchor::Right);
	PG_GuiItem::anchor(addButton, PG_Anchor::Top, column, PG_Anchor::Top);

	connect(addButton, &PG_GuiButton::clicked, currObject, currObject->getFunction("void addArea()"));

	if (auto prop = currObject->getProperty("areas")) {
		flowView->setModel(currObject->getPropertyValue(prop).get<PG_ListModelBase*>());
	}
}

void RootScene::openEXE(const PG_File& file) {
	reset();

	auto rootItem = dynamic_cast<PG_GuiItem*>(getRoot()->findFirstInTree(PG_Search::byName("rootItem")));
	auto toolbar = dynamic_cast<PG_GuiItem*>(getRoot()->findFirstInTree(PG_Search::byName("toolbar")));
	if (!rootItem || !toolbar) return;
	m_mainModel->openExe();
}

template<typename GuiType, typename CreationFunc>
GuiType* createGuiToggleButtonModeElement(PG_GuiItem* root, PG_Object* wrapper, const std::string& name, const std::string& propName, const std::string& guiPropName, int toggleValue, CreationFunc&& createFunc) {

	GuiType* guiItem = createFunc(name, root);

	PG_Property guiProp = guiItem->getProperty(guiPropName);
	assert(guiProp);
	PG_Property selectionProp = wrapper->getProperty(propName);
	if (!selectionProp) return guiItem;

	auto isToggledFunc = [wrapper, toggleValue, selectionProp]() {
		return wrapper->getPropertyValue(selectionProp).get<int>() == toggleValue;
		};

	guiItem->setPropertyValue(guiProp, PG_Any::from(isToggledFunc()));

	PG_Object::connect(wrapper, selectionProp, guiItem, [guiItem, guiProp, isToggledFunc]() {
		guiItem->setPropertyValue(guiProp, PG_Any::from(isToggledFunc()));
		});

	if (guiProp.getChangedFuncPtr()) {
		PG_Object::connect(guiItem, guiProp, wrapper, [wrapper, selectionProp, guiItem, guiProp, toggleValue]() {
			if (guiItem->getPropertyValue(guiProp).get<bool>()) {
				wrapper->setPropertyValue(selectionProp, PG_Any::from(toggleValue));
			}
		});
	}

	return guiItem;
}

void RootScene::setupToolbarGeneric() {
	toolbarRow->clearGuiChildren();
	auto backButton = PG_new<PG_GuiButton>("BackButton", glm::vec2(0.0f), glm::vec2(39.f, 30.f), toolbarRow);
	backButton->setIcon("gui/icon_back.png");
	backButton->setBackground("M_GuiRoundedRectangleAll5");
	backButton->setTooltipText("Back");
	backButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);
	backButton->setVisible(m_mainModel->getUiStackSize() > 1);

	connect(backButton, &PG_GuiButton::clicked, m_mainModel, &PG_DisaMapEditorMainModel::back, PG_ConnectionType_Event);

	auto currObject = m_mainModel->getCurrentObject();
	if (!currObject) return;

	{
		auto saveFunc = currObject->getFunction("void saveToFile()");
		auto isModifiedProp = currObject->getProperty("isModified");
		if (saveFunc && isModifiedProp) {
			auto importBtn = PG_new<PG_GuiButton>("SaveButton", glm::vec2(0.0f), glm::vec2(39.f, 30.f), toolbarRow);
			importBtn->setIcon("gui/icon_save.png");
			importBtn->setBackground("M_GuiRoundedRectangleAll5");
			importBtn->setTooltipText("Save");
			importBtn->setEnabled(currObject->getPropertyValue(isModifiedProp).get<bool>());
			connect(currObject.get(), isModifiedProp, importBtn, [importBtn, obj = currObject.get(), isModifiedProp]() {
				importBtn->setEnabled(obj->getPropertyValue(isModifiedProp).get<bool>());
				});

			connect(importBtn, &PG_GuiButton::clicked, currObject.get(), [obj = currObject.get(), saveFunc] {
				saveFunc.callArgs(obj);
				});
		}
	}
	{
		auto func = currObject->getFunction("void addNewCampaign(const std::string&,bool*)");
		if (func) {
			auto importBtn = PG_new<PG_GuiButton>("AddButton", glm::vec2(0.0f), glm::vec2(39.f, 30.f), toolbarRow);
			importBtn->setIcon("gui/icon_add.png");
			importBtn->setBackground("M_GuiRoundedRectangleAll5");
			importBtn->setTooltipText("Add new campaign");

			connect(importBtn, &PG_GuiButton::clicked, currObject.get(), [this, obj = currObject.get(), func] {
				auto* popup = PG_PopUpDialog::createNewEntry(rootItem, "Create new campaign", "Please enter a name for your campaign!");

				connect(popup, &PG_PopUpDialog::confirmClickedPath, this, [this, obj, func](const std::string& str) {
					if (str.empty()) return;
					bool success = false;
					func.callArgs(obj, str, &success);
					if (success) return;
					//show error dialog
					auto* popup = PG_PopUpDialog::createError(rootItem, "Error", "Failed to create a new campaign! See terminal for more info.");
					popup->open();
					});
				connect(popup, &PG_PopUpDialog::cancelClicked, popup, &PG_PopUpDialog::close);
				connect(popup, &PG_PopUpDialog::confirmClicked, popup, &PG_PopUpDialog::close);
				popup->open();
				});

		}
	}

	{
		auto func = currObject->getFunction("void addNewDungeon(const std::string&,const PG_DisaFileEntry&,const PG_DisaFileEntry&,int,int,int)");
		if (func) {
			auto importBtn = PG_new<PG_GuiButton>("AddButton", glm::vec2(0.0f), glm::vec2(39.f, 30.f), toolbarRow);
			importBtn->setIcon("gui/icon_add.png");
			importBtn->setBackground("M_GuiRoundedRectangleAll5");
			importBtn->setTooltipText("Add new dungeon");

			connect(importBtn, &PG_GuiButton::clicked, currObject.get(), [this, obj = currObject.get(), func] {
				auto item = PG_new<PG_DisaAddDungeonDialog>("PG_DisaAddDungeonDialog", rootItem);
				item->setModel(obj, func);
				item->open();
				});
		}
	}

	if (auto func = currObject->getFunction("void importFile(const PG_File&)")) {
		auto importBtn = PG_new<PG_GuiButton>("ImportButton", glm::vec2(0.0f), glm::vec2(39.f, 30.f), toolbarRow);
		importBtn->setIcon("gui/icon_folder.png");
		importBtn->setBackground("M_GuiRoundedRectangleAll5");
		importBtn->setTooltipText("Import file");

		connect(importBtn, &PG_GuiButton::clicked, currObject.get(), [obj = currObject.get(), func] {
			if (PG_File result = PG_PopUpDialog::selectFile("png,mpd,tscript,script")) {
				func.callArgs(obj, result);
			}
		});
	}

	if (auto prop = currObject->getProperty("subUiState")) {
		auto icons = m_mainModel->getCurrent()->getSubModeIcons();
		int i = 0;
		for (const auto& icon : icons) {
			auto modeBtn = createGuiToggleButtonModeElement<PG_GuiToggleButton>(toolbarRow, currObject.get(), "ModeBtn", "subUiState", "toggled", icon.subMode,
				[&i, s = icons.size(), &icon](const std::string& name, PG_GuiItem* root) {
					auto toggleButton = PG_new<PG_GuiToggleButton>(name, glm::vec2(0.f), glm::vec2(39.f, 30.f), root);
					toggleButton->setTooltipText(icon.name);
					toggleButton->setIcon(icon.icon);
					//toggleButton->setText(icon.name);
					toggleButton->setBackground(i == 0 ? "M_GuiToolButtonStart" : i != s - 1 ? "M_GuiToolButton" : "M_GuiToolButtonEnd");
					toggleButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);
					toggleButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::HorizontalCenter);
					return toggleButton;
				});
			++i;
		}
	}


	if (auto gridModel = m_mainModel->getGridModel()) {
		if (auto prop = gridModel->getProperty("searchText")) {

			PG_DisaSearchField* searchField = PG_new<PG_DisaSearchField>("searchField", glm::vec2(0.f), glm::vec2(300.f, 30.f), toolbarRow);
			searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::Right);
			searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
			searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);
			searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_MaxSize, glm::vec2(300.f, 30.f));
			searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_MinSize, glm::vec2(80.f, 30.f));

			connect(searchField, searchField->getProperty("text"), gridModel, prop);
		}
	}
}

#include <Windows.h>

void RootScene::setupToolbarMapView() {
	toolbarRow->clearGuiChildren();
	auto* mapNode = getGameScene(m_center);

	auto backButton = PG_new<PG_GuiButton>("BackButton", glm::vec2(40.f, 0.0f), glm::vec2(39.f, 30.f), toolbarRow);
	backButton->setIcon("gui/icon_back.png");
	backButton->setBackground("M_GuiRoundedRectangleAll5");
	backButton->setTooltipText("Back");
	backButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);

	connect(backButton, &PG_GuiButton::clicked, m_mainModel, &PG_DisaMapEditorMainModel::back, PG_ConnectionType_Event);


	auto saveButton = PG_new<PG_GuiButton>("SaveButton", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
	saveButton->setIcon("gui/icon_save.png");
	saveButton->setBackground("M_GuiRoundedRectangleAll5");
	saveButton->setTooltipText("Save");
	//saveButton->setEnabled(false);

	auto* currObject = dynamic_cast<PG_Object*>(m_mainModel->getCurrent().get());

	if (auto isModifiedProp = currObject->getProperty("isModified")) {
		saveButton->setEnabled(currObject->getPropertyValue(isModifiedProp).get<bool>());
		connect(currObject, isModifiedProp, saveButton, [saveButton, currObject, isModifiedProp]() {
			saveButton->setEnabled(currObject->getPropertyValue(isModifiedProp).get<bool>());
			});
	}

	connect(saveButton, &PG_GuiButton::clicked, this, [this]() {
		auto* obj = dynamic_cast<PG_Object*>(m_mainModel->getCurrent().get());
		if (auto func = obj->getFunction("bool save()")) {
			func.callArgs(obj);
			return;
		}
		if (auto func = obj->getFunction("bool saveFile()")) {
			func.callArgs(obj);
			return;
		}
	});

	PG_new<PG_GuiItem>("Spacer", glm::vec2(0.f, 0.f), glm::vec2(30.f, 30.f), toolbarRow);

	auto axisButton = PG_new<PG_GuiToggleButton>("AxisButton", glm::vec2(40.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
	axisButton->setToggled(true);
	axisButton->setIcon("gui/icon_axis.png");
	axisButton->setBackground("M_GuiToolButtonStart");
	axisButton->setTooltipText("Hide/Show axis");
	axisButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);

	connect(axisButton, &PG_GuiToggleButton::toggleChanged, this, [this](bool toggled) {
		if (auto* mapNode = getGameScene(m_center)) mapNode->setAxisVisible(toggled);
		});

	/*
	auto hiddenButton = PG_new<PG_GuiToggleButton>("HiddenButton", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
	hiddenButton->setToggled(false);
	hiddenButton->setIcon("gui/icon_eye.png");
	hiddenButton->setBackground("M_GuiToolButton");
	hiddenButton->setTooltipText("Hide/Show hidden models");

	connect(hiddenButton, &PG_GuiToggleButton::toggleChanged, this, [this](bool toggled) {
		if (auto* mapNode = getGameScene(m_center)) mapNode->setShowHiddenModel(toggled);
		});
	*/
	

	auto actorButton = PG_new<PG_GuiToggleButton>("ActorButton", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
	actorButton->setIcon("gui/icon_actor.png");
	actorButton->setBackground("M_GuiToolButton");
	actorButton->setTooltipText("Hide/Show actor positions");
	actorButton->setToggled(mapNode->isActorsVisible());
	actorButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);

	connect(actorButton, &PG_GuiToggleButton::toggleChanged, this, [this](bool toggled) {
		if (auto* mapNode = getGameScene(m_center)) mapNode->setActorsVisible(toggled);
		});

	auto geoPanelButton = PG_new<PG_GuiToggleButton>("GeoPanelButton", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
	geoPanelButton->setIcon("gui/disa/icon_geoPanel.png");
	geoPanelButton->setBackground("M_GuiToolButton");
	geoPanelButton->setTooltipText("Hide/Show GeoPanels");
	geoPanelButton->setToggled(mapNode->isGeoPanelsVisible());
	geoPanelButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);

	connect(geoPanelButton, &PG_GuiToggleButton::toggleChanged, this, [this](bool toggled) {
		if (auto* mapNode = getGameScene(m_center)) mapNode->setGeoPanelsVisible(toggled);
		});

	auto hdButton = PG_new<PG_GuiToggleButton>("HDButton", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
	hdButton->setToggled(false);
	hdButton->setIcon("gui/disa/icon_HD.png");
	hdButton->setBackground("M_GuiToolButtonEnd");
	hdButton->setTooltipText("Toggle HD textures");
	hdButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);

	connect(hdButton, &PG_GuiToggleButton::toggleChanged, this, [this](bool toggled) {
		if (auto* mapNode = getGameScene(m_center)) mapNode->setHDMode(toggled);
		});

	//export/import
	/*
	auto exportGLTFButton = PG_new<PG_GuiButton>("ExportGltfButton", glm::vec2(20.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
	exportGLTFButton->setIcon("gui/icon_export.png");
	exportGLTFButton->setBackground("M_GuiToolButtonStart");
	exportGLTFButton->setTooltipText("Export GLTF");

	connect(exportGLTFButton, &PG_GuiButton::clicked, this, [this]() {
		if (auto* mapNode = getGameScene(m_center))
			mapNode->exportGLTFClicked();
		});

	auto exportJsonButton = PG_new<PG_GuiButton>("ExportJsonButton", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
	exportJsonButton->setIcon("gui/icon_json_export.png");
	exportJsonButton->setBackground("M_GuiToolButton");
	exportJsonButton->setTooltipText("Export Json");

	connect(exportJsonButton, &PG_GuiButton::clicked, this, [this]() {
		if (PG_DisaMapListModel* model = dynamic_cast<PG_DisaMapListModel*>(m_centerLocation.getModelRoot())) {
			model->exportJSON(m_centerLocation);
		}
		});

	auto importJsonButton = PG_new<PG_GuiToggleButton>("ImportJsonButton", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
	importJsonButton->setIcon("gui/icon_json_import.png");
	importJsonButton->setBackground("M_GuiToolButtonEnd");
	importJsonButton->setTooltipText("Import Json");
	connect(importJsonButton, &PG_GuiButton::clicked, this, [this]() {
		if (PG_DisaMapListModel* model = dynamic_cast<PG_DisaMapListModel*>(m_centerLocation.getModelRoot())) {
			model->importJSON(m_centerLocation);
			//postCallEvent(&RootScene::reloadMap);
		}
		});
	*/

	PG_new<PG_GuiItem>("Spacer", glm::vec2(0.f, 0.f), glm::vec2(10.f, 30.f), toolbarRow);

	//EDIT
	auto editToggleButton = PG_new<PG_GuiToggleButton>("EditButton", glm::vec2(3.f, 0.f), glm::vec2(100.f, 30.f), toolbarRow);
	editToggleButton->setIcon("gui/icon_edit.png");
	editToggleButton->getIcon()->setPosition(glm::vec2(3.f, 0.f));
	editToggleButton->setText("Edit");
	editToggleButton->setBackground("M_GuiToolButtonStart");
	editToggleButton->setTooltipText("Enable editting");
	editToggleButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);
	//editToggleButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::HorizontalCenter); //TODO fix button text position

	connect(editToggleButton, &PG_GuiToggleButton::toggleChanged, this, [this](bool toggled) {
		if (auto* mapNode = getGameScene(m_center)) mapNode->setEnableEditing(toggled);
	});

	auto playTestButton = PG_new<PG_GuiButton>("PlayTestButton", glm::vec2(0.f), glm::vec2(39.f, 30.f), toolbarRow);
	playTestButton->setIcon("gui/icon_play.png");
	playTestButton->setBackground("M_GuiToolButtonEnd");
	playTestButton->setTooltipText("Play test");
	playTestButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);
	//playTestButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::HorizontalCenter);

	struct CommunicationObject : public PG_Object {
		using PG_Object::PG_Object;

		void init() {
			customMessageId = RegisterWindowMessage(TEXT("START_PLAY_TEST"));
			if (customMessageId == 0) {
				OUT_ERROR("Failed to register custom message");
				return;
			}
		}

		void sendPlayTest() {
			// Find the target window by its title
			HWND targetWindow = FindWindow(NULL, TEXT("DISGAEA"));
			if (targetWindow == NULL) {
				OUT_ERROR("A running Disa executable not found!");
				return;
			}

			// Post the custom message to the target window
			if (!PostMessage(targetWindow, customMessageId, 0, 0)) {
				OUT_ERROR("Failed to send message");
				return;
			}
		}

		UINT customMessageId;
	};
	auto* com = findFirstChild<CommunicationObject>("CommunicationObject");
	if(!com) com = PG_new<CommunicationObject>("CommunicationObject", this);

	connect(playTestButton, &PG_GuiButton::clicked, this, [this] {
		if (auto* mapNode = getGameScene(m_center)) {
			
			mapNode->getMapModel()->saveMPD(PG_File(m_mainModel->getExePath().getDir() + "/mod/CustomCampaigns/debugMap.mpd"));

			auto* com = findFirstChild<CommunicationObject>("CommunicationObject");
			if (!com) com = PG_new<CommunicationObject>("CommunicationObject", this);

			com->sendPlayTest();
		}

		});

	auto assetsButton = PG_new<PG_GuiToggleButton>("OpenJsonButton", glm::vec2(20.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
	assetsButton->setIcon("gui/icon_model.png");
	assetsButton->setBackground("M_GuiRoundedRectangleAll5");
	assetsButton->setTooltipText("Assets");
	assetsButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::Right);

	connect(assetsButton, &PG_GuiToggleButton::toggleChanged, this, [this, assetsButton](bool toggled) {
		if (toggled && m_center) {
			PG_DisaMapScene* gameScene = m_center->findFirstChild<PG_SceneRenderer>("GameSceneRenderer")->findFirstChild<PG_DisaMapScene>("GameScene");
			if (!gameScene) return;
			

			auto* window = PG_new<PG_GuiWindowPopup>("AssetsWindow", this);
			window->setType(PG_GuiWindowPopup::Type_Utility);

			window->setCreator([assetModel = gameScene->getMapModel()->getAssets()](PG_GuiWindowPopupInstance* i) {
				i->setSize(glm::vec2(590.f, 500.f));
				auto w = PG_new<PG_DisaAssetsWindow>("DisaAssetsWindow", i);
				w->fill(i);
				w->setModel(assetModel);
				});
			connect(window, &PG_GuiWindowPopup::activeChanged, gameScene->getMapModel()->getAssets(), &PG_DisaAssetsModel::setEditEnabled);

			window->open();
			connect(window, &PG_GuiWindowPopup::activeChanged, assetsButton, [assetsButton](bool active) {
				if (auto* caller = dynamic_cast<PG_GuiWindowPopup*>(PG_Object::caller())) {
					assetsButton->setToggled(false);
					caller->deleteLater();
				}
				});
		}
		else if (auto window = findFirstChild<PG_GuiWindowPopup>("AssetsWindow"); window && !window->isDeleteLater()) {
			window->close();
		}
		});
}