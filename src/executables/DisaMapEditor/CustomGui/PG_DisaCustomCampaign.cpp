#include "PG_DisaCustomCampaign.h"

#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiRow.h>
#include <Scene/Gui/PG_GuiColumn.h>
#include <Scene/Gui/PG_GuiTextField.h>
#include <Scene/Gui/PG_GuiLines.h>
#include <Scene/Gui/PG_GuiListView.h>
#include <Scene/Gui/PG_GuiSpinBox.h>
#include <Scene/Gui/PG_GuiDropDown.h>
#include <Scene/Gui/PG_GuiCreateSelectionElement.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiToggleButton.h>
#include <Scene/Gui/PG_GuiFlowView.h>
#include <Scene/Gui/PG_GuiTextEdit.h>
#include <Scene/Model/PG_PropertyDynamicWrapper.h>
#include <Scene/Model/PG_ListModelFilter.h>
#include <CustomGui/PG_DisaItemSelector.h>
#include <CustomGui/PG_DisaSelectorDropDown.h>
#include <Models/PG_DisaMapEditorMainModel.h>
#include <Models/PG_CategoryFilterModel.h>
#include <Models/PG_UiStateCustomCampaignsModel.h>

constexpr glm::vec3 backgroundColor = glm::vec3(40.f / 255.f);
constexpr glm::vec3 backgroundDecoColor = glm::vec3(45.f / 255.f);
constexpr glm::vec3 blueColor = glm::vec3(133.f, 208.f, 255.f) / 255.f;
constexpr glm::vec3 blueColorDark = glm::vec3(25.f, 25.f, 30.f) / 255.f;
constexpr float campaignAreaBaseHeight = 200.f;
constexpr float dungeonHeight = 230.f + 32.f;


PG_GuiText* PG_DisaCustomCampaignBase::textCreator(const std::string& name, PG_GuiItem* root, float fontSize) {
	auto item = PG_new<PG_GuiText>("Text", glm::vec2(0.f), glm::vec2(323.f, fontSize), root);
	item->setText(name);
	item->setFontSize(fontSize);
	item->setColor(blueColor);
	return item;
}

PG_GuiTextField* PG_DisaCustomCampaignBase::textFieldCreator(const std::string& name, PG_GuiItem* root) {
	auto item = PG_new<PG_GuiTextField>(name, glm::vec2(0.f), glm::vec2(323.f, 35.f), root);
	item->setPlaceholderText(name);
	return item;
}

void PG_DisaCustomCampaignBase::setLocation(const PG_ListModelLocation& location) {
	if (m_location == location) return;
	m_location = location;
	updateModel();
}

void PG_DisaCustomCampaignBase::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.contains(PG_Identity_Item) || m_location != location;
	if (hasItem || identities.contains(PG_Identity_Location)) {
		m_location = location;
		updateModel();
	}
}

void PG_DisaCustomCampaignBase::callFunc(const std::string& funcName) const {
	for (auto* selection : m_model->getSelectionItems()) {
		if (auto func = selection->getFunction(funcName)) {
			func.callArgs(selection);
		}
	}
}


void PG_DisaCustomCampaignBase::updateModel() {
	if (!m_location) {
		m_model->clearSelectionItems();
		return;
	}

	auto item = m_location.getData(PG_Identity_Item).convert<PG_Object*>();
	if (!item) {
		m_model->clearSelectionItems();
		return;
	}
	m_model->setSelectionItems({ item });
}



