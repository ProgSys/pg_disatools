#include "PG_DisaAddDungeonDialog.h"

#include <Base/PG_Application.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiColumn.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiTextField.h>
#include <Scene/Gui/PG_GuiTextEdit.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiGridView.h>
#include <Scene/Gui/PG_GuiCreateSelectionElement.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>
#include <Scene/Model/PG_PropertyDynamicWrapper.h>

#include <CustomGui/PG_DisaCustomCampaign.h>
#include <PG_DisaGridTile.h>
#include <Util/Color/ColorDefined.h>
#include <nfd.h>

#include <Scene/Model/PG_ListSelectionModel.h>
#include <Models/PG_OverviewListModel.h>
#include <Models/PG_DisaDataDatModel.h>
#include <Models/PG_DisaMapEditorMainModel.h>
#include <Resources/PG_ResourceManager.h>

constexpr glm::vec3 backgroundColor = glm::vec3(40.f / 255.f);
constexpr glm::vec3 backgroundDecoColor = glm::vec3(45.f / 255.f);
constexpr glm::vec3 backgroundDeco2Color = glm::vec3(30.f / 255.f);
constexpr glm::vec3 backgroundDarkColor = glm::vec3(20.f / 255.f);
constexpr glm::vec3 offWeightColor = glm::vec3(200.f / 255.f);
constexpr glm::vec3 blueColor = glm::vec3(133.f, 208.f, 255.f) / 255.f;
constexpr glm::vec3 bobbleTextColor = glm::vec3(76.f, 111.f, 199.f) / 255.f;


struct DungeonStyleEntry {
	std::string fileName;
	std::string displayName;
	PG_DisaMPPStyle style;
	PG_DisaFileEntry orgaFileEntry;
	int index = -1;

	operator const std::string& () const { return displayName; }

	int getLocation() const { return index; }
	void setLocation(int _index) { index = _index; }

	PG_Variety getData(const PG_ListModelBase* self, Identity identity) const {
		switch (identity)
		{
		case Identity_Text:
			return displayName;
		case Identity_Edit: {
			return orgaFileEntry;
		}
		case Identity_PreviewImage: 
			return PG_DisaMPPPack{ self->getLocation(index) };
		case Identity_Custom:
			return style;
		default: break;
		}
		return orgaFileEntry.getData(0, identity);
	}
};

class DungeonStyleListModel : public PG_ListSelectionModel<DungeonStyleEntry, true> {
public:
	using PG_ListSelectionModel<DungeonStyleEntry, true>::PG_ListSelectionModel;
};

static inline PG_DisaDataDatModel* getDisaDataDatModel() {
	if (!PG_DisaMapEditorMainModel::get() || PG_DisaMapEditorMainModel::get()->getUiStackSize() < 1) return nullptr;
	
	if (auto* overviewModel = dynamic_cast<PG_OverviewListModel*>(PG_DisaMapEditorMainModel::get()->getUiStackEntry(0).get())) {
		if (overviewModel->empty()) return nullptr;

		if (auto dataDatModel = dynamic_cast<PG_DisaDataDatModel*>(overviewModel->get(0).model.get())) {
			return dataDatModel;
		}
	}

	return nullptr;
}


