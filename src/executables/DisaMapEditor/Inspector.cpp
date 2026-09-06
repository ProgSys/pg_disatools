#include "Inspector.h"

#include <Views/PG_DisaMapSceneView.h>
#include <Controllers/PG_DisaEditorModule.h>
#include <Scene/Editor/PG_EditorModule.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiColumn.h>
#include <Scene/Gui/PG_GuiDropDown.h>
#include <Scene/Gui/PG_GuiCreateSelectionElement.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiSpinBox.h>
#include <Scene/Gui/PG_GuiToggleButton.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiTextEdit.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiScrollBar.h>
#include <Scene/Gui/PG_GuiListView.h>
#include <Scene/Gui/PG_GuiGridView.h>
#include <Scene/Model/PG_ListModel.h>
#include <Scene/Model/PG_ListModelFilter.h>
#include <Util/Color/ColorConverters.h>
#include <Util/PG_RangesTransform.h>
#include <Util/PG_StringUtil.h>
#include <Models/PG_DisaMapEditorMainModel.h>


#include <Disa/map/PG_DisaIntersection.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/map/PG_DisaEventTileModel.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaCharactersModel.h>
#include <Disa/PG_DisaItemModel.h>
#include <Disa/PG_DisaMagicModel.h>
#include <Disa/PG_DisaGeoEffectsModel.h>
#include <Disa/CustomGui/PG_DisaSearchField.h>

#include <Drag/DisaNewCharacterDrag.h>

#include <CustomGui/PG_DisaCharacterPortraitGridTile.h>
#include <CustomGui/PG_DisaEventTileListItem.h>
#include <CustomGui/PG_DisaItemDropDown.h>
#include <CustomGui/PG_DisaUVItem.h>
#include <Util/Color/ColorDefined.h>
#include <Util/PG_Math.h>
#include <Util/PG_RangesTransform.h>

constexpr glm::vec3 backgroundColor = glm::vec3(40.f / 255.f);
constexpr float inspectorWidth = 310.f;

template<typename T, typename R, R T::* Member>
static PG_Any getVecValue(const std::vector<PG_Object*>& selection, const std::vector<PG_Property>& properties, const PG_Any& defaultValue) {
	auto selcIt = selection.begin();
	auto propIt = properties.begin();

	auto any = PG_SelectionWrapper::overrideWithModel(*selcIt)->getPropertyValue(*propIt);

	R out = any.get<T>().*Member;
	++selcIt, ++propIt;

	for (; selcIt != selection.end(); ++selcIt, ++propIt) {
		auto any2 = PG_SelectionWrapper::overrideWithModel(*selcIt)->getPropertyValue(*propIt);

		if (out != any2.get<T>().*Member) return defaultValue;
	}

	return PG_Any::from<R>(out);

}


template<typename T, typename R, R T::* Member>
static void setVecValue(const std::vector<PG_Object*>& selection, const std::vector<PG_Property>& properties, const PG_Any& value) {

	auto selcIt = selection.begin();
	auto propIt = properties.begin();
	R memberValue = value.is<int>()? value.get<int>(): value.get<R>();
	for (; selcIt != selection.end(); ++selcIt, ++propIt) {
		auto model = PG_SelectionWrapper::overrideWithModel(*selcIt);
		auto any = model->getPropertyValue(*propIt);
		T v = any.get<T>();
		(v.*Member) = memberValue;
		model->setPropertyValue(*propIt, PG_Any::from(v));
	}
}


template<int glm::ivec3::* Member>
static PG_Any getIVec3Value(const std::vector<PG_Object*>& selection, const std::vector<PG_Property>& properties, const PG_Any& defaultValue) {
	return getVecValue<glm::ivec3, int, Member>(selection, properties, defaultValue);
}

template<int glm::ivec3::* Member>
static void setIVec3Value(const std::vector<PG_Object*>& selection, const std::vector<PG_Property>& properties, const PG_Any& value) {
	return setVecValue<glm::ivec3, int, Member>(selection, properties, value);
}

template<int glm::ivec2::* Member>
static PG_Any getIVec2Value(const std::vector<PG_Object*>& selection, const std::vector<PG_Property>& properties, const PG_Any& defaultValue) {
	return getVecValue<glm::ivec2, int, Member>(selection, properties, defaultValue);
}

template<int glm::ivec2::* Member>
static void setIVec2Value(const std::vector<PG_Object*>& selection, const std::vector<PG_Property>& properties, const PG_Any& value) {
	return setVecValue<glm::ivec2, int, Member>(selection, properties, value);
}

/******* Models ********/

/**
* @brief Filter for characters
*/
class FilterUniqueModel : public PG_ListModelFilter {
	std::string m_searchText;
	static inline bool m_showAll = false;
public:
	using PG_ListModelFilter::PG_ListModelFilter;
	void setSearchText(const std::string& str) {
		if (m_searchText == str) return;
		m_searchText = str;
		invalidate();
	}
	bool isShowAll() const { return m_showAll; }
	void setShowAll(bool showAll) {
		if (showAll == m_showAll) return;
		m_showAll = showAll;
		invalidate();
	}

	bool filter(const PG_Variety& v) const final {
		assert(v.is<PG_DisaCharacterModel>());
		unsigned short id = v.get<PG_DisaCharacterModel>().id;
		if (!m_showAll) {
			if (id > 10000) return false;
			if (id > 1000 && (id % 10) != 0) {
				return true;
			}
		}

		if (!m_searchText.empty()) {
			const auto& name = v.get<PG_DisaCharacterModel>().nameNormelized;
			return !(PG_StringUtil::contains(name, m_searchText, false) || PG_StringUtil::contains(std::to_string(id), m_searchText, false));
		}

		return false;
	}
};

/**
* @brief Only displayes tiles for a given postion
*/
class EventTilePositionFilter : public PG_ListModelFilter {
	glm::ivec2 m_tilePosition;
public:
	using PG_ListModelFilter::PG_ListModelFilter;

	void setTilePosition(glm::ivec2 pos) {
		if (pos == m_tilePosition) return;
		m_tilePosition = pos;
		invalidate();
	}

	bool filter(const PG_Variety& v) const final {
		//assert(v.is<PG_DisaEventTileModel>());
		return v.get<PG_DisaEventTileModel*>()->getTilePosition() != m_tilePosition;
	}

	void addEvent() {
		if (auto model = dynamic_cast<PG_DisaEventTileListModel*>(getModel())) {
			model->addEvent(m_tilePosition);
		}
	}
};

/**************/


PG_GuiRow* createRow(PG_GuiItem* parent, const std::string& textStr, float textWidth = 240.f) {
	PG_GuiRow* row = PG_new<PG_GuiRow>("Row", glm::vec2(0.f), glm::vec2(40.f, 32.f), parent);
	PG_GuiItem::anchor(row, PG_Anchor::Right, parent, PG_Anchor::Right);
	PG_GuiItem::anchor(row, PG_Anchor::Left, parent, PG_Anchor::Left);
	row->setSpacing(0.f);

	auto text = PG_new<PG_GuiText>("Text", glm::vec2(0.f), glm::vec2(textWidth, 32.f), row);
	text->setColor(PG_Color::hentaiBlueFrgb);
	text->setFontSize(21.f);
	text->setWrapMode(PG_GuiText::None);
	text->setVerticalAlignment(PG_GuiText::ETextAlignment::VerticalCenter);
	text->setText(textStr);
	return row;
	};

void Inspector::setup(PG_DisaMapSceneView* scene, PG_GuiItem* root) {
	auto mod = scene->getModule<PG_DisaEditorModule>();
	assert(mod);
	auto* inspector = PG_new<Inspector>(root);
	inspector->mod = mod;
	inspector->root = root;

	PG_Object::connect(mod, &PG_DisaEditorModule::modeChanged, inspector, &Inspector::createFunctionBar);
	PG_Object::connect(mod, &PG_DisaEditorModule::modeChanged, inspector, &Inspector::createInspector);
	PG_Object::connect(mod, &PG_EditorModule::selectionChanged, inspector, &Inspector::createInspector);
	PG_Object::connect(mod, &PG_DisaEditorModule::materialInspectorModeChanged, inspector, &Inspector::createInspector);
}