void PG_DisaCustomCampaignDungeon::init() {
	PG_DisaCustomCampaignBase::init();

	m_model = PG_new<PG_PropertyDynamicWrapper>("Wrapper", this);
	m_model->registerProperty("name", PG_Any::from(std::string()));
	m_model->registerProperty("dungeon", PG_Any::from(std::string()));
	m_model->registerProperty("identifier", PG_Any::from(std::string()));
	m_model->registerProperty("description", PG_Any::from(std::string()));
	m_model->registerProperty("bonusRank", PG_Any::from<int>(0));
	m_model->registerProperty("bgm", PG_Any::from<int>(0));
	m_model->registerProperty("unlockCriteria", PG_Any::from<EDisaUnlockCriteria>(EDisaUnlockCriteria::DisaUnlockCriteria_PreviousCompleted),
		&PG_PropertyDynamicWrapper::getValueConvert<EDisaUnlockCriteria, int>, &PG_PropertyDynamicWrapper::setValueConvert<int, EDisaUnlockCriteria>);
	m_model->registerProperty("rootModel", PG_Any::from<PG_ListModelBase*>(nullptr));

	m_model->registerProperty("scriptIntro", PG_Any::from(std::string()));
	m_model->registerProperty("scriptIntroSkipStory", PG_Any::from(std::string()));
	m_model->registerProperty("scriptExit", PG_Any::from(std::string()));
	m_model->registerProperty("scriptExitSkipStory", PG_Any::from(std::string()));

	m_model->registerProperty("tabIndex", PG_Any::from<int>(0), true);

	setMaterial("M_GuiRoundedRectangleAll5");
	setColor(blueColorDark);
	setHeight(dungeonHeight);
	constexpr float itemBaseWidth = 210.f;


	auto* toolbar = PG_new<PG_GuiRow>("DungeonsRow", this);
	toolbar->setHeight(32);
	toolbar->setMargin(PG_Anchor::Left, 12.f);
	toolbar->setMargin(PG_Anchor::Top, 5.f);
	anchor(toolbar, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(toolbar, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(toolbar, PG_Anchor::Right, this, PG_Anchor::Right);

	{
		auto* tab0 = PG_new<PG_GuiToggleButton>("Info", toolbar);
		tab0->setText("Info");
		tab0->setSize(glm::vec2(100.f, 30.f));
		tab0->setBackground("M_GuiRoundedRectangleAll5");
		tab0->setAutoToggle(false);
		tab0->setToggled(true);

		PG_Object::connect(tab0, &PG_GuiButton::clicked, m_model, [m = m_model]() {
			m->setPropertyValue(m->getProperty("tabIndex"), 0);
		});
		PG_Object::connect(m_model, m_model->getProperty("tabIndex"), tab0, [m = m_model, tab0]() {
			tab0->setToggled(m->getPropertyValue(m->getProperty("tabIndex")).toInt() == 0);
		});

		auto* tab1 = PG_new<PG_GuiToggleButton>("Scripts", toolbar);
		tab1->setText("Scripts");
		tab1->setSize(glm::vec2(100.f, 30.f));
		tab1->setBackground("M_GuiRoundedRectangleAll5");
		tab1->setAutoToggle(false);

		PG_Object::connect(tab1, &PG_GuiButton::clicked, m_model, [m = m_model]() {
			m->setPropertyValue(m->getProperty("tabIndex"), 1);
		});
		PG_Object::connect(m_model, m_model->getProperty("tabIndex"), tab1, [m = m_model, tab1]() {
			tab1->setToggled(m->getPropertyValue(m->getProperty("tabIndex")).toInt() == 1);
		});
	}

	auto* row = PG_new<PG_GuiRow>("DungeonsRow", this);
	anchor(row, PG_Anchor::Top, toolbar, PG_Anchor::Bottom);
	anchor(row, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(row, PG_Anchor::Right, this, PG_Anchor::Right);
	anchor(row, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	row->setMargin(12.f);

	row->setVisible(m_model->getPropertyValue(m_model->getProperty("tabIndex")).toInt() == 0);
	PG_Object::connect(m_model, m_model->getProperty("tabIndex"), row, [m = m_model, row]() {
		row->setVisible(m->getPropertyValue(m->getProperty("tabIndex")).toInt() == 0);
		});

	{
		{ //Column1
			auto* column = PG_new<PG_GuiColumn>("Column1", row);
			PG_GuiItem::anchor(column, PG_Anchor::Top, row, PG_Anchor::Top);
			PG_GuiItem::anchor(column, PG_Anchor::Bottom, row, PG_Anchor::Bottom);
			column->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
			column->setLayoutProperty(PG_GuiItem::LayoutProperty_MinSize, glm::vec2(itemBaseWidth));
			column->setWidth(itemBaseWidth);

			textCreator("Name", column);
			auto* item = PG_Gui::createGuiElementV2<PG_GuiTextField>(column, m_model, "Name", "name", "text", textFieldCreator);
			anchor(item, PG_Anchor::Left, column, PG_Anchor::Left);
			anchor(item, PG_Anchor::Right, column, PG_Anchor::Right);
			item->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);

			textCreator("Description", column);
			item = PG_Gui::createGuiElementV2<PG_GuiTextField>(column, m_model, "Description", "description", "text", textFieldCreator);
			anchor(item, PG_Anchor::Left, column, PG_Anchor::Left);
			anchor(item, PG_Anchor::Right, column, PG_Anchor::Right);
			item->makeMultiline();
			item->setHeight(114.f);
			item->getTextItem()->setMaxChars(512);


		}

		{ //Column2
			auto* column = PG_new<PG_GuiColumn>("Column2", row);
			PG_GuiItem::anchor(column, PG_Anchor::Top, row, PG_Anchor::Top);
			PG_GuiItem::anchor(column, PG_Anchor::Bottom, row, PG_Anchor::Bottom);
			column->setWidth(itemBaseWidth);

			textCreator("Identifier", column);
			auto* item = PG_Gui::createGuiElementV2<PG_GuiTextField>(column, m_model, "Identifier", "identifier", "text", textFieldCreator);
			anchor(item, PG_Anchor::Left, column, PG_Anchor::Left);
			anchor(item, PG_Anchor::Right, column, PG_Anchor::Right);
			item->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);

			auto selectorCreator = [this](const std::string& name, PG_GuiItem* root) {
				auto guiItem = PG_DisaItemSelector::createDungeonSelector(name, root, nullptr);
				if (auto prop = m_model->getProperty("rootModel")) {
					PG_Object::connect(m_model, prop, guiItem, [this, guiItem, prop]() {
						dynamic_cast<PG_ListModelFilter*>(guiItem->getModel())->setModel(m_model->getPropertyValue(prop).get<PG_ListModelBase*>());
						});
				}

				return guiItem;
				};

			textCreator("Dungeon", column);
			auto* itemSelector = PG_Gui::createGuiElementV2<PG_DisaItemSelector>(column, m_model, "DungeonSelector", "dungeon", "name", selectorCreator);
			anchor(itemSelector, PG_Anchor::Left, column, PG_Anchor::Left);
			anchor(itemSelector, PG_Anchor::Right, column, PG_Anchor::Right);

		}

		{ //Column3
			auto* column = PG_new<PG_GuiColumn>("Column3", row);
			PG_GuiItem::anchor(column, PG_Anchor::Top, row, PG_Anchor::Top);
			PG_GuiItem::anchor(column, PG_Anchor::Bottom, row, PG_Anchor::Bottom);
			column->setWidth(itemBaseWidth);

			auto spinBoxCreator = [](const std::string& name, PG_GuiItem* root) {
				auto* item = PG_GuiSpinBox::createInt(name, glm::vec2(0.f), glm::vec2(itemBaseWidth, 35.f), 1, 99999, root);
				return item;
				};

			textCreator("Bonus Rank", column);
			auto* item = PG_Gui::createGuiElementV2<PG_GuiSpinBox>(column, m_model, "BonusRank", "bonusRank", "valueInt", spinBoxCreator);
			anchor(item, PG_Anchor::Left, column, PG_Anchor::Left);
			anchor(item, PG_Anchor::Right, column, PG_Anchor::Right);
			item->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);

			textCreator("BGM", column);
			item = PG_Gui::createGuiElementV2<PG_GuiSpinBox>(column, m_model, "BGM", "bgm", "valueInt", spinBoxCreator);
			anchor(item, PG_Anchor::Left, column, PG_Anchor::Left);
			anchor(item, PG_Anchor::Right, column, PG_Anchor::Right);
			item->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);

			textCreator("Unlock Criteria", column);
			auto dropDown = PG_Gui::createGuiElementV2<PG_GuiDropDown>(column, m_model, "unlockCriteria", "unlockCriteria", "index",
				[](const std::string& name, PG_GuiItem* root) {
					auto dropDown = PG_new<PG_GuiDropDown>(name, glm::vec2(0.f, 0.f), glm::vec2(itemBaseWidth, 32.f), root);
					//dropDown->setTooltipText("Unlock Criteria");
					auto model = PG_new<PG_ListModel<std::string>>(dropDown);

					model->set({
						{ "None" },
						{ "Previous Completed" },
						{ "Partial Completed" },
						});
					dropDown->setModel(model);
					return dropDown;
				});
			anchor(dropDown, PG_Anchor::Left, column, PG_Anchor::Left);
			anchor(dropDown, PG_Anchor::Right, column, PG_Anchor::Right);
		}

		{ //Column3
			auto* column = PG_new<PG_GuiColumn>("Column4", row);
			column->setWidth(39.f);
			column->setHeight(95.f);
			column->setSpacing(2.f);
			anchor(column, PG_Anchor::VerticalCenter, row, PG_Anchor::VerticalCenter);

			auto* button = PG_new<PG_GuiButton>("Delete", glm::vec2(0.0f), glm::vec2(39.f, 30.f), column);
			button->setBackground("M_GuiRoundedRectangleTop5");
			button->setIcon("I_icon_delete");
			button->setTooltipText("Delete");

			connect(button, &PG_GuiButton::clicked, this, [this] { callFunc("void remove()"); }, PG_ConnectionType_Event);

			button = PG_new<PG_GuiButton>("Up", glm::vec2(0.0f), glm::vec2(39.f, 30.f), column);
			button->setIcon("I_disa_icon_up");
			button->setTooltipText("Move Up");

			connect(button, &PG_GuiButton::clicked, this, [this] { callFunc("void moveUp()"); });

			button = PG_new<PG_GuiButton>("Down", glm::vec2(0.0f), glm::vec2(39.f, 30.f), column);
			button->setIcon("I_disa_icon_down");
			button->setBackground("M_GuiRoundedRectangleBottom5");
			button->setTooltipText("Move Down");

			connect(button, &PG_GuiButton::clicked, this, [this] { callFunc("void moveDown()"); });
		}
	}



	auto* rowScripts = PG_new<PG_GuiRow>("ScriptsRow", this);
	anchor(rowScripts, PG_Anchor::Top, toolbar, PG_Anchor::Bottom);
	anchor(rowScripts, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(rowScripts, PG_Anchor::Right, this, PG_Anchor::Right);
	anchor(rowScripts, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	rowScripts->setMargin(12.f);

	rowScripts->setVisible(m_model->getPropertyValue(m_model->getProperty("tabIndex")).toInt() == 1);
	PG_Object::connect(m_model, m_model->getProperty("tabIndex"), rowScripts, [m = m_model, rowScripts]() {
		rowScripts->setVisible(m->getPropertyValue(m->getProperty("tabIndex")).toInt() == 1);
		});


	PG_CategoryFilterModel* filterScriptModel = PG_new<PG_CategoryFilterModel>("FilterModel", this);
	filterScriptModel->setFilterCategory(PG_CategoryFilterModel::TalkScript);

	if (auto prop = m_model->getProperty("rootModel")) {
		PG_Object::connect(m_model, prop, filterScriptModel, [m = m_model, filterScriptModel, prop]() {
			filterScriptModel->setModel(m->getPropertyValue(prop).get<PG_ListModelBase*>());
			});
	}

	{
		auto* column = PG_new<PG_GuiColumn>("Column1", rowScripts);
		PG_GuiItem::anchor(column, PG_Anchor::Top, rowScripts, PG_Anchor::Top);
		PG_GuiItem::anchor(column, PG_Anchor::Bottom, rowScripts, PG_Anchor::Bottom);
		column->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
		column->setLayoutProperty(PG_GuiItem::LayoutProperty_MinSize, glm::vec2(itemBaseWidth));
		column->setWidth(itemBaseWidth);

		textCreator("Intro", column);
		auto* item = PG_new<PG_DisaSelectorDropDown>("Intro", column);
		item->setSize(glm::vec2(32.f));
		PG_Gui::connectGuiElementV2(m_model, "scriptIntro", item->getTextFiled(), "text");
		item->setModel(filterScriptModel);
		anchor(item, PG_Anchor::Left, column, PG_Anchor::Left);
		anchor(item, PG_Anchor::Right, column, PG_Anchor::Right);
		item->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);

		textCreator("Intro skip story", column);
		item = PG_new<PG_DisaSelectorDropDown>("IntroSkipStory", column);
		item->setSize(glm::vec2(32.f));
		PG_Gui::connectGuiElementV2(m_model, "scriptIntroSkipStory", item->getTextFiled(), "text");
		item->setModel(filterScriptModel);
		anchor(item, PG_Anchor::Left, column, PG_Anchor::Left);
		anchor(item, PG_Anchor::Right, column, PG_Anchor::Right);
		item->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);
	}

	{
		auto* column = PG_new<PG_GuiColumn>("Column2", rowScripts);
		PG_GuiItem::anchor(column, PG_Anchor::Top, rowScripts, PG_Anchor::Top);
		PG_GuiItem::anchor(column, PG_Anchor::Bottom, rowScripts, PG_Anchor::Bottom);
		column->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
		column->setLayoutProperty(PG_GuiItem::LayoutProperty_MinSize, glm::vec2(itemBaseWidth));
		column->setWidth(itemBaseWidth);

		textCreator("Exit", column);
		auto* item = PG_new<PG_DisaSelectorDropDown>("Exit", column);
		item->setSize(glm::vec2(32.f));
		PG_Gui::connectGuiElementV2(m_model, "scriptExit", item->getTextFiled(), "text");
		item->setModel(filterScriptModel);
		anchor(item, PG_Anchor::Left, column, PG_Anchor::Left);
		anchor(item, PG_Anchor::Right, column, PG_Anchor::Right);
		item->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);

		textCreator("Exit skip story", column);
		item = PG_new<PG_DisaSelectorDropDown>("ExitSkipStory", column);
		item->setSize(glm::vec2(32.f));
		PG_Gui::connectGuiElementV2(m_model, "scriptExitSkipStory", item->getTextFiled(), "text");
		item->setModel(filterScriptModel);
		anchor(item, PG_Anchor::Left, column, PG_Anchor::Left);
		anchor(item, PG_Anchor::Right, column, PG_Anchor::Right);
		item->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);
	}
}