static inline void createWindow(PG_GuiPopupInstance* parent, PG_Object* model) {
	PG_GuiMouseArea* blockArea = PG_new<PG_GuiMouseArea>("PG_GuiMouseArea", parent);
	blockArea->fill(parent);
	blockArea->setHoverEnabled(true);
	blockArea->setConferEnabled(false);

	auto bg = PG_new<PG_GuiRectangle>("N_BgRec", parent);
	bg->setColor(PG_Color::blackFrgb);
	bg->setAlpha(0.4F);
	bg->fill(parent);

	auto window = PG_new<PG_GuiRectangle>("N_PupUpWindowRec", bg);
	
	window->setColor(backgroundColor);
	window->setMaterial("M_GuiRoundedRectangleAll5");
	window->setClip(true);
	window->setMargin(22.f);
	PG_GuiItem::anchor(window, PG_Anchor::Top, bg, PG_Anchor::Top);
	PG_GuiItem::anchor(window, PG_Anchor::Bottom, bg, PG_Anchor::Bottom);
	PG_GuiItem::anchor(window, PG_Anchor::Left, bg, PG_Anchor::Left);
	PG_GuiItem::anchor(window, PG_Anchor::Right, bg, PG_Anchor::Right);


	auto* column = PG_new<PG_GuiColumn>("Column", window);
	column->fill(window);
	column->setMargin({ 12.f , 2.f, 22.f, 22.f});

	PG_DisaCustomCampaignBase::textCreator("Name", column);
	auto* textField = PG_Gui::createGuiElementV2<PG_GuiTextField>(column, model, "Name", "name", "text", PG_DisaCustomCampaignBase::textFieldCreator);
	PG_GuiItem::anchor(textField, PG_Anchor::Left, column, PG_Anchor::Left);
	PG_GuiItem::anchor(textField, PG_Anchor::Right, column, PG_Anchor::Right);
	textField->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);
	textField->getTextItem()->setValidator(PG_SimpleWordValidator());

	auto* gridView = PG_new<PG_GuiGridView>("grid", column);
	gridView->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	PG_GuiItem::anchor(gridView, PG_Anchor::Left, column, PG_Anchor::Left);
	PG_GuiItem::anchor(gridView, PG_Anchor::Right, column, PG_Anchor::Right);
	gridView->setClip(true);

	auto styleModel = PG_new<DungeonStyleListModel>(model);
	//styleModel->push_back(DungeonStyleEntry{"dev", "Dev"}); //todo

	if (auto* datModel = getDisaDataDatModel()) {
		for (auto entry : std::vector<DungeonStyleEntry>{ 
			{ "mp00101.mpp", "Grassplane", {1, 3, 2}},
			{ "mp01202.mpp", "Grassplane 2", {3, 4, 0}},
			{ "mp00201.mpp", "Castle", {1, 2, 0}},
			{ "mp00301.mpp", "Lava", {1, 2, 0}},
			{ "mp00401.mpp", "Golden Castle", {1, 3, 0}},
			{ "mp00501.mpp", "Ice", {1, 2, 0}},
			{ "mp00601.mpp", "Swamp", {1, 4, 0}},
			{ "mp00701.mpp", "Space", {1, 4, 0}},
			{ "mp00901.mpp", "Interior", {1, 3, 0}},
			{ "mp01301.mpp", "Celest", {1, 2, 0}},
			{ "mp01102.mpp", "Special", {1, 2, 0}},
			}
			) {

			if (auto findEntry = datModel->find(PG_File(entry.fileName), false)) {
				entry.orgaFileEntry = findEntry;
				styleModel->push_back(entry);
			}
		}

	}
	gridView->setModel(styleModel);

	gridView->setItem(PG_GuiGridView::getDefaultCreateFunc<PG_DisaGridTile>());
	gridView->setItemUpdate(PG_GuiGridView::getDefaultUpdateFunc<PG_DisaGridTile>());

	auto* bar = PG_new<PG_GuiItem>("Bottom", column);
	bar->setHeight(50.f);
	PG_GuiItem::anchor(bar, PG_Anchor::Left, column, PG_Anchor::Left);
	PG_GuiItem::anchor(bar, PG_Anchor::Right, column, PG_Anchor::Right);

	PG_GuiButton* confirm = PG_new<PG_GuiButton>("N_ConfirmButton", glm::vec2(0.f), glm::vec2(180.f, 42.f), bar);
	confirm->setText("Confirm");
	confirm->setBackground("M_GuiRoundedRectangleAll5");
	confirm->centerText();
	confirm->setEnabled(false);
	PG_GuiItem::anchor(confirm, PG_Anchor::VerticalCenter, bar, PG_Anchor::VerticalCenter);
	PG_GuiItem::anchor(confirm, PG_Anchor::Left, bar, PG_Anchor::Left);

	if (auto prop = model->getProperty("isConfirmEnabled")) {
		auto isEnabledFunc = [textField, model, prop, styleModel]() {
			model->setPropertyValue(prop, !textField->getText().empty() && styleModel->hasSelection());
			};
		PG_GuiItem::connect(textField, &PG_GuiTextField::textChanged, model, isEnabledFunc);
		PG_GuiItem::connect(styleModel, &DungeonStyleListModel::selectionChanged, model, isEnabledFunc);

		PG_GuiItem::connect(model, prop, confirm, [model, prop, confirm]() {
			confirm->setEnabled(model->getPropertyValue(prop).get<bool>());
			});

		PG_GuiItem::connect(confirm, &PG_GuiButton::clicked, model, [model, styleModel]() {
			std::string name = model->getPropertyValue("name").get<std::string>();
			PG_Object* targetModel = model->getPropertyValue("targetModel").get<PG_Object*>();
			PG_ObjectFunction targetFunc = model->getPropertyValue("targetFunc").get<PG_ObjectFunction>();

			if (name.empty() || !styleModel->hasSelection() || !targetModel || !targetFunc) return;
			const DungeonStyleEntry selectedItem = std::move(styleModel->getSelectionItems().front());
			if (!selectedItem.orgaFileEntry) return;

			//find new
			auto newFile = selectedItem.orgaFileEntry.getParent()->find("new_" + selectedItem.fileName, false);
			if (!newFile) {
				OUT_ERROR("Failed to find: \"" << "new_" + selectedItem.fileName << "\"!");
				return;
			}

			targetFunc.callArgs(targetModel, name, selectedItem.orgaFileEntry, newFile, selectedItem.style.topIndex, selectedItem.style.sideIndex, selectedItem.style.decalIndex);
		});
		PG_GuiItem::connect(confirm, &PG_GuiButton::clicked, parent, &PG_GuiPopupInstance::close, ConnectionType_Event);
	}

	PG_GuiButton* cancel = PG_new<PG_GuiButton>("N_CancelButton", glm::vec2(0.f), glm::vec2(180.f, 42.f), bar);
	cancel->setText("Cancel");
	cancel->setBackground("M_GuiRoundedRectangleAll5");
	cancel->centerText();
	PG_GuiItem::anchor(cancel, PG_Anchor::VerticalCenter, bar, PG_Anchor::VerticalCenter);
	PG_GuiItem::anchor(cancel, PG_Anchor::Right, bar, PG_Anchor::Right);

	PG_GuiItem::connect(cancel, &PG_GuiButton::clicked, parent, &PG_GuiPopupInstance::close);
}

void PG_DisaAddDungeonDialog::init() {

	m_model = PG_new<PG_PropertyDynamicWrapper>("Wrapper", this);
	m_model->registerProperty("name", PG_Any::from(std::string()), true);
	m_model->registerProperty("isConfirmEnabled", PG_Any::from<bool>(false), true);
	
	setCreator([this](PG_GuiPopupInstance* parent) {
		createWindow(parent, m_model);
		});


}

void PG_DisaAddDungeonDialog::setModel(PG_Object* model, const PG_ObjectFunction& targetFunc) {
	m_model->registerProperty("targetModel", PG_Any::from<PG_Object*>(model));
	m_model->registerProperty("targetFunc", PG_Any::from<PG_ObjectFunction>(targetFunc));
}