static inline void createForTerrainEdit(PG_GuiColumn* column, PG_SelectionWrapper* wrapper) {
	column->clearGuiChildren();

	wrapper->registerProperty("mobility", PG_Any::from<int>(0));
	wrapper->registerProperty("geoColor", PG_Any::from<int>(0));
	wrapper->registerProperty("isStartingPoint", PG_Any::from<bool>(false), &PG_SelectionWrapper::getValueDefault, 
		[](const std::vector<PG_Object*>& selection, const std::vector<PG_Property>&properties, const PG_Any & value) {
			PG_Gui::overrideWithModel(selection.back())->setPropertyValue(properties.back(), value);
	});

	wrapper->registerProperty("topA", PG_Any::from<char>(0), &PG_SelectionWrapper::getValueFirst, &PG_SelectionWrapper::setValueDiff);
	wrapper->registerProperty("topB", PG_Any::from<char>(0), &PG_SelectionWrapper::getValueFirst, &PG_SelectionWrapper::setValueDiff);
	wrapper->registerProperty("topC", PG_Any::from<char>(0), &PG_SelectionWrapper::getValueFirst, &PG_SelectionWrapper::setValueDiff);
	wrapper->registerProperty("topD", PG_Any::from<char>(0), &PG_SelectionWrapper::getValueFirst, &PG_SelectionWrapper::setValueDiff);


	wrapper->registerProperty("bottomAx", "bottomA", PG_Any::from<int8_t>(0), &getVecValue<glm::i8vec2, int8_t, &glm::i8vec2::x>, &setVecValue<glm::i8vec2, int8_t, &glm::i8vec2::x>);
	wrapper->registerProperty("bottomAy", "bottomA", PG_Any::from<int8_t>(0), &getVecValue<glm::i8vec2, int8_t, &glm::i8vec2::y>, &setVecValue<glm::i8vec2, int8_t, &glm::i8vec2::y>);
	wrapper->registerProperty("bottomBx", "bottomB", PG_Any::from<int8_t>(0), &getVecValue<glm::i8vec2, int8_t, &glm::i8vec2::x>, &setVecValue<glm::i8vec2, int8_t, &glm::i8vec2::x>);
	wrapper->registerProperty("bottomBy", "bottomB", PG_Any::from<int8_t>(0), &getVecValue<glm::i8vec2, int8_t, &glm::i8vec2::y>, &setVecValue<glm::i8vec2, int8_t, &glm::i8vec2::y>);
	wrapper->registerProperty("bottomCx", "bottomC", PG_Any::from<int8_t>(0), &getVecValue<glm::i8vec2, int8_t, &glm::i8vec2::x>, &setVecValue<glm::i8vec2, int8_t, &glm::i8vec2::x>);
	wrapper->registerProperty("bottomCy", "bottomC", PG_Any::from<int8_t>(0), &getVecValue<glm::i8vec2, int8_t, &glm::i8vec2::y>, &setVecValue<glm::i8vec2, int8_t, &glm::i8vec2::y>);
	wrapper->registerProperty("bottomDx", "bottomD", PG_Any::from<int8_t>(0), &getVecValue<glm::i8vec2, int8_t, &glm::i8vec2::x>, &setVecValue<glm::i8vec2, int8_t, &glm::i8vec2::x>);
	wrapper->registerProperty("bottomDy", "bottomD", PG_Any::from<int8_t>(0), &getVecValue<glm::i8vec2, int8_t, &glm::i8vec2::y>, &setVecValue<glm::i8vec2, int8_t, &glm::i8vec2::y>);
	
	wrapper->registerProperty("sideVisibleN", "sideVisibility", PG_Any::from<bool>(false), &getVecValue<glm::bvec4, bool, &glm::bvec4::x>, &setVecValue<glm::bvec4, bool, &glm::bvec4::x>);
	wrapper->registerProperty("sideVisibleE", "sideVisibility", PG_Any::from<bool>(false), &getVecValue<glm::bvec4, bool, &glm::bvec4::y>, &setVecValue<glm::bvec4, bool, &glm::bvec4::y>);
	wrapper->registerProperty("sideVisibleS", "sideVisibility", PG_Any::from<bool>(false), &getVecValue<glm::bvec4, bool, &glm::bvec4::z>, &setVecValue<glm::bvec4, bool, &glm::bvec4::z>);
	wrapper->registerProperty("sideVisibleW", "sideVisibility", PG_Any::from<bool>(false), &getVecValue<glm::bvec4, bool, &glm::bvec4::w>, &setVecValue<glm::bvec4, bool, &glm::bvec4::w>);

	constexpr float textWidth = 140.f;
	{
		PG_GuiRow* row = createRow(column, "Mobility:", textWidth);
		auto dropDown = PG_Gui::createGuiElementV2<PG_GuiDropDown>(row, wrapper, "mobilityDropDown", "mobility", "index",
			[](const std::string& name, PG_GuiItem* root) {
				auto dropDown = PG_new<PG_GuiDropDown>(name, glm::vec2(0.f, 0.f), glm::vec2(40.f, 32.f), root);
				auto model = PG_new<PG_ListModel<std::string>>(dropDown);

				model->set({
						{ "Unlimited"},
						{ "Fly only"},
						{ "Impassable"},
					});
				dropDown->setModel(model);

				return dropDown;
			});
		dropDown->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	}

	{
		PG_GuiRow* row = createRow(column, "Geo Color:", textWidth);
		auto dropDown = PG_Gui::createGuiElementV2<PG_GuiDropDown>(row, wrapper, "geoColorDropDown", "geoColor", "index",
			[](const std::string& name, PG_GuiItem* root) {
				auto dropDown = PG_new<PG_GuiDropDown>(name, glm::vec2(0.f, 0.f), glm::vec2(40.f, 32.f), root);
				auto model = PG_new<PG_ListModel<std::string>>(dropDown);

				model->set({
					{ "None"},
					{ "Red"},
					{ "Green"},
					{ "Blue"},
					{ "Yellow"},
					{ "Magenta"},
					{ "Cyan"},
					{ "White"}
					});
				dropDown->setModel(model);

				return dropDown;
			});
		dropDown->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	}


	{
		PG_GuiRow* row = createRow(column, "Corners:", 90.f);

		struct CreateInfo {
			std::string target;
			std::string background;
		};

		constexpr float spinTexWidth = 50.0f;
		for (CreateInfo& info : std::array<CreateInfo, 4>{
			CreateInfo{"topA", "M_GuiRoundedRectangleLeft5"},
			CreateInfo{"topB", "M_GuiRectangle"},
			CreateInfo{"topC", "M_GuiRectangle"},
			CreateInfo{"topD", "M_GuiRoundedRectangleRight5"},
			}) {

			auto* spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, char>(row, wrapper, info.target + "SpinBox", info.target);
			PG_GuiSpinBox::setupInt(spinBox, 1, 127);
			spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
			spinBox->getBackground()->setMaterial(info.background);
			spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		}
	}

	{
		PG_GuiRow* row = createRow(column, "Bottom:", 90.f);

		constexpr float spinTexWidth = 48.0f;

		auto* spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int8_t>(row, wrapper, "bottomAxSpinBox", "bottomAx");
		PG_GuiSpinBox::setupInt(spinBox, 0, 127);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int8_t>(row, wrapper, "bottomAySpinBox", "bottomAy");
		PG_GuiSpinBox::setupInt(spinBox, 0, 127);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		auto spacer = PG_new<PG_GuiItem>("spacer", glm::vec2(0.f), glm::vec2(1.f), row);
		spacer->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int8_t>(row, wrapper, "bottomBxSpinBox", "bottomBx");
		PG_GuiSpinBox::setupInt(spinBox, 0, 127);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int8_t>(row, wrapper, "bottomBySpinBox", "bottomBy");
		PG_GuiSpinBox::setupInt(spinBox, 0, 127);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
	}

	{
		PG_GuiRow* row = createRow(column, "", 90.f);
		constexpr float spinTexWidth = 48.0f;
		auto* spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int8_t>(row, wrapper, "bottomCxSpinBox", "bottomCx");
		PG_GuiSpinBox::setupInt(spinBox, 0, 127);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int8_t>(row, wrapper, "bottomCySpinBox", "bottomCy");
		PG_GuiSpinBox::setupInt(spinBox, 0, 127);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		auto spacer = PG_new<PG_GuiItem>("spacer", glm::vec2(0.f), glm::vec2(1.f), row);
		spacer->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int8_t>(row, wrapper, "bottomDxSpinBox", "bottomDx");
		PG_GuiSpinBox::setupInt(spinBox, 0, 127);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int8_t>(row, wrapper, "bottomDySpinBox", "bottomDy");
		PG_GuiSpinBox::setupInt(spinBox, 0, 127);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
	}

	{
		PG_GuiRow* row = createRow(column, "Visible:", 90.f);

		struct CreateInfo {
			std::string target;
			std::string toolTip;
			std::string background;
		};

		for (CreateInfo& info : std::array<CreateInfo, 4>{
			CreateInfo{"sideVisibleN", "Side visible N", "M_GuiRoundedRectangleLeft5"},
			CreateInfo{"sideVisibleE", "Side visible E", "M_GuiRectangle"},
			CreateInfo{"sideVisibleS", "Side visible S", "M_GuiRectangle"},
			CreateInfo{"sideVisibleW", "Side visible W", "M_GuiRoundedRectangleRight5"},
			}) {

			PG_Gui::createGuiElementV2<PG_GuiToggleButton>(row, wrapper, info.target + "Checkbox", info.target, "toggled",
				[&info](const std::string& name, PG_GuiItem* root) {
					auto toggleButton = PG_new<PG_GuiToggleButton>(name, glm::vec2(0.f), glm::vec2(50.f, 32.f), root);
					toggleButton->setTooltipText(info.toolTip);
					toggleButton->setIcon("gui/icon_eye.png");
					toggleButton->setBackground(info.background);
					toggleButton->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
					return toggleButton;
				});
		}




	}

	auto* guiToggleButton = PG_Gui::createGuiElementV2<PG_GuiToggleButton>(column, wrapper, "isStartingPointCheckbox", "isStartingPoint", "toggled",
		[](const std::string& name, PG_GuiItem* root) {
			auto toggleButton = PG_new<PG_GuiToggleButton>(name, glm::vec2(5.f, 0.f), glm::vec2(40.f, 32.f), root);
			toggleButton->setTooltipText("Starting point of the map");
			toggleButton->setIcon("gui/disa/icon_teleporter.png");
			toggleButton->setText("Set teleporter");
			toggleButton->setBackground("M_GuiRoundedRectangleAll5");
			return toggleButton;
		});
	PG_GuiItem::anchor(guiToggleButton, PG_Anchor::Right, column, PG_Anchor::Right);
	PG_GuiItem::anchor(guiToggleButton, PG_Anchor::Left, column, PG_Anchor::Left);
}

static inline PG_Any getValueSumUvOutline(const std::vector<PG_Object*>& selection, const std::vector<PG_Property>& properties, const PG_Any& defaultValue) {

	auto selcIt = selection.begin();
	auto propIt = properties.begin();

	std::vector<std::array<glm::vec2, 4>> outlines;
	for (; selcIt != selection.end(); ++selcIt, ++propIt) {
		auto any = (*selcIt)->getPropertyValue(*propIt);
		outlines.push_back(any.get<std::array<glm::vec2, 4>>());
	}
	return PG_Any::from(outlines);
}

static inline void setValueSumUvOutline(const std::vector<PG_Object*>& selection, const std::vector<PG_Property>& properties, const PG_Any& value) {
	return;
	//TODO notused
	auto selcIt = selection.begin();
	auto propIt = properties.begin();

	const std::vector<std::array<glm::vec2, 4>>& outlines = value.get<std::vector<std::array<glm::vec2, 4>>>();
	auto outlineIt = outlines.begin();
	for (; selcIt != selection.end(); ++selcIt, ++propIt, ++outlineIt) {
		//*selcIt->setPropertyValue(*propIt, PG_Any::from(*outlineIt));
	}
}