void PG_DisaCustomCampaignArea::init() {
	PG_DisaCustomCampaignBase::init();
	setMaterial("M_GuiRoundedRectangleAll5");
	setColor(backgroundColor);
	setHeight(campaignAreaBaseHeight);

	m_model = PG_new<PG_PropertyDynamicWrapper>("Wrapper", this);
	m_model->registerProperty("name", PG_Any::from(std::string()));
	m_model->registerProperty("description", PG_Any::from(std::string()));
	m_model->registerProperty("backgroundImage", PG_Any::from(std::string()));
	m_model->registerProperty("rootModel", PG_Any::from<PG_ListModelBase*>(nullptr));
	m_model->registerProperty("dungeons", PG_Any::from<PG_ListModelBase*>(nullptr));

	PG_GuiColumn* columnRoot = PG_new<PG_GuiColumn>("ColumnMain", this);
	columnRoot->fill(this);
	columnRoot->setMargin({ 5.f, 5.f, 12.f, 12.f });

	 { //row 1
		auto* row1 = PG_new<PG_GuiRow>("Row", columnRoot);
		anchor(row1, PG_Anchor::Left, columnRoot, PG_Anchor::Left);
		anchor(row1, PG_Anchor::Right, columnRoot, PG_Anchor::Right);
		row1->setHeight(160.f);
		row1->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 0.f);

		auto* column11 = PG_new<PG_GuiColumn>("Column11", row1);
		column11->setSize(glm::vec2(323.f, 160.f));
		column11->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
		column11->setLayoutProperty(PG_GuiItem::LayoutProperty_MinSize, glm::vec2(323.f));

		textCreator("Name", column11);
		auto* item = PG_Gui::createGuiElementV2<PG_GuiTextField>(column11, m_model, "Name", "name", "text", textFieldCreator);
		anchor(item, PG_Anchor::Left, column11, PG_Anchor::Left);
		anchor(item, PG_Anchor::Right, column11, PG_Anchor::Right);
		item->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 22.f);

		textCreator("Title", column11);
		item = PG_Gui::createGuiElementV2<PG_GuiTextField>(column11, m_model, "Description", "description", "text", textFieldCreator);
		anchor(item, PG_Anchor::Left, column11, PG_Anchor::Left);
		anchor(item, PG_Anchor::Right, column11, PG_Anchor::Right);

		auto* column12 = PG_new<PG_GuiColumn>("Column12", row1);
		column12->setSize(glm::vec2(200.f, 160.f));

		auto selectorCreator = [this](const std::string& name, PG_GuiItem* root) {
			auto guiItem = PG_DisaItemSelector::createImageSelector(name, root, nullptr);
			if (auto prop = m_model->getProperty("rootModel")) {
				PG_Object::connect(m_model, prop, guiItem, [this, guiItem, prop]() {
					dynamic_cast<PG_ListModelFilter*>(guiItem->getModel())->setModel(m_model->getPropertyValue(prop).get<PG_ListModelBase*>());
					});
			}

			return guiItem;
		};

		textCreator("Background Image", column12);
		auto* itemSelector = PG_Gui::createGuiElementV2<PG_DisaItemSelector>(column12, m_model, "BackgroundSelector", "backgroundImage", "name", selectorCreator);

		auto* column13 = PG_new<PG_GuiColumn>("Column13Test", row1);
		column13->setWidth(39.f + 13.f);
		column13->setHeight(125.f);
		column13->setSpacing(2.f);
		anchor(column13, PG_Anchor::VerticalCenter, row1, PG_Anchor::VerticalCenter);

		auto* button = PG_new<PG_GuiButton>("AddDungeon", glm::vec2(0.0f), glm::vec2(39.f, 30.f), column13);
		button->setIcon("I_icon_add");
		button->setBackground("M_GuiRoundedRectangleTop5");
		button->setTooltipText("Add Dungeon");

		connect(button, &PG_GuiButton::clicked, this, [this] { callFunc("void add()"); });

		button = PG_new<PG_GuiButton>("Delete", glm::vec2(0.0f), glm::vec2(39.f, 30.f), column13);
		button->setIcon("I_icon_delete");
		button->setTooltipText("Delete");

		connect(button, &PG_GuiButton::clicked, this, [this] { callFunc("void remove()"); }, PG_ConnectionType_Event);

		button = PG_new<PG_GuiButton>("Up", glm::vec2(0.0f), glm::vec2(39.f, 30.f), column13);
		button->setIcon("I_disa_icon_up");
		button->setTooltipText("Move Up");

		connect(button, &PG_GuiButton::clicked, this, [this] { callFunc("void moveUp()"); });

		button = PG_new<PG_GuiButton>("Down", glm::vec2(0.0f), glm::vec2(39.f, 30.f), column13);
		button->setIcon("I_disa_icon_down");
		button->setBackground("M_GuiRoundedRectangleBottom5");
		button->setTooltipText("Move Down");

		connect(button, &PG_GuiButton::clicked, this, [this] { callFunc("void moveDown()"); });

	}

	textCreator("Dungeons", columnRoot, 20.f);

	auto* devider = PG_new<PG_GuiLines>("Devider", glm::vec2(0.f), glm::vec2(0.f, 10.f), columnRoot);
	devider->makeLineX();
	PG_GuiItem::anchor(devider, PG_Anchor::Left, columnRoot, PG_Anchor::Left);
	PG_GuiItem::anchor(devider, PG_Anchor::Right, columnRoot, PG_Anchor::Right);
	devider->setColor(backgroundDecoColor);


	auto flowView = PG_new<PG_GuiListView>("DungeonsListView", columnRoot);
	PG_GuiItem::anchor(flowView, PG_Anchor::Left, columnRoot, PG_Anchor::Left);
	PG_GuiItem::anchor(flowView, PG_Anchor::Right, columnRoot, PG_Anchor::Right);

	flowView->setItem(PG_GuiListView::getDefaultCreateFunc<PG_DisaCustomCampaignDungeon>());
	flowView->setItemUpdate(PG_GuiListView::getDefaultUpdateFunc<PG_DisaCustomCampaignDungeon>());
	flowView->setCellSize(dungeonHeight);
	flowView->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	flowView->setSpacing(5.f);

	if (auto prop = m_model->getProperty("dungeons")) {
		PG_Object::connect(m_model, prop, flowView, [this, flowView, prop]() {
			auto* model = m_model->getPropertyValue(prop).get<PG_ListModelBase*>();
			if (flowView->getModel()) {
				disconnect(flowView->getModel(), nullptr, this, nullptr);
			}

			if (model) {
				setHeight(campaignAreaBaseHeight + (dungeonHeight + 5.f) * model->size() + 20.f);
				connect(model, &PG_ListModelBase::sizeChanged, this, [this, model] {
					setHeight(campaignAreaBaseHeight + (dungeonHeight + 5.f) * model->size() + 20.f);
					if (auto* flow = findParent<PG_GuiFlowView>(10)) { //a bit ugly, but oh well...
						flow->updateChildSize();
					}
				});
			}
			flowView->setModel(model);
		});
	}
}