static inline void createForMaterialEdit(Inspector* self, PG_GuiColumn* column, PG_SelectionWrapper* wrapper) {
	column->clearGuiChildren();

	PG_GuiRow* materialModeRow = PG_new<PG_GuiRow>("materialModeRow", column);
	{
		materialModeRow->setWidth(100.f);
		materialModeRow->setHeight(30.f);
		materialModeRow->setSpacing(3.f);
		PG_GuiItem::anchor(materialModeRow, PG_Anchor::Left, column, PG_Anchor::Left);
		PG_GuiItem::anchor(materialModeRow, PG_Anchor::Right, column, PG_Anchor::Right);

		const float width = inspectorWidth /3.f - 10.f;
		auto* btn = PG_new<PG_GuiToggleButton>("base", materialModeRow);
		btn->setSize(glm::vec2(width, 30.f));
		btn->setText("Base");
		btn->setAutoToggle(false);
		btn->setBackground("M_GuiRoundedRectangleLeft5");
		btn->getText()->setPropertyValue("horizontalAlignment", PG_Any::from(PG_GuiText::HorizontalCenter));
		btn->setLayoutProperty(PG_GuiToggleButton::LayoutProperty_Flow, PG_Anchor::HorizontalCenter);
		btn->setToggled(self->getMaterialInspectorMode() == PG_DisaEditorModule::MaterialInspectorMode_Base);

		PG_Object::connect(self->mod, &PG_DisaEditorModule::materialInspectorModeChanged, btn, [btn](PG_DisaEditorModule::EMaterialInspectorMode mode) {
			btn->setToggled(mode == PG_DisaEditorModule::MaterialInspectorMode_Base);
			});
		PG_Object::connect(btn, &PG_GuiToggleButton::clicked, self, [self] {
			self->setMaterialInspectorMode(PG_DisaEditorModule::MaterialInspectorMode_Base);
		});

		btn = PG_new<PG_GuiToggleButton>("decal", materialModeRow);
		btn->setSize(glm::vec2(width, 30.f));
		btn->setText("Decal");
		btn->setAutoToggle(false);
		btn->getText()->setPropertyValue("horizontalAlignment", PG_Any::from(PG_GuiText::HorizontalCenter));
		btn->setLayoutProperty(PG_GuiToggleButton::LayoutProperty_Flow, PG_Anchor::HorizontalCenter);
		btn->setToggled(self->getMaterialInspectorMode() == PG_DisaEditorModule::MaterialInspectorMode_Decal);

		PG_Object::connect(self->mod, &PG_DisaEditorModule::materialInspectorModeChanged, btn, [btn](PG_DisaEditorModule::EMaterialInspectorMode mode) {
			btn->setToggled(mode == PG_DisaEditorModule::MaterialInspectorMode_Decal);
			});
		PG_Object::connect(btn, &PG_GuiToggleButton::clicked, self, [self] {
			self->setMaterialInspectorMode(PG_DisaEditorModule::MaterialInspectorMode_Decal);
			});

		btn = PG_new<PG_GuiToggleButton>("shadow", materialModeRow);
		btn->setSize(glm::vec2(width, 30.f));
		btn->setText("Shadow");
		btn->setAutoToggle(false);
		btn->setBackground("M_GuiRoundedRectangleRight5");
		btn->getText()->setPropertyValue("horizontalAlignment", PG_Any::from(PG_GuiText::HorizontalCenter));
		btn->setLayoutProperty(PG_GuiToggleButton::LayoutProperty_Flow, PG_Anchor::HorizontalCenter);
		btn->setToggled(self->getMaterialInspectorMode() == PG_DisaEditorModule::MaterialInspectorMode_Shadow);

		PG_Object::connect(self->mod, &PG_DisaEditorModule::materialInspectorModeChanged, btn, [btn](PG_DisaEditorModule::EMaterialInspectorMode mode) {
			btn->setToggled(mode == PG_DisaEditorModule::MaterialInspectorMode_Shadow);
			});
		PG_Object::connect(btn, &PG_GuiToggleButton::clicked, self, [self] {
			self->setMaterialInspectorMode(PG_DisaEditorModule::MaterialInspectorMode_Shadow);
		});
	}


	wrapper->registerProperty("isDecalSupported", "index", PG_Any::from<bool>(0), 
		[](const std::vector<PG_Object*>& selection, const std::vector<PG_Property>&, const PG_Any& defaultValue) ->PG_Any {
			bool isDecalSupported = std::ranges::all_of(selection, [](const PG_Object* obj) -> bool {
				auto m = dynamic_cast<const PG_DisaMapTileMaterialModel*>(obj);
				return m && (m->getIndex() == 4 || m->getIndex() == 9 || m->getIndex() == 10);
				});
			return isDecalSupported;
		},
		&PG_SelectionWrapper::setValueNone);
	wrapper->registerProperty("texU", PG_Any::from<unsigned char>(0), &PG_SelectionWrapper::getValueFirst, &PG_SelectionWrapper::setValueDiff);
	wrapper->registerProperty("texV", PG_Any::from<unsigned char>(0), &PG_SelectionWrapper::getValueFirst, &PG_SelectionWrapper::setValueDiff);
	wrapper->registerProperty("texVRight", PG_Any::from<unsigned char>(0), &PG_SelectionWrapper::getValueFirst, &PG_SelectionWrapper::setValueDiff);

	wrapper->registerProperty("texWidth", PG_Any::from<unsigned char>(0), &PG_SelectionWrapper::getValueFirst, &PG_SelectionWrapper::setValueDiff);
	wrapper->registerProperty("texHeight", PG_Any::from<unsigned char>(0), &PG_SelectionWrapper::getValueFirst, &PG_SelectionWrapper::setValueDiff);
	wrapper->registerProperty("texHeightRight", PG_Any::from<unsigned char>(0), &PG_SelectionWrapper::getValueFirst, &PG_SelectionWrapper::setValueDiff);

	wrapper->registerProperty("texture", PG_Any::from<PG_Texture>(PG_Texture()));
	wrapper->registerProperty("texUvOutlines", "texUvOutline", PG_Any::from<std::vector<std::array<glm::vec2, 4>>>({}), &getValueSumUvOutline, &setValueSumUvOutline);
	
	wrapper->registerProperty("texRotation", PG_Any::from<int>(-1));
	wrapper->registerProperty("texMirrorVertical", PG_Any::from<bool>(false), &PG_SelectionWrapper::getValueFirst);
	wrapper->registerProperty("texMirrorHorizontal", PG_Any::from<bool>(false), &PG_SelectionWrapper::getValueFirst);
	wrapper->registerProperty("texIsShadow", PG_Any::from<bool>(false), &PG_SelectionWrapper::getValueFirst);
	
	PG_Gui::connectChanged(materialModeRow->findFirstChild<PG_GuiToggleButton>("decal"),
		wrapper, "isDecalSupported", [self](PG_GuiToggleButton* btn, const PG_Any& value) {
			const bool bValue = value.get<bool>();
			if (!bValue && self->getMaterialInspectorMode() == PG_DisaEditorModule::MaterialInspectorMode_Decal) {
				self->setMaterialInspectorMode(PG_DisaEditorModule::MaterialInspectorMode_Base);
			}
			else {
				btn->setEnabled(bValue);
			}
		});
	
		constexpr float spinTexWidth = 50.f;
	{

		auto row = createRow(column, "UV:", 80.f);
		row->setMargin(PG_Anchor::Right, 5.f);
		row->setMargin(PG_Anchor::Left, 5.f);
		auto* spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned char>(row, wrapper, "texUSpinBox", "texU");
		PG_GuiSpinBox::setupInt(spinBox, 0, 255);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned char>(row, wrapper, "texVSpinBox", "texV");
		PG_GuiSpinBox::setupInt(spinBox, 0, 255);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRectangle");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned char>(row, wrapper, "texVRightSpinBox", "texVRight");
		PG_GuiSpinBox::setupInt(spinBox, 0, 255);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
	}

	{
		auto row = createRow(column, "Size:", 80.f);
		row->setMargin(PG_Anchor::Right, 5.f);
		row->setMargin(PG_Anchor::Left, 5.f);
		auto spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned char>(row, wrapper, "texWidthSpinBox", "texWidth");
		PG_GuiSpinBox::setupInt(spinBox, 0, 255);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned char>(row, wrapper, "texHeightSpinBox", "texHeight");
		PG_GuiSpinBox::setupInt(spinBox, 0, 255);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRectangle");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned char>(row, wrapper, "texHeightRightSpinBox", "texHeightRight");
		PG_GuiSpinBox::setupInt(spinBox, 0, 255);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
	}

	{
		auto row = createRow(column, "Rotation:", 80.f);
		row->setMargin(PG_Anchor::Right, 5.f);
		row->setMargin(PG_Anchor::Left, 5.f);

		auto dropDown = PG_Gui::createGuiElementV2<PG_GuiDropDown>(row, wrapper, "rotationDropDown", "texRotation", "index",
			[](const std::string& name, PG_GuiItem* root) {
				auto dropDown = PG_new<PG_GuiDropDown>(name, glm::vec2(0.f, 0.f), glm::vec2(100.f, 32.f), root);
				auto model = PG_new<PG_ListModel<std::string>>(dropDown);

				model->set({
					{ "North"},
					{ "East"},
					{ "South"},
					{ "West"},
					});
				dropDown->setModel(model);
				return dropDown;
			});
		dropDown->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::Right);
	}

	{
		auto row = createRow(column, "Mirror:", 80.f);
		row->setMargin(PG_Anchor::Right, 5.f);
		row->setMargin(PG_Anchor::Left, 5.f);

		auto btn = PG_new<PG_GuiToggleButton>("toggleBtn", row);
		btn->setSize(glm::vec2(50.f, 32.f));
		btn->setIcon("gui/icon_mirror_vertical.png");
		btn->setBackground("M_GuiRoundedRectangleLeft5");
		btn->setTooltipText("Mirror Vertical");
		btn->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
		PG_Gui::connectGuiElementV2(wrapper, "texMirrorVertical", btn, "toggled");

		btn = PG_new<PG_GuiToggleButton>("toggleBtn", row);
		btn->setSize(glm::vec2(50.f, 32.f));
		btn->setIcon("gui/icon_mirror_horizontal.png");
		btn->setBackground("M_GuiRoundedRectangleRight5");
		btn->setTooltipText("Mirror Horizontal");
		btn->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
		PG_Gui::connectGuiElementV2(wrapper, "texMirrorHorizontal", btn, "toggled");
	}

	{
		auto row = createRow(column, "Flags:", 80.f);
		row->setMargin(PG_Anchor::Right, 5.f);
		row->setMargin(PG_Anchor::Left, 5.f);

		auto btn = PG_new<PG_GuiToggleButton>("toggleBtn", row);
		btn->setSize(glm::vec2(50.f, 32.f));
		btn->setIcon("gui/disa/icon_shadow.png");
		btn->setBackground("M_GuiRoundedRectangleAll5");
		btn->setTooltipText("Is shadow");
		btn->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
		PG_Gui::connectGuiElementV2(wrapper, "texIsShadow", btn, "toggled");
	}

	auto uvItem = PG_new<PG_DisaUVItem>("uvItem", glm::vec2(0.f, 0.f), glm::vec2(inspectorWidth - 10), column);
	PG_Gui::connectGuiElementV2(wrapper, "texture", uvItem, "image");
	PG_Gui::connectGuiElementV2(wrapper, "texUvOutlines", uvItem, "outlines");


	{
		static unsigned char texGridU = 32;
		static unsigned char texGridV = 32;
		static bool texIsGridEnabled = true;
		constexpr auto texGridUPropName = "texGridU";
		constexpr auto texGridVPropName = "texGridV";
		constexpr auto texIsGridEnabledPropName = "texIsGridEnabled";
		wrapper->registerProperty(texGridUPropName, PG_Any::from<unsigned char>(texGridU), true);
		wrapper->registerProperty(texGridVPropName, PG_Any::from<unsigned char>(texGridV), true);
		wrapper->registerProperty(texIsGridEnabledPropName, PG_Any::from<bool>(texIsGridEnabled), true);

		PG_Object::connect(wrapper, wrapper->getProperty(texGridUPropName), wrapper, [wrapper]() {
			texGridU = wrapper->getPropertyValue(wrapper->getProperty(texGridUPropName)).get<unsigned char>();
			});
		PG_Object::connect(wrapper, wrapper->getProperty(texGridVPropName), wrapper, [wrapper]() {
			texGridV = wrapper->getPropertyValue(wrapper->getProperty(texGridVPropName)).get<unsigned char>();
			});
		PG_Object::connect(wrapper, wrapper->getProperty(texIsGridEnabledPropName), wrapper, [wrapper]() {
			texIsGridEnabled = wrapper->getPropertyValue(wrapper->getProperty(texIsGridEnabledPropName)).get<bool>();
			});

		auto roundValue = [wrapper](glm::ivec2 value) -> glm::ivec2 {
			if (wrapper->getPropertyValue(wrapper->getProperty(texIsGridEnabledPropName)).toBool()){
				glm::ivec2 rounder = {
					wrapper->getPropertyValue(wrapper->getProperty(texGridUPropName)).get<unsigned char>(),
					wrapper->getPropertyValue(wrapper->getProperty(texGridVPropName)).get<unsigned char>()
				};
				return { PG_Math::roundTo(value.x, rounder.x), PG_Math::roundTo(value.y, rounder.y) };
			}

			return value;
		};

		PG_GuiRow* row = PG_new<PG_GuiRow>("Row", glm::vec2(0.f), glm::vec2(40.f, 32.f), column);
		PG_GuiItem::anchor(row, PG_Anchor::Right, uvItem, PG_Anchor::Right);
		PG_GuiItem::anchor(row, PG_Anchor::Left, uvItem, PG_Anchor::Left);

		row->setSpacing(0.f);
		auto text = PG_new<PG_GuiText>("UVAtText", glm::vec2(0.f), glm::vec2(100.f, 32.f), row);
		text->setColor(PG_Color::hentaiBlueFrgb);
		text->setFontSize(14.f);
		text->setWrapMode(PG_GuiText::None);
		text->setVerticalAlignment(PG_GuiText::ETextAlignment::Top);
		PG_Object::connect(uvItem, &PG_DisaUVItem::hoverAt, text, [text, roundValue](glm::ivec2 value) {
			if (value.x < 0 || value.y < 0) {
				text->setVisible(false);
				return;
			}
			value = roundValue(value);

			text->setText(std::to_string(value.x) + " " + std::to_string(value.y));
			text->setVisible(true);
		});

		PG_Object::connect(uvItem, &PG_DisaUVItem::clickedAt, wrapper, [wrapper, roundValue](glm::ivec2 value) {
			value = roundValue(value);
			wrapper->setPropertyValue(wrapper->getProperty("texU"), static_cast<unsigned char>(value.x % 255));
			wrapper->setPropertyValue(wrapper->getProperty("texV"), static_cast<unsigned char>(value.y % 255));
			});

		auto* spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned char>(row, wrapper, "texGridUSpinBox", "texGridU");
		PG_GuiSpinBox::setupInt(spinBox, 1, 255);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned char>(row, wrapper, "texGridVSpinBox", "texGridV");
		PG_GuiSpinBox::setupInt(spinBox, 1, 255);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRectangle");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		auto btn = PG_new<PG_GuiToggleButton>("toggleGridBtn", row);
		btn->setSize(glm::vec2(50.f, 32.f));
		btn->setIcon("gui/icon_grid.png");
		btn->setBackground("M_GuiRoundedRectangleRight5");
		btn->setTooltipText("Grid");
		btn->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
		PG_Gui::connectGuiElementV2(wrapper, "texIsGridEnabled", btn, "toggled");
	}

	{
		auto btn = PG_new<PG_GuiButton>("autoPlaneProjectBtn", glm::vec2(0.f, 0.f), glm::vec2(40.f, 32.f), column);
		btn->setText("Auto plane project");
		btn->setIcon("gui/disa/icon_event.png");
		btn->setTooltipText("Projects UVs onto the plane originating from the first selected face");
		btn->setBackground("M_GuiRoundedRectangleAll5");
		PG_GuiItem::anchor(btn, PG_Anchor::Right, column, PG_Anchor::Right);
		PG_GuiItem::anchor(btn, PG_Anchor::Left, column, PG_Anchor::Left);

		PG_Object::connect(btn, &PG_GuiButton::clicked, wrapper, [wrapper] {
			const auto& selection = wrapper->getSelectionItems();
			if (selection.size() < 1) return;
			auto* tileMaterialModel = dynamic_cast<PG_DisaMapTileMaterialModel*>(selection.front());
			if (!tileMaterialModel) return;
			
			struct ProjetionPlane {
				glm::vec3 origin;
				glm::mat3 base;

				glm::vec2 project(glm::vec3 p) const{
					p -= origin;
					glm::vec2 uv(base * p);
					uv *= 256.f;
					uv = glm::round(uv);
					/*
					uv.x = std::fmodf(uv.x, 256.f);
					uv.y = std::fmodf(uv.y, 256.f);
					if (uv.x < 0.f) uv.x += 256.f;
					if (uv.y < 0.f) uv.y += 256.f;
					*/
					return uv;
				}
			};


			ProjetionPlane plane;
			plane.origin = tileMaterialModel->getTile()->getTranslation();

			using PointsGetFunc = std::array<glm::vec3, 4>(PG_DisaMapTileModel::*)(const glm::vec3&) const;
			PointsGetFunc pointsGetFunc;
			switch (tileMaterialModel->getIndex()) {
			case 5:
			case 0: { //N
				plane.base = {
					glm::vec3(-1.f, 0.f, 0.f),
					glm::vec3(0.f, -1.f, 0.f),
					glm::vec3(0.f, 0.f, -1.f) //normal
				};
				pointsGetFunc = &PG_DisaMapTileModel::getSideN;
			} break;
			case 6:
			case 1: { //E
				plane.base = {
				glm::vec3(0.f, 0.f, 1.f),
				glm::vec3(0.f, -1.f, 0.f),
				glm::vec3(-1.f, 0.f, 0.f) //normal
				};
				pointsGetFunc = &PG_DisaMapTileModel::getSideE;
			} break;
			case 7:
			case 2: { //S
				plane.base = {
				glm::vec3(1.f, 0.f, 0.f),
				glm::vec3(0.f, -1.f, 0.f),
				glm::vec3(0.f, 0.f, 1.f) //normal
				};
				pointsGetFunc = &PG_DisaMapTileModel::getSideS;
			} break;
			case 8:
			case 3: { //W
				plane.base = {
				glm::vec3(0.f, 0.f, -1.f),
				glm::vec3(0.f, -1.f, 0.f),
				glm::vec3(1.f, 0.f, 0.f) //normal
				};
				pointsGetFunc = &PG_DisaMapTileModel::getSideW;
			} break;
			default:
			case 4: //top base
			case 9: // top decal, top shadow
			case 10: {
				plane.base = {
				glm::vec3(-1.f, 0.f, 0.f),
				glm::vec3(0.f, 0.f, -1.f),
				glm::vec3(0.f, 1.f, 0.f) //normal
				};
				pointsGetFunc = &PG_DisaMapTileModel::getTop;
			} break;
			}

			
			if (tileMaterialModel->getRotation() != 0) {
				glm::quat q = glm::angleAxis(glm::radians(-90.f * tileMaterialModel->getRotation()), plane.base[2]);
				plane.base = glm::mat3(q) * plane.base;
			}

			constexpr float textureScale = 256.f;
			{
				glm::mat3 toTextureSpace = glm::inverse(plane.base);
				glm::vec2 projectDirU(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
				glm::vec2 projectDirV(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
				glm::vec2 projectOriginMin(std::numeric_limits<float>::max());

				for (glm::vec3 p : std::invoke(pointsGetFunc, tileMaterialModel->getTile(), tileMaterialModel->getTile()->getTranslation())) {
					glm::vec2 project(toTextureSpace * p);
					std::cout << "Point: " << p << " proj: " << project << std::endl;
					projectDirU = { glm::min(projectDirU.x, project.x),  glm::max(projectDirU.y, project.x) };
					projectDirV = { glm::min(projectDirV.x, project.y),  glm::max(projectDirV.y, project.y) };
					if (project.x <= projectOriginMin.x && project.y <= projectOriginMin.y) {
						plane.origin = p;
						projectOriginMin = project + glm::vec2(0.1f);
					}
				}
				std::cout << "Origin: " << plane.origin << std::endl;

				plane.base[0] *= projectDirU.y - projectDirU.x;
				plane.base[1] *= projectDirV.y - projectDirV.x;
				plane.base[0] *= textureScale / tileMaterialModel->getWidth();
				plane.base[1] *= textureScale / tileMaterialModel->getHeight();
				plane.origin -= plane.base[0] * (tileMaterialModel->getU() / textureScale) + plane.base[1] * (tileMaterialModel->getV() / textureScale);
				plane.base = glm::inverse(plane.base);
			}



			std::array<glm::vec2, 4> projected;
			for (auto* item : selection ) {
				auto* model = dynamic_cast<PG_DisaMapTileMaterialModel*>(item);
				if (!model) continue;
				model->setMirrorHorizontal(tileMaterialModel->isMirrorHorizontal());
				model->setMirrorVertical(tileMaterialModel->isMirrorVertical());
				model->setRotation(tileMaterialModel->getRotation());


				int i = 0;
				for (glm::vec3 p : std::invoke(pointsGetFunc, model->getTile(), model->getTile()->getTranslation())) {
					projected[i++] = plane.project(p);
				}
				glm::vec2 min(std::numeric_limits<float>::max());
				glm::vec2 max(std::numeric_limits<float>::lowest());
				for (glm::vec2 p : projected) {
					min = glm::min(min, p);
					max = glm::max(max, p);
				}
				constexpr auto modV = [](float x) {
					x = std::fmodf(x, textureScale);
					if (x < 0.f) x += textureScale;
					return x;
				};

				//shift into bounds
				{
					glm::vec2 shift = glm::vec2(modV(min.x), modV(min.y)) - min;
					for (glm::vec2& p : projected) p += shift;
					min += shift;
					max += shift;
				}

				//compute the trapeze
				glm::vec2 segmentLeft(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
				glm::vec2 segmentRight(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
				const float center = glm::mix(min.x, max.x, 0.5f);
				for (glm::vec2 p : projected) {
					if (p.x < center) {
						segmentLeft.x = std::min(segmentLeft.x , p.y);
						segmentLeft.y = std::max(segmentLeft.y, p.y);
					}else {
						segmentRight.x = std::min(segmentRight.x, p.y);
						segmentRight.y = std::max(segmentRight.y, p.y);
					}
				}

				segmentLeft = glm::round(segmentLeft);
				segmentRight = glm::round(segmentRight);

				model->setU(min.x);
				model->setV(segmentLeft.x);
				model->setVRight(std::min(segmentRight.x, textureScale-1.f));

				model->setWidth(std::min(glm::distance(min.x, max.x), 255.f));
				model->setHeight(std::min(glm::distance(segmentLeft.x, segmentLeft.y), 255.f));
				model->setHeightRight(std::min(glm::distance(segmentRight.x, segmentRight.y), 255.f));
			}
		});
	}

	{
		auto btn = PG_new<PG_GuiButton>("clearTextureBtn", glm::vec2(0.f, 0.f), glm::vec2(40.f, 32.f), column);
		btn->setText("Clear texture");
		btn->setIcon("gui/icon_delete.png");
		btn->setBackground("M_GuiRoundedRectangleAll5");
		PG_GuiItem::anchor(btn, PG_Anchor::Right, column, PG_Anchor::Right);
		PG_GuiItem::anchor(btn, PG_Anchor::Left, column, PG_Anchor::Left);

		PG_Object::connect(btn, &PG_GuiButton::clicked, wrapper, [wrapper] {
			for (auto* item : wrapper->getSelectionItems()) {
				auto func = item->getFunction("void clearTexture()");
				if (!func) continue;
				func.callArgs(item);
			}
		});
	}
}

static inline void createForEventEdit(PG_GuiColumn* column, PG_SelectionWrapper* wrapper) {
	column->clearGuiChildren();

	auto selectionChangedProp = wrapper->registerSelectionChanged();

	auto addButton = PG_new<PG_GuiButton>("AddButton", glm::vec2(0.f), glm::vec2(100.f, 30.f), column);
	addButton->setIcon("gui/icon_add.png");
	addButton->setText("Add event");
	addButton->setBackground("M_GuiRoundedRectangleAll5");
	PG_GuiItem::anchor(addButton, PG_Anchor::Left, column, PG_Anchor::Left);
	PG_GuiItem::anchor(addButton, PG_Anchor::Right, column, PG_Anchor::Right);

	auto* listWrapper = PG_new<PG_GuiItem>("ListWrapper", column);
	listWrapper->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	PG_GuiItem::anchor(listWrapper, PG_Anchor::Left, column, PG_Anchor::Left);
	PG_GuiItem::anchor(listWrapper, PG_Anchor::Right, column, PG_Anchor::Right);

	auto listView = PG_new<PG_GuiListView>("PG_GuiListView", listWrapper);
	listView->fill(listWrapper);
	listView->setClip(true);
	listView->setItem(PG_GuiGridView::getDefaultCreateFunc<PG_DisaEventTileListItem>());
	listView->setItemUpdate(PG_GuiGridView::getDefaultUpdateFunc<PG_DisaEventTileListItem>());
	listView->setCellSize(35);

	auto infoText = PG_new<PG_GuiText>("PG_GuiText", listWrapper);
	infoText->setFontSize(21.f);
	infoText->setColor(PG_Color::hentaiBlueFrgb);
	infoText->setHorizontalAlignment(PG_Anchor::HorizontalCenter);
	infoText->setText("Multi selection is not supported");
	PG_GuiItem::anchor(infoText, PG_Anchor::Top, listWrapper, PG_Anchor::Top);
	PG_GuiItem::anchor(infoText, PG_Anchor::Left, listWrapper, PG_Anchor::Left);
	PG_GuiItem::anchor(infoText, PG_Anchor::Right, listWrapper, PG_Anchor::Right);


	PG_Object::connect(addButton, &PG_GuiButton::clicked, wrapper, [wrapper] {
		auto* item = dynamic_cast<EventTilePositionFilter*>(wrapper->getSelectionItems().front());
		item->addEvent();
	});

	auto setEventModel = [wrapper, listView, infoText, addButton] {
		if (wrapper->getSelectionItems().size() != 1) {
			listView->setModel(nullptr);
			addButton->setEnabled(false);
			infoText->setVisible(true);

		}
		else {
			listView->setModel(dynamic_cast<EventTilePositionFilter*>(wrapper->getSelectionItems().front()));
			addButton->setEnabled(true);
			infoText->setVisible(false);
		}
		};

	PG_Object::connect(wrapper, selectionChangedProp, listView, setEventModel);
	setEventModel();


}

constexpr float spinTexWidth = 50.f;


static inline void createForObjectEdit(PG_GuiColumn* column, PG_SelectionWrapper* wrapper) {
	column->clearGuiChildren();

	wrapper->registerProperty("posX", "objectDisaTranslation" ,PG_Any::from<int>(0), &getIVec3Value<&glm::ivec3::x>, &setIVec3Value<&glm::ivec3::x>);
	wrapper->registerProperty("posY", "objectDisaTranslation" ,PG_Any::from<int>(0), &getIVec3Value<&glm::ivec3::y>, &setIVec3Value<&glm::ivec3::y>);
	wrapper->registerProperty("posZ", "objectDisaTranslation" ,PG_Any::from<int>(0), &getIVec3Value<&glm::ivec3::z>, &setIVec3Value<&glm::ivec3::z>);

	wrapper->registerProperty("rotationX", "objectDisaRotation", PG_Any::from<int>(0), &getIVec3Value<&glm::ivec3::x>, &setIVec3Value<&glm::ivec3::x>);
	wrapper->registerProperty("rotationY", "objectDisaRotation", PG_Any::from<int>(0), &getIVec3Value<&glm::ivec3::y>, &setIVec3Value<&glm::ivec3::y>);
	wrapper->registerProperty("rotationZ", "objectDisaRotation", PG_Any::from<int>(0), &getIVec3Value<&glm::ivec3::z>, &setIVec3Value<&glm::ivec3::z>);

	wrapper->registerProperty("scaleX", "objectDisaScale", PG_Any::from<int>(0), &getIVec3Value<&glm::ivec3::x>, &setIVec3Value<&glm::ivec3::x>);
	wrapper->registerProperty("scaleY", "objectDisaScale", PG_Any::from<int>(0), &getIVec3Value<&glm::ivec3::y>, &setIVec3Value<&glm::ivec3::y>);
	wrapper->registerProperty("scaleZ", "objectDisaScale", PG_Any::from<int>(0), &getIVec3Value<&glm::ivec3::z>, &setIVec3Value<&glm::ivec3::z>);

	wrapper->registerProperty("scrollUvX", "uvScroll", PG_Any::from<int>(0), &getIVec2Value<&glm::ivec2::x>, &setIVec2Value<&glm::ivec2::x>);
	wrapper->registerProperty("scrollUvY", "uvScroll", PG_Any::from<int>(0), &getIVec2Value<&glm::ivec2::y>, &setIVec2Value<&glm::ivec2::y>);

	{
		auto row = createRow(column, "Translation:", 80.f);
		row->setMargin(PG_Anchor::Right, 5.f);
		row->setMargin(PG_Anchor::Left, 5.f);

		auto* spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int>(row, wrapper, "xSpinBox", "posX");
		PG_GuiSpinBox::setupInt(spinBox, -999999, 999999);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int>(row, wrapper, "ySpinBox", "posY");
		PG_GuiSpinBox::setupInt(spinBox, -999999, 999999);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRectangle");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int>(row, wrapper, "zRightSpinBox", "posZ");
		PG_GuiSpinBox::setupInt(spinBox, -999999, 999999);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
	}

	{
		auto row = createRow(column, "Rotation:", 80.f);
		row->setMargin(PG_Anchor::Right, 5.f);
		row->setMargin(PG_Anchor::Left, 5.f);

		auto* spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int>(row, wrapper, "xSpinBox", "rotationX");
		PG_GuiSpinBox::setupInt(spinBox, 0, 360);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int>(row, wrapper, "ySpinBox", "rotationY");
		PG_GuiSpinBox::setupInt(spinBox, 0, 360);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRectangle");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int>(row, wrapper, "zRightSpinBox", "rotationZ");
		PG_GuiSpinBox::setupInt(spinBox, 0, 360);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
	}

	{
		auto row = createRow(column, "Scale:", 80.f);
		row->setMargin(PG_Anchor::Right, 5.f);
		row->setMargin(PG_Anchor::Left, 5.f);

		auto* spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int>(row, wrapper, "xSpinBox", "scaleX");
		PG_GuiSpinBox::setupInt(spinBox, 1, 99999);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int>(row, wrapper, "ySpinBox", "scaleY");
		PG_GuiSpinBox::setupInt(spinBox, 1, 99999);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRectangle");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int>(row, wrapper, "zRightSpinBox", "scaleZ");
		PG_GuiSpinBox::setupInt(spinBox, 1, 99999);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
	}

	{
		auto row = createRow(column, "UV Scroll:", 80.f);
		row->setMargin(PG_Anchor::Right, 5.f);
		row->setMargin(PG_Anchor::Left, 5.f);

		auto* spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int>(row, wrapper, "xUVSpinBox", "scrollUvX");
		PG_GuiSpinBox::setupInt(spinBox, -255, 255);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);

		spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, int>(row, wrapper, "yUVSpinBox", "scrollUvY");
		PG_GuiSpinBox::setupInt(spinBox, -255, 255);
		spinBox->setSize(glm::vec2(spinTexWidth, 32.f));
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");
		spinBox->setLayoutProperty(PG_GuiSpinBox::LayoutProperty_Flow, PG_Anchor::Right);
	}

	{
		auto duplicateButton = PG_new<PG_GuiButton>("DuplicateButton", glm::vec2(0.f), glm::vec2(100.f, 30.f), column);
		duplicateButton->setIcon("gui/icon_add.png");
		duplicateButton->setText("Duplicate");
		duplicateButton->setBackground("M_GuiRoundedRectangleAll5");
		PG_GuiItem::anchor(duplicateButton, PG_Anchor::Left, column, PG_Anchor::Left);
		PG_GuiItem::anchor(duplicateButton, PG_Anchor::Right, column, PG_Anchor::Right);

		PG_GuiItem::connect(duplicateButton, &PG_GuiButton::clicked, wrapper, [wrapper] {
			for (auto* obj : wrapper->getSelectionItems()) {
				auto val = obj->getPropertyValue("index");
				if (!val) continue;
				PG_DisaMapEditorMainModel::get()->dublicateModelPressed(val.toInt());
			}

			});
	}
}

constexpr glm::vec3 g_disaTextColor = PG_Color::rgbToF(glm::rgb(70, 52, 48));

inline void setLableText(PG_GuiText* text) {
	text->setWrapMode(PG_GuiText::EWrapMode::None);
	text->setWidth(200.f);
	text->setVerticalAlignment(PG_GuiText::ETextAlignment::VerticalCenter);
	text->setColor(glm::rgb(255, 190, 80));
	text->setBorderColor(glm::vec3(0.f));
	text->setMaterial(PG_Material("M_GuiTextBorderSDF"));
	text->setFontSize(32.f);
	text->setFont("BalooThambi/BalooThambi-Regular.ttf");
}

inline void setText(PG_GuiText* text) {
	text->setWrapMode(PG_GuiText::EWrapMode::Elide);
	text->setVerticalAlignment(PG_GuiText::ETextAlignment::VerticalCenter);
	text->setFontSize(47.f);
	text->setColor(g_disaTextColor);
}

static inline void createForActorNoneSelectEdit(PG_GuiColumn* column) {
	column->clearGuiChildren();

	auto filterModel = PG_new<FilterUniqueModel>("FilterUniqueModel", column);
	filterModel->setModel(PG_DisaCharactersModel::get());


	{
		PG_GuiItem* searchArea = PG_new<PG_GuiItem>("AearchArea", column);
		PG_GuiItem::anchor(searchArea, PG_Anchor::Left, column, PG_Anchor::Left);
		PG_GuiItem::anchor(searchArea, PG_Anchor::Right, column, PG_Anchor::Right);
		searchArea->setHeight(30.f);

		auto filterBtn = PG_new<PG_GuiToggleButton>("filterBtn", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), searchArea);
		filterBtn->setIcon("gui/icon_eye.png");
		filterBtn->setBackground("M_GuiToolButtonEnd");
		filterBtn->setTooltipText("Show All");
		filterBtn->setToggled(filterModel->isShowAll());
		PG_GuiItem::anchor(filterBtn, PG_Anchor::Top, searchArea, PG_Anchor::Top);
		PG_GuiItem::anchor(filterBtn, PG_Anchor::Right, searchArea, PG_Anchor::Right);
		PG_GuiItem::connect(filterBtn, &PG_GuiToggleButton::toggleChanged, filterModel, [filterModel, s = filterBtn] { filterModel->setShowAll(s->isToggled()); });

		PG_DisaSearchField* searchField = PG_new<PG_DisaSearchField>("searchField", glm::vec2(0.f), glm::vec2(300.f, 30.f), searchArea);
		PG_GuiItem::anchor(searchField, PG_Anchor::Top, searchArea, PG_Anchor::Top);
		PG_GuiItem::anchor(searchField, PG_Anchor::Right, filterBtn, PG_Anchor::Left);
		PG_GuiItem::anchor(searchField, PG_Anchor::Left, searchArea, PG_Anchor::Left);
		searchField->setBackground("M_GuiRoundedRectangleLeft5");
		PG_GuiItem::connect(searchField, &PG_GuiTextField::textChanged, filterModel, [filterModel, s = searchField] { filterModel->setSearchText(s->getText()); });
	}
	

	auto* gridWrapper = PG_new<PG_GuiItem>("grid", column);
	gridWrapper->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	PG_GuiItem::anchor(gridWrapper, PG_Anchor::Left, column, PG_Anchor::Left);
	PG_GuiItem::anchor(gridWrapper, PG_Anchor::Right, column, PG_Anchor::Right);

	auto gridView = PG_new<PG_GuiGridView>("grid", gridWrapper);
	gridView->fill(gridWrapper);
	gridView->setClip(true);
	gridView->setItem(PG_GuiGridView::getDefaultCreateFunc<PG_DisaCharacterPortraitGridTile>());
	gridView->setItemUpdate(PG_GuiGridView::getDefaultUpdateFunc<PG_DisaCharacterPortraitGridTile>());
	gridView->setCellSize(glm::ivec2((inspectorWidth - 20)/3));
	gridView->setModel(filterModel);
	PG_new<DisaNewCharacterDrag>("DisaNewCharacterDrag", gridView);

	PG_GuiScrollBar* scroll = PG_new<PG_GuiScrollBar>("ScrollBar", gridWrapper);
	PG_GuiItem::anchor(scroll, PG_Anchor::Top, gridView, PG_Anchor::Top);
	PG_GuiItem::anchor(scroll, PG_Anchor::Bottom, gridView, PG_Anchor::Bottom);
	PG_GuiItem::anchor(scroll, PG_Anchor::Right, gridView, PG_Anchor::Right);

	constexpr glm::vec3 blueColor = glm::vec3(133.f, 208.f, 255.f) / 255.f;
	scroll->setSize(glm::vec2(10.0f, 0.0f));
	scroll->setScrollBarColors(PG_Gui::ColorSet{ blueColor, blueColor * 1.1f , blueColor * 1.1f });
	if (auto rec = dynamic_cast<PG_GuiRectangle*>(scroll->getHandle())) {
		rec->setMaterial("M_GuiRoundedRectangleAll5");
	}
	gridView->setupVerticalScrollBar(scroll);
}

static inline void createForActorEdit(PG_GuiColumn* column, PG_SelectionWrapper* wrapper) {
	struct RotationToIndex {
		PG_Any operator()(const PG_Any& v) const {
			return int(v.get<char>() + 1);
		}
	};

	struct IndexToRotation {
		PG_Any operator()(const PG_Any& v) const {
			return char(glm::clamp(v.get<int>() - 1, -1, 3));
		}
	};

	struct WeaponIdToIndex {
		PG_Any operator()(const PG_Any& v) const {
			auto weaponId = v.get<unsigned short>();
			auto findIt = std::ranges::find(*PG_DisaItemsModel::get(), weaponId, &PG_DisaItemModel::id);
			if (findIt == PG_DisaItemsModel::get()->end()) return (int)-1;
			return (int)std::distance(PG_DisaItemsModel::get()->begin(), findIt);
		}
	};

	struct IndexToWeaponId {
		PG_Any operator()(const PG_Any& v) const {
			int index = v.get<int>();
			if (index < 0) return (unsigned short)0;
			const PG_DisaItemsModel* itemsModel = PG_DisaItemsModel::get();
			return itemsModel->PG_ListModel<PG_DisaItemModel>::get(index).id;
		}
	};

	struct MagicIdToIndex {
		PG_Any operator()(const PG_Any& v) const {
			auto weaponId = v.get<unsigned short>();
			auto findIt = std::ranges::find(*PG_DisaMagicsModel::get(), weaponId, &PG_DisaMagicModel::id);
			if (findIt == PG_DisaMagicsModel::get()->end()) return (int)-1;
			return (int)std::distance(PG_DisaMagicsModel::get()->begin(), findIt);
		}
	};

	struct IndexToMagicId {
		PG_Any operator()(const PG_Any& v) const {
			int index = v.get<int>();
			if (index < 0) return (unsigned short)0;
			const PG_DisaMagicsModel* itemsModel = PG_DisaMagicsModel::get();
			return itemsModel->PG_ListModel<PG_DisaMagicModel>::get(index).id;
		}
	};

	struct GeoEffectIdToIndex {
		PG_Any operator()(const PG_Any& v) const {
			auto weaponId = v.get<unsigned short>();
			auto findIt = std::ranges::find(*PG_DisaGeoEffectsModel::get(), weaponId, &PG_DisaGeoEffectModel::id);
			if (findIt == PG_DisaGeoEffectsModel::get()->end()) return (int)-1;
			return (int)std::distance(PG_DisaGeoEffectsModel::get()->begin(), findIt);
		}
	};

	struct IndexToGeoEffectId {
		PG_Any operator()(const PG_Any& v) const {
			int index = v.get<int>();
			if (index < 0) return (unsigned short)0;
			const PG_DisaGeoEffectsModel* itemsModel = PG_DisaGeoEffectsModel::get();
			return itemsModel->PG_ListModel<PG_DisaGeoEffectModel>::get(index).id;
		}
	};

	column->clearGuiChildren();

	wrapper->registerProperty("characterId", PG_Any::from<unsigned short>(1));
	wrapper->registerProperty("characterName", PG_Any::from<std::string>("Multi Selection"));
	wrapper->registerProperty("characterTitle", PG_Any::from<std::string>("Multi Selection"));
	wrapper->registerProperty("characterPortrait", PG_Any::from<PG_Texture>(PG_Texture("gui/disa/disaMissingno.png")));
	wrapper->registerProperty("characterLevel", PG_Any::from<unsigned short>(1));
	wrapper->registerProperty("characterRotation", PG_Any::from<char>(1), &PG_PropertyDynamicWrapper::getValueRemap<RotationToIndex>, &PG_PropertyDynamicWrapper::setValueRemap<IndexToRotation>);
	wrapper->registerProperty("characterAlliance", PG_Any::from<unsigned char>(0), &PG_PropertyDynamicWrapper::getValueConvert<unsigned char, int>, &PG_PropertyDynamicWrapper::setValueConvert<int, unsigned char>);
	wrapper->registerProperty("characterAi", PG_Any::from<unsigned char>(0), &PG_PropertyDynamicWrapper::getValueConvert<unsigned char, int>, &PG_PropertyDynamicWrapper::setValueConvert<int, unsigned char>);
	wrapper->registerProperty("characterWeapon", PG_Any::from<unsigned short>(1));
	wrapper->registerProperty("characterWeaponIndex", "characterWeapon", PG_Any::from<unsigned int>(0), &PG_PropertyDynamicWrapper::getValueRemap<WeaponIdToIndex>, &PG_PropertyDynamicWrapper::setValueRemap<IndexToWeaponId>);

	wrapper->registerProperty("characterItemSlot0", PG_Any::from<unsigned short>(1));
	wrapper->registerProperty("characterItemSlot0Index", "characterItemSlot0", PG_Any::from<unsigned int>(0), &PG_PropertyDynamicWrapper::getValueRemap<WeaponIdToIndex>, &PG_PropertyDynamicWrapper::setValueRemap<IndexToWeaponId>);
	wrapper->registerProperty("characterItemSlot1", PG_Any::from<unsigned short>(1));
	wrapper->registerProperty("characterItemSlot1Index", "characterItemSlot1", PG_Any::from<unsigned int>(0), &PG_PropertyDynamicWrapper::getValueRemap<WeaponIdToIndex>, &PG_PropertyDynamicWrapper::setValueRemap<IndexToWeaponId>);
	wrapper->registerProperty("characterItemSlot2", PG_Any::from<unsigned short>(1));
	wrapper->registerProperty("characterItemSlot2Index", "characterItemSlot2", PG_Any::from<unsigned int>(0), &PG_PropertyDynamicWrapper::getValueRemap<WeaponIdToIndex>, &PG_PropertyDynamicWrapper::setValueRemap<IndexToWeaponId>);


	wrapper->registerProperty("characterMagicSlot0", PG_Any::from<unsigned short>(1));
	wrapper->registerProperty("characterMagicSlot0Index", "characterMagicSlot0", PG_Any::from<unsigned int>(0), &PG_PropertyDynamicWrapper::getValueRemap<MagicIdToIndex>, &PG_PropertyDynamicWrapper::setValueRemap<IndexToMagicId>);
	wrapper->registerProperty("characterMagicSlot1", PG_Any::from<unsigned short>(1));
	wrapper->registerProperty("characterMagicSlot1Index", "characterMagicSlot1", PG_Any::from<unsigned int>(0), &PG_PropertyDynamicWrapper::getValueRemap<MagicIdToIndex>, &PG_PropertyDynamicWrapper::setValueRemap<IndexToMagicId>);
	wrapper->registerProperty("characterMagicSlot2", PG_Any::from<unsigned short>(1));
	wrapper->registerProperty("characterMagicSlot2Index", "characterMagicSlot2", PG_Any::from<unsigned int>(0), &PG_PropertyDynamicWrapper::getValueRemap<MagicIdToIndex>, &PG_PropertyDynamicWrapper::setValueRemap<IndexToMagicId>);
	wrapper->registerProperty("characterMagicSlot3", PG_Any::from<unsigned short>(1));
	wrapper->registerProperty("characterMagicSlot3Index", "characterMagicSlot3", PG_Any::from<unsigned int>(0), &PG_PropertyDynamicWrapper::getValueRemap<MagicIdToIndex>, &PG_PropertyDynamicWrapper::setValueRemap<IndexToMagicId>);
	
	wrapper->registerProperty("characterMagicLevel0", "characterMagicLevels", PG_Any::from<unsigned char>(0), &getVecValue<glm::u8vec4, unsigned char, &glm::u8vec4::x>, &setVecValue<glm::u8vec4, unsigned char, &glm::u8vec4::x>);
	wrapper->registerProperty("characterMagicLevel1", "characterMagicLevels", PG_Any::from<unsigned char>(0), &getVecValue<glm::u8vec4, unsigned char, &glm::u8vec4::y>, &setVecValue<glm::u8vec4, unsigned char, &glm::u8vec4::y>);
	wrapper->registerProperty("characterMagicLevel2", "characterMagicLevels", PG_Any::from<unsigned char>(0), &getVecValue<glm::u8vec4, unsigned char, &glm::u8vec4::z>, &setVecValue<glm::u8vec4, unsigned char, &glm::u8vec4::z>);
	wrapper->registerProperty("characterMagicLevel3", "characterMagicLevels", PG_Any::from<unsigned char>(0), &getVecValue<glm::u8vec4, unsigned char, &glm::u8vec4::w>, &setVecValue<glm::u8vec4, unsigned char, &glm::u8vec4::w>);

	wrapper->registerProperty("characterGeoColor", PG_Any::from<unsigned int>(0));
	wrapper->registerProperty("characterGeoEffect", PG_Any::from<unsigned short>(0));
	wrapper->registerProperty("characterGeoEffectIndex","characterGeoEffect", PG_Any::from<unsigned short>(0), &PG_PropertyDynamicWrapper::getValueRemap<GeoEffectIdToIndex>, &PG_PropertyDynamicWrapper::setValueRemap<IndexToGeoEffectId>);

	PG_GuiImage* frame = PG_new<PG_GuiImage>("NameFrame", column);
	frame->setMaterial(PG_Material("M_DisaGuiImageFrameB"));
	frame->setHeight(80.f);
	PG_GuiItem::anchor(frame, PG_Anchor::Right, column, PG_Anchor::Right);
	PG_GuiItem::anchor(frame, PG_Anchor::Left, column, PG_Anchor::Left);

	{

		auto spinBoxCharacterId = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned short>(frame, wrapper, "characterIdSpinBox", "characterId");
		spinBoxCharacterId->setWidth(120.f);
		spinBoxCharacterId->getBackground()->setClip(false);
		spinBoxCharacterId->getBackground()->setAlpha(0.f);
		spinBoxCharacterId->getText()->setColor(g_disaTextColor);
		spinBoxCharacterId->getText()->setMargin(PG_Anchor::Left, 0.f);
		spinBoxCharacterId->getText()->setFontSize(35.f);
		spinBoxCharacterId->getText()->setWrapMode(PG_GuiText::EWrapMode::None);
		spinBoxCharacterId->getText()->setHorizontalAlignment(PG_GuiText::ETextAlignment::Left);
		PG_GuiSpinBox::setupInt(spinBoxCharacterId, 0, 65530);

		PG_GuiItem::anchor(spinBoxCharacterId, PG_Anchor::Right, frame, PG_Anchor::Right);
		PG_GuiItem::anchor(spinBoxCharacterId, PG_Anchor::Top, frame, PG_Anchor::Top);
		PG_GuiItem::anchor(spinBoxCharacterId, PG_Anchor::Bottom, frame, PG_Anchor::Bottom);


		PG_GuiText* text = PG_new<PG_GuiText>("Name", frame);
		PG_GuiItem::anchor(text, PG_Anchor::Right, spinBoxCharacterId, PG_Anchor::Left);
		PG_GuiItem::anchor(text, PG_Anchor::Left, frame, PG_Anchor::Left);
		PG_GuiItem::anchor(text, PG_Anchor::Top, frame, PG_Anchor::Top);
		PG_GuiItem::anchor(text, PG_Anchor::Bottom, frame, PG_Anchor::Bottom);
		setText(text);
		text->setMargin(PG_Anchor::Left, 25.f);

		PG_Gui::connectChanged(text,
			wrapper, "characterName", [](PG_GuiText* item, const PG_Any& value) {
				item->setText(value.get<std::string>());
			});

		//lables
		PG_GuiText* label = PG_new<PG_GuiText>("NameLabel", frame);
		PG_GuiItem::anchor(label, PG_Anchor::Bottom, frame, PG_Anchor::Top);
		PG_GuiItem::anchor(label, PG_Anchor::Left, text, PG_Anchor::Left);
		setLableText(label);
		label->setPosition(glm::vec2(0.f, 5.f));
		label->setText("NAME");

		label = PG_new<PG_GuiText>("IDLabel", frame);
		PG_GuiItem::anchor(label, PG_Anchor::Bottom, frame, PG_Anchor::Top);
		PG_GuiItem::anchor(label, PG_Anchor::Left, spinBoxCharacterId, PG_Anchor::Left);
		setLableText(label);
		label->setPosition(glm::vec2(0.f, 5.f));
		label->setText("ID");
	}

	frame = PG_new<PG_GuiImage>("BaseStatsFrame", column);
	frame->setMaterial(PG_Material("M_DisaGuiImageFrameA"));
	frame->setHeight(190.f);
	PG_GuiItem::anchor(frame, PG_Anchor::Right, column, PG_Anchor::Right);
	PG_GuiItem::anchor(frame, PG_Anchor::Left, column, PG_Anchor::Left);

	{
		PG_GuiImage* img = PG_new<PG_GuiImage>("face", frame);
		img->setCropSize(glm::vec2(160.f));
		PG_GuiItem::anchor(img, PG_Anchor::Top, frame, PG_Anchor::Top);
		PG_GuiItem::anchor(img, PG_Anchor::Left, frame, PG_Anchor::Left);
		img->setPosition(glm::vec2(15.f));
		img->setSize(glm::vec2(160.f));

		PG_Gui::connectChanged(img, wrapper, "characterPortrait", [](PG_GuiImage* item, const PG_Any& value) {
				item->setSourceData(value.get<PG_Texture>());
			});
			
		PG_GuiText* label = PG_new<PG_GuiText>("ClassLabel", frame);
		PG_GuiItem::anchor(label, PG_Anchor::Top, img, PG_Anchor::Top);
		PG_GuiItem::anchor(label, PG_Anchor::Left, img, PG_Anchor::Right);
		setLableText(label);
		label->setHeight(34.f);
		label->setWidth(80.f);
		label->setPosition(glm::vec2(15.f, 0.f));
		label->setText("CLASS");

		PG_GuiText* text = PG_new<PG_GuiText>("ClassText", frame);
		PG_GuiItem::anchor(text, PG_Anchor::Top, label, PG_Anchor::Top);
		PG_GuiItem::anchor(text, PG_Anchor::Left, label, PG_Anchor::Right);
		PG_GuiItem::anchor(text, PG_Anchor::Right, frame, PG_Anchor::Right);
		setText(text);
		text->setFontSize(32.f);
		text->setHeight(34.f);
		text->setMargin(PG_Anchor::Left, 15.f);

		PG_Gui::connectChanged(text,
			wrapper, "characterTitle", [](PG_GuiText* item, const PG_Any& value) {
				item->setText(value.get<std::string>());
			});

		PG_GuiText* label2 = PG_new<PG_GuiText>("LevelLabel", frame);
		PG_GuiItem::anchor(label2, PG_Anchor::Top, label, PG_Anchor::Bottom);
		PG_GuiItem::anchor(label2, PG_Anchor::Left, img, PG_Anchor::Right);
		setLableText(label2);
		label2->setHeight(34.f);
		label2->setWidth(80.f);
		label2->setPosition(glm::vec2(15.f, 5.f));
		label2->setText("LV");

		auto lvSpinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned short>(frame, wrapper, "LevelText", "characterLevel");
		PG_GuiSpinBox::setupInt(lvSpinBox, 1, std::numeric_limits<unsigned short>::max());
		PG_GuiItem::anchor(lvSpinBox, PG_Anchor::Top, label2, PG_Anchor::Top);
		PG_GuiItem::anchor(lvSpinBox, PG_Anchor::Left, text, PG_Anchor::Left);
		PG_GuiItem::anchor(lvSpinBox, PG_Anchor::Right, frame, PG_Anchor::Right);
		setText(lvSpinBox->getText());
		lvSpinBox->getText()->setFontSize(35.f);
		lvSpinBox->getText()->setHorizontalAlignment(PG_Anchor::Left);
		lvSpinBox->getText()->setMargin(PG_Anchor::Left, 0.f);
		lvSpinBox->getBackground()->setClip(false);
		lvSpinBox->getBackground()->setAlpha(0.f);
		lvSpinBox->setHeight(34.f);
	}

	{

		PG_GuiRow* row = createRow(column, "Rotation:");
		auto dropDown = PG_Gui::createGuiElementV2<PG_GuiDropDown>(row, wrapper, "mobilityDropDown", "characterRotation", "index",
			[](const std::string& name, PG_GuiItem* root) {
				auto dropDown = PG_new<PG_GuiDropDown>(name, glm::vec2(0.f, 0.f), glm::vec2(40.f, 32.f), root);
				auto model = PG_new<PG_ListModel<std::string>>(dropDown);

				model->set({
					{ "North"},
					{ "East"},
					{ "South"},
					{ "West"},
					});
				dropDown->setModel(model);
				return dropDown;
			});
		dropDown->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	}

	{
		PG_GuiRow* row = createRow(column, "Alliance:");
		auto dropDown = PG_Gui::createGuiElementV2<PG_GuiDropDown>(row, wrapper, "mobilityDropDown", "characterAlliance", "index",
			[](const std::string& name, PG_GuiItem* root) {
				auto dropDown = PG_new<PG_GuiDropDown>(name, glm::vec2(0.f, 0.f), glm::vec2(40.f, 32.f), root);
				auto model = PG_new<PG_ListModel<std::string>>(dropDown);

				model->set({
					{ "User controlled"},
					{ "Enemy"},
					{ "Enemy, third party"},
					{ "Passive"},
					{ "Neutral"},
					});
				dropDown->setModel(model);
				return dropDown;
			});
		dropDown->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	}

	{
		PG_GuiRow* row = createRow(column, "Ai:");
		auto dropDown = PG_Gui::createGuiElementV2<PG_GuiDropDown>(row, wrapper, "aiDropDown", "characterAi", "index",
			[](const std::string& name, PG_GuiItem* root) {
				auto dropDown = PG_new<PG_GuiDropDown>(name, glm::vec2(0.f, 0.f), glm::vec2(40.f, 32.f), root);
				auto model = PG_new<PG_ListModel<std::string>>(dropDown);

				model->set({
					{ "Disabled"},
					{ "Guard"},
					{ "Attack"},
					{ "Hold Ground"},
					});
				dropDown->setModel(model);
				PG_GuiDropDown::connect(dropDown, &PG_GuiDropDown::indexChanged, dropDown, [dropDown](int index) {
					switch (index)
					{
					case 0: dropDown->setTooltipText("Does nothing"); break;
					case 1: dropDown->setTooltipText("The AI holds its position, engaging only if attacked or if an enemy enters its range."); break;
					case 2: dropDown->setTooltipText("The AI actively advances toward enemies to brutally fight."); break;
					case 3: dropDown->setTooltipText("The AI remains stationary, attacking only enemies that come within weapon range."); break;
					default:
						break;
					}
				});
				
				return dropDown;
			});
		dropDown->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	}
	PG_new<PG_GuiItem>("spacerItem", glm::vec2(0.f), glm::vec2(12.f), column);
	{
		const auto addItemSelection = [&](const char* title, const char* slot, const char* slotIndex) {
			PG_GuiRow* row = createRow(column, title);

			auto spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned short>(row, wrapper, "itemID", slot);
			spinBox->setWidth(80.f);
			spinBox->setHeight(32.f);
			PG_GuiSpinBox::setupInt(spinBox, 0, std::numeric_limits<unsigned short>::max());
			spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");

			auto dropDown = PG_Gui::createGuiElementV2<PG_DisaItemDropDown>(row, wrapper, "itemDropDown", slotIndex, "index",
				[](const std::string& name, PG_GuiItem* root) {
					auto dropDown = PG_new<PG_DisaItemDropDown>(name, glm::vec2(0.f, 0.f), glm::vec2(40.f, 32.f), root);
					dropDown->setModel(PG_DisaItemsModel::get());
					return dropDown;
				});
			dropDown->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
			dropDown->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");
			};

		addItemSelection("Weapon ID:", "characterWeapon", "characterWeaponIndex");
		addItemSelection("Item 1 ID:", "characterItemSlot0", "characterItemSlot0Index");
		addItemSelection("Item 2 ID:", "characterItemSlot1", "characterItemSlot1Index");
		addItemSelection("Item 3 ID:", "characterItemSlot2", "characterItemSlot2Index");
	}
	PG_new<PG_GuiItem>("spacerItem", glm::vec2(0.f), glm::vec2(12.f), column);
	{
		const auto addMagicSelection = [&](const char* title, const char* slot, const char* slotIndex, const char* slotLevel) {
			PG_GuiRow* row = createRow(column, title, 160.f);

			auto spinBoxLevel = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned char>(row, wrapper, "magicLevel", slotLevel);
			spinBoxLevel->setWidth(80.f);
			spinBoxLevel->setHeight(32.f);
			PG_GuiSpinBox::setupInt(spinBoxLevel, 0, std::numeric_limits<unsigned char>::max());
			spinBoxLevel->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");

			auto spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned short>(row, wrapper, "magicID", slot);
			spinBox->setWidth(80.f);
			spinBox->setHeight(32.f);
			PG_GuiSpinBox::setupInt(spinBox, 0, std::numeric_limits<unsigned short>::max());
			spinBox->getBackground()->setMaterial("M_GuiRoundedRectangle");


			auto dropDown = PG_Gui::createGuiElementV2<PG_DisaItemDropDown>(row, wrapper, "magicDropDown", slotIndex, "index",
				[](const std::string& name, PG_GuiItem* root) {
					auto dropDown = PG_new<PG_DisaItemDropDown>(name, glm::vec2(0.f, 0.f), glm::vec2(40.f, 32.f), root);
					dropDown->setModel(PG_DisaMagicsModel::get());
					return dropDown;
				});
			dropDown->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
			dropDown->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");



			};

		addMagicSelection("Magic 1 Lv/ID:", "characterMagicSlot0", "characterMagicSlot0Index", "characterMagicLevel0");
		addMagicSelection("Magic 2 Lv/ID:", "characterMagicSlot1", "characterMagicSlot1Index", "characterMagicLevel1");
		addMagicSelection("Magic 3 Lv/ID:", "characterMagicSlot2", "characterMagicSlot2Index", "characterMagicLevel2");
		addMagicSelection("Magic 4 Lv/ID:", "characterMagicSlot3", "characterMagicSlot3Index", "characterMagicLevel3");
	}

	{
		auto text = PG_new<PG_GuiText>("Text", glm::vec2(0.f), glm::vec2(300, 32.f), column);
		text->setColor(PG_Color::hentaiBlueFrgb);
		text->setFontSize(21.f);
		text->setWrapMode(PG_GuiText::None);
		text->setVerticalAlignment(PG_GuiText::ETextAlignment::VerticalCenter);
		text->setText("Only applies to geo symbols:"); //2800

	}
	{
		PG_GuiRow* row = createRow(column, "Geo color:");
		auto dropDown = PG_Gui::createGuiElementV2<PG_GuiDropDown>(row, wrapper, "geoColorDropDown", "characterGeoColor", "index",
			[](const std::string& name, PG_GuiItem* root) {
				auto dropDown = PG_new<PG_GuiDropDown>(name, glm::vec2(0.f, 0.f), glm::vec2(40.f, 32.f), root);
				auto model = PG_new<PG_ListModel<std::string>>(dropDown);

				model->set({
					{ "None"},
					{ "Red"},
					{ "Green"},
					{ "Blue"},
					{ "Yellow"},
					{ "Magenta"},
					{ "Cyan"},
					{ "White"}
					});
				dropDown->setModel(model);

				return dropDown;
			});
		dropDown->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	}

	{
		PG_GuiRow* row = createRow(column, "Geo effect ID:");

		auto spinBox = PG_Gui::createSpinBoxV2<PG_GuiSpinBox, unsigned short>(row, wrapper, "effectID", "characterGeoEffect");
		spinBox->setWidth(80.f);
		spinBox->setHeight(32.f);
		PG_GuiSpinBox::setupInt(spinBox, 0, std::numeric_limits<unsigned short>::max());
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");

		auto dropDown = PG_Gui::createGuiElementV2<PG_GuiDropDown>(row, wrapper, "itemDropDown", "characterGeoEffectIndex", "index",
			[](const std::string& name, PG_GuiItem* root) {
				auto dropDown = PG_new<PG_GuiDropDown>(name, glm::vec2(0.f, 0.f), glm::vec2(40.f, 32.f), root);
				dropDown->setModel(PG_DisaGeoEffectsModel::get());

				PG_GuiDropDown::connect(dropDown, &PG_GuiDropDown::indexChanged, dropDown, [dropDown](int index) {
					if (index < 0 || index >= PG_DisaGeoEffectsModel::get()->size()) {
						dropDown->setTooltipText({});
						return;
					}
					dropDown->setTooltipText((PG_DisaGeoEffectsModel::get()->begin() + index)->getTooltip());
					});

				return dropDown;
			});
		dropDown->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
		dropDown->getBackground()->setMaterial("M_GuiRoundedRectangleRight5");

	}
}

void Inspector::createFunctionBar() {
	auto toolbar = root->findFirstChild<PG_GuiColumn>("Tools");

	if (mod->getMode() == PG_DisaEditorModule::Mode_View) {
		if (toolbar) delete toolbar;
		if (auto editModeColumn = root->findFirstChild<PG_GuiColumn>("EditMode"))
			delete editModeColumn;
		return;
	}

	if (toolbar) return;

	toolbar = PG_new<PG_GuiColumn>("Tools", root);
	toolbar->setWidth(42.f);
	toolbar->setPosition(glm::vec2(10.f));
	toolbar->setSpacing(3.f);
	PG_GuiItem::anchor(toolbar, PG_Anchor::Top, root, PG_Anchor::Top);
	PG_GuiItem::anchor(toolbar, PG_Anchor::Bottom, root, PG_Anchor::Bottom);
	PG_GuiItem::anchor(toolbar, PG_Anchor::Left, root, PG_Anchor::Left);

	auto btn = PG_new<PG_GuiToggleButton>("TerrainEdit", toolbar);
	btn->setSize(glm::vec2(42));
	btn->setIcon("gui/icon_smallTiles.png");
	btn->setTooltipText("Terrain edit");
	btn->setTooltipPositionHint(ETooltipPositionHint::PG_TooltipPositionHint_Right);
	btn->setBackground("M_GuiRoundedRectangleTop5");
	btn->setAutoToggle(false);
	btn->setToggled(mod->getMode() == PG_DisaEditorModule::Mode_Terrain);

	connect(mod, &PG_DisaEditorModule::modeChanged, btn, [btn](PG_DisaEditorModule::EMode mode) {
		btn->setToggled(mode == PG_DisaEditorModule::Mode_Terrain);
		});
	connect(btn, &PG_GuiToggleButton::clicked, this, [this] {
		mod->setMode(PG_DisaEditorModule::Mode_Terrain);
		});

	btn = PG_new<PG_GuiToggleButton>("MaterialEdit", toolbar);
	btn->setSize(glm::vec2(42));
	btn->setIcon("gui/icon_texture.png");
	btn->setTooltipText("Material edit");
	btn->setTooltipPositionHint(ETooltipPositionHint::PG_TooltipPositionHint_Right);
	btn->setAutoToggle(false);
	btn->setToggled(mod->getMode() == PG_DisaEditorModule::Mode_Material);

	connect(mod, &PG_DisaEditorModule::modeChanged, btn, [btn](PG_DisaEditorModule::EMode mode) {
		btn->setToggled(mode == PG_DisaEditorModule::Mode_Material);
		});
	connect(btn, &PG_GuiToggleButton::clicked, this, [this] {
		mod->setMode(PG_DisaEditorModule::Mode_Material);
		});

	btn = PG_new<PG_GuiToggleButton>("ObjectEdit", toolbar);
	btn->setSize(glm::vec2(42));
	btn->setIcon("gui/icon_model.png");
	btn->setTooltipText("Object edit");
	btn->setTooltipPositionHint(PG_TooltipPositionHint_Right);
	btn->setAutoToggle(false);
	btn->setToggled(mod->getMode() == PG_DisaEditorModule::Mode_Object);

	connect(mod, &PG_DisaEditorModule::modeChanged, btn, [btn](PG_DisaEditorModule::EMode mode) {
		btn->setToggled(mode == PG_DisaEditorModule::Mode_Object);
		});
	connect(btn, &PG_GuiToggleButton::clicked, this, [this] {
		mod->setMode(PG_DisaEditorModule::Mode_Object);
		});

	btn = PG_new<PG_GuiToggleButton>("EventEdit", toolbar);
	btn->setSize(glm::vec2(42));
	btn->setIcon("gui/disa/icon_eventTile.png");
	btn->setTooltipText("Event edit");
	btn->setTooltipPositionHint(ETooltipPositionHint::PG_TooltipPositionHint_Right);
	btn->setAutoToggle(false);
	btn->setToggled(mod->getMode() == PG_DisaEditorModule::Mode_EventTile);

	connect(mod, &PG_DisaEditorModule::modeChanged, btn, [btn](PG_DisaEditorModule::EMode mode) {
		btn->setToggled(mode == PG_DisaEditorModule::Mode_EventTile);
		});
	connect(btn, &PG_GuiToggleButton::clicked, this, [this] {
		mod->setMode(PG_DisaEditorModule::Mode_EventTile);
		});

	btn = PG_new<PG_GuiToggleButton>("ActorEdit", toolbar);
	btn->setSize(glm::vec2(42));
	btn->setIcon("gui/icon_actor.png");
	btn->setTooltipText("Actor edit");
	btn->setTooltipPositionHint(ETooltipPositionHint::PG_TooltipPositionHint_Right);
	btn->setBackground("M_GuiRoundedRectangleBottom5");
	btn->setAutoToggle(false);
	btn->setToggled(mod->getMode() == PG_DisaEditorModule::Mode_Actor);

	connect(mod, &PG_DisaEditorModule::modeChanged, btn, [btn](PG_DisaEditorModule::EMode mode) {
		btn->setToggled(mode == PG_DisaEditorModule::Mode_Actor);
		});
	connect(btn, &PG_GuiToggleButton::clicked, this, [this] {
		mod->setMode(PG_DisaEditorModule::Mode_Actor);
		});

	{ //side
		auto editModeColumn = PG_new<PG_GuiColumn>("EditMode", root);
		editModeColumn->setWidth(32.f);
		editModeColumn->setHeight(32.f * 4.f + 3.f * 3.f);
		editModeColumn->setPosition(glm::vec2(10.f, 0.f));
		editModeColumn->setSpacing(3.f);
		editModeColumn->setVisible(false);
		PG_GuiItem::anchor(editModeColumn, PG_Anchor::Left, btn, PG_Anchor::Right);
		PG_GuiItem::anchor(editModeColumn, PG_Anchor::VerticalCenter, btn, PG_Anchor::VerticalCenter);
		connect(mod, &PG_DisaEditorModule::modeChanged, editModeColumn, [editModeColumn](PG_DisaEditorModule::EMode mode) {
			editModeColumn->setVisible(mode == PG_DisaEditorModule::Mode_Object);
			});

		auto modeBtn = PG_new<PG_GuiToggleButton>("SelectMode", editModeColumn);
		modeBtn->setTooltipPositionHint(ETooltipPositionHint::PG_TooltipPositionHint_Right);
		modeBtn->setSize(glm::vec2(32.f));
		modeBtn->setIcon("gui/icon_cursor.png");
		modeBtn->setTooltipText("Select mode");
		modeBtn->setBackground("M_GuiRoundedRectangleTop5");
		modeBtn->setToggled(mod->getEditMode() == PG_DisaEditorModule::EEditMode::Select);

		connect(mod, &PG_DisaEditorModule::editModeChanged, modeBtn, [modeBtn](PG_DisaEditorModule::EEditMode mode) {
			modeBtn->setToggled(mode == PG_DisaEditorModule::EEditMode::Select);
			});
		connect(modeBtn, &PG_GuiToggleButton::clicked, this, [this] {
			mod->setEditMode(PG_DisaEditorModule::EEditMode::Select);
			});

		modeBtn = PG_new<PG_GuiToggleButton>("TranslateMode", editModeColumn);
		modeBtn->setTooltipPositionHint(ETooltipPositionHint::PG_TooltipPositionHint_Right);
		modeBtn->setSize(glm::vec2(32.f));
		modeBtn->setIcon("gui/icon_axis.png");
		modeBtn->setTooltipText("Translate mode");
		modeBtn->setToggled(mod->getEditMode() == PG_DisaEditorModule::EEditMode::Translate);

		connect(mod, &PG_DisaEditorModule::editModeChanged, modeBtn, [modeBtn](PG_DisaEditorModule::EEditMode mode) {
			modeBtn->setToggled(mode == PG_DisaEditorModule::EEditMode::Translate);
			});
		connect(modeBtn, &PG_GuiToggleButton::clicked, this, [this] {
			mod->setEditMode(PG_DisaEditorModule::EEditMode::Translate);
			});

		modeBtn = PG_new<PG_GuiToggleButton>("RotateMode", editModeColumn);
		modeBtn->setTooltipPositionHint(ETooltipPositionHint::PG_TooltipPositionHint_Right);
		modeBtn->setSize(glm::vec2(32.f));
		modeBtn->setIcon("gui/icon_rotate.png");
		modeBtn->setTooltipText("Rotate mode");
		modeBtn->setToggled(mod->getEditMode() == PG_DisaEditorModule::EEditMode::Rotate);

		connect(mod, &PG_DisaEditorModule::editModeChanged, modeBtn, [modeBtn](PG_DisaEditorModule::EEditMode mode) {
			modeBtn->setToggled(mode == PG_DisaEditorModule::EEditMode::Rotate);
			});
		connect(modeBtn, &PG_GuiToggleButton::clicked, this, [this] {
			mod->setEditMode(PG_DisaEditorModule::EEditMode::Rotate);
			});

		modeBtn = PG_new<PG_GuiToggleButton>("ScaleMode", editModeColumn);
		modeBtn->setTooltipPositionHint(ETooltipPositionHint::PG_TooltipPositionHint_Right);
		modeBtn->setSize(glm::vec2(32.f));
		modeBtn->setIcon("gui/icon_scale.png");
		modeBtn->setTooltipText("Scale mode");
		modeBtn->setBackground("M_GuiRoundedRectangleBottom5");
		modeBtn->setToggled(mod->getEditMode() == PG_DisaEditorModule::EEditMode::Scale);

		connect(mod, &PG_DisaEditorModule::editModeChanged, modeBtn, [modeBtn](PG_DisaEditorModule::EEditMode mode) {
			modeBtn->setToggled(mode == PG_DisaEditorModule::EEditMode::Scale);
			});
		connect(modeBtn, &PG_GuiToggleButton::clicked, this, [this] {
			mod->setEditMode(PG_DisaEditorModule::EEditMode::Scale);
			});
	}
}

void Inspector::createInspector() {
	auto inspector = root->findFirstChild<PG_GuiRectangle>("Inspector");
	if (mod->getMode() == PG_DisaEditorModule::Mode_View) {
		if (inspector) delete inspector;
		return;
	}

	//find items
	std::vector<PG_Object*> items;
	auto* wrapper = inspector? inspector->findFirstChild<PG_SelectionWrapper>("SelectionWrapper"): nullptr;
	bool isNew = false;
	if(!mod->getSelection().empty()){
		if (m_lastMode != mod->getMode()) {
			if (wrapper) delete wrapper;
			wrapper = nullptr;
			m_lastMode = mod->getMode();
			isNew = true;
		}

		if (!wrapper) {
			wrapper = PG_new<PG_SelectionWrapper>("SelectionWrapper");
			isNew = true;
		}

		//create elements
		switch (mod->getMode()) {
		case PG_DisaEditorModule::Mode_View: return;
		case PG_DisaEditorModule::Mode_Object:
		case PG_DisaEditorModule::Mode_Terrain:
		case PG_DisaEditorModule::Mode_Actor:
			std::ranges::transform(mod->getSelection(), std::back_inserter(items), [](const PG_EditorActorInfo& info) -> PG_Object* { return info.actor.get(); });
			break;
		case PG_DisaEditorModule::Mode_Material:
			for (auto* item : wrapper->getSelectionItems() | PG_CastTo<PG_DisaMapTileMaterialModel*>) {
				item->deleteLater();
			}

			std::ranges::transform(mod->getSelection(), std::back_inserter(items), [this, wrapper](const PG_EditorActorInfo& info) -> PG_Object* {
				if (info.additionalData.is<PG_DisaFaceHit>()) {
					auto hit = info.additionalData.get<PG_DisaFaceHit>();
					auto model = dynamic_cast<PG_DisaMapTileModel*>(dynamic_cast<PG_DisaTileViewNode*>(info.actor.get())->getTileModel());

					auto* materialModel = PG_new<PG_DisaMapTileMaterialModel>("PG_DisaMapTileMaterialModel", wrapper);
					materialModel->setSortIndex(info.index);
					connect(PG_DisaMapEditorMainModel::get(), &PG_DisaMapEditorMainModel::paintSurfacesPressed, materialModel, [materialModel](unsigned int index) {
						materialModel->setTextureIndex(index + 1);
						});
					materialModel->setTileModel(model);
					auto mode = getMaterialInspectorMode();
					switch (hit.face) {
					case PG_DisaFaceHit::Face_Top: {
						switch (mode)
						{
						default:
						case PG_DisaEditorModule::MaterialInspectorMode_Base: materialModel->setIndex(4); break;
						case PG_DisaEditorModule::MaterialInspectorMode_Decal: materialModel->setIndex(9); break;
						case PG_DisaEditorModule::MaterialInspectorMode_Shadow: materialModel->setIndex(10); break;
						}
					} break;
					case PG_DisaFaceHit::Face_West: materialModel->setIndex(mode == PG_DisaEditorModule::MaterialInspectorMode_Base ? 3 : 8); break;
					case PG_DisaFaceHit::Face_East: materialModel->setIndex(mode == PG_DisaEditorModule::MaterialInspectorMode_Base ? 1 : 6); break;
					case PG_DisaFaceHit::Face_South: materialModel->setIndex(mode == PG_DisaEditorModule::MaterialInspectorMode_Base ? 2 : 7); break;
					case PG_DisaFaceHit::Face_North: materialModel->setIndex(mode == PG_DisaEditorModule::MaterialInspectorMode_Base ? 0 : 5); break;
					default: break;
					}
					return materialModel;
				}
				return info.actor.get();
				});

			std::ranges::sort(items, {}, [](PG_Object* m) -> unsigned int {
				if (auto tm = dynamic_cast<PG_DisaMapTileMaterialModel*>(m)) {
					return tm->getSortIndex();
				}
				return 0;
				});
			break;
			case PG_DisaEditorModule::Mode_EventTile: {
					std::ranges::transform(mod->getSelection(), std::back_inserter(items), [this, wrapper](const PG_EditorActorInfo& info) -> PG_Object* {
						glm::ivec2 pos(0);
						PG_DisaEventTileListModel* eventTileModel = nullptr;
						if (info.additionalData.is<PG_DisaFaceHit>()) {
							auto hit = info.additionalData.get<PG_DisaFaceHit>();
							auto model = dynamic_cast<PG_DisaMapTileModel*>(dynamic_cast<PG_DisaTileViewNode*>(info.actor.get())->getTileModel());
							pos = model->getTilePosition();
							eventTileModel = model->findParent<PG_DisaMapModel>(3)->getEvents();
						}
						else if (info.additionalData.is<PG_DisaEmptyTileHit>()) {
							auto hit = info.additionalData.get<PG_DisaEmptyTileHit>();
							pos = hit.tilePos;
							eventTileModel = dynamic_cast<PG_DisaMapViewNode*>(info.actor.get())->getMapModel()->getEvents();
						}
						assert(eventTileModel);
						if (!eventTileModel) return nullptr;
						auto* selectionModel = PG_new<EventTilePositionFilter>("EventTilePositionFilter", wrapper);
						selectionModel->setTilePosition(pos);
						selectionModel->setModel(eventTileModel);
						return selectionModel;
					});
					std::erase_if(items, [](PG_Object* obj) { return !static_cast<bool>(obj); });
			}break;
		}
	}

	if (items.empty()) {
		if (wrapper) {
			isNew = true;
			delete wrapper;
			wrapper = nullptr;
		}
		if (mod->getMode() != PG_DisaEditorModule::Mode_Actor) {
			if (inspector) delete inspector;
			return;
		}
	}

	PG_GuiColumn* column;
	if (inspector) {
		column = inspector->findFirstChild<PG_GuiColumn>("Column");
	}else{
		isNew = true;
		inspector = PG_new<PG_GuiRectangle>("Inspector", root);
		inspector->setMaterial("M_GuiRoundedRectangleAll5");
		inspector->setColor(backgroundColor);
		inspector->setAlpha(0.7f);

		PG_GuiItem::anchor(inspector, PG_Anchor::Top, root, PG_Anchor::Top);
		PG_GuiItem::anchor(inspector, PG_Anchor::Right, root, PG_Anchor::Right);
		PG_GuiItem::anchor(inspector, PG_Anchor::Bottom, root, PG_Anchor::Bottom);
		inspector->setMargin(10.f);
		inspector->setClip(true);
		inspector->moveFront();

		auto area = PG_new<PG_GuiMouseArea>("MouseAreaBlocker", inspector);
		area->setHoverEnabled(true);
		area->fill(inspector);

		column = PG_new<PG_GuiColumn>("Column", inspector);
		PG_GuiItem::anchor(column, PG_Anchor::Left, inspector, PG_Anchor::Left);
		PG_GuiItem::anchor(column, PG_Anchor::Right, inspector, PG_Anchor::Right);
		PG_GuiItem::anchor(column, PG_Anchor::Top, inspector, PG_Anchor::Top);
		PG_GuiItem::anchor(column, PG_Anchor::Bottom, inspector, PG_Anchor::Bottom);
		column->setMargin(4.f);
		column->setHeight(100.f);
	}

	if (!wrapper) {
		if (isNew) {
			switch (mod->getMode()) {
			case PG_DisaEditorModule::Mode_Actor:
				inspector->setWidth(inspectorWidth);
				createForActorNoneSelectEdit(column);
				break;
			}
		}
	}else{
		wrapper->setParent(inspector);
		wrapper->setSelectionItems(items);

		//create elements
		if(isNew){
			switch (mod->getMode()) {
			case PG_DisaEditorModule::Mode_View: 	column->clearGuiChildren();  return;
			case PG_DisaEditorModule::Mode_Terrain:
				inspector->setWidth(inspectorWidth);
				createForTerrainEdit(column, wrapper);
				break;
			case PG_DisaEditorModule::Mode_Material:
				inspector->setWidth(inspectorWidth);
				createForMaterialEdit(this, column, wrapper);
				break;
			case PG_DisaEditorModule::Mode_Object:
				inspector->setWidth(inspectorWidth);
				createForObjectEdit(column, wrapper);
				break;
			case PG_DisaEditorModule::Mode_EventTile:
				inspector->setWidth(inspectorWidth);
				createForEventEdit(column, wrapper);
				break;
			case PG_DisaEditorModule::Mode_Actor:
				inspector->setWidth(540.f);
				createForActorEdit(column, wrapper);
				break;
			}
		}
	}
}
