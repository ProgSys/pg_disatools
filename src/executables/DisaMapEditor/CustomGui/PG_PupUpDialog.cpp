#include "PG_PupUpDialog.h"

#include <Base/PG_Application.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiTextField.h>
#include <Scene/Gui/PG_GuiTextEdit.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiDropDown.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>
#include <Scene/Gui/PG_GuiCreateSelectionElement.h>
#include <Scene/Model/PG_ListModel.h>

#include <Util/Color/ColorDefined.h>
#include <nfd.h>

constexpr glm::vec3 backgroundColor = glm::vec3(40.f / 255.f);
constexpr glm::vec3 backgroundDecoColor = glm::vec3(45.f / 255.f);
constexpr glm::vec3 backgroundDeco2Color = glm::vec3(30.f / 255.f);
constexpr glm::vec3 backgroundDarkColor = glm::vec3(20.f / 255.f);
constexpr glm::vec3 offWeightColor = glm::vec3(200.f / 255.f);
constexpr glm::vec3 blueColor = glm::vec3(133.f, 208.f, 255.f) / 255.f;
constexpr glm::vec3 bobbleTextColor = glm::vec3(76.f, 111.f, 199.f) / 255.f;

PG_File PG_PopUpDialog::selectFile(const std::string& fileExtensions) {
	nfdchar_t* outPath = nullptr;
	nfdresult_t result = NFD_OpenDialog(fileExtensions.c_str(), nullptr, &outPath);

	PG_EventLoop::doRun();
	PG_Application::grabMouse(nullptr);

	std::string path;
	if (outPath) {
		path = outPath;
		delete[] outPath;
	}

	if (result == NFD_OKAY) {
		return PG_File(path);
	}
	return PG_File();
}

static inline PG_GuiRectangle* createWindow(PG_GuiPopupInstance* parent) {

	PG_GuiMouseArea* blockArea = PG_new<PG_GuiMouseArea>("PG_GuiMouseArea", parent);
	blockArea->fill(parent);
	blockArea->setHoverEnabled(true);
	blockArea->setConferEnabled(false);

	auto bg = PG_new<PG_GuiRectangle>("N_BgRec", parent);
	bg->setColor(PG_Color::blackFrgb);
	bg->setAlpha(0.4F);
	bg->fill(parent);

	auto window = PG_new<PG_GuiRectangle>("N_PupUpWindowRec", bg);
	window->setSize(glm::vec2(650.f, 330.f));
	window->setColor(backgroundColor);
	window->setMaterial("M_GuiRoundedRectangleAll5");
	window->setClip(true);
	PG_GuiItem::anchor(window, PG_Anchor::HorizontalCenter, bg, PG_Anchor::HorizontalCenter);
	PG_GuiItem::anchor(window, PG_Anchor::VerticalCenter, bg, PG_Anchor::VerticalCenter);

	{
		//decoration
		auto rec = PG_new<PG_GuiRectangle>("N_DecoRec", window);
		rec->setColor(backgroundDecoColor);
		rec->setEulerAngleZ(-21.f);
		rec->setPosition(glm::vec2(0.f, 250.f));
		rec->setSize(glm::vec2(1000.f, 1000.f));
		PG_GuiItem::anchor(rec, PG_Anchor::Top, window, PG_Anchor::Top);
		PG_GuiItem::anchor(rec, PG_Anchor::Left, window, PG_Anchor::Left);

		rec = PG_new<PG_GuiRectangle>("N_DecoRec2", window);
		rec->setColor(backgroundDeco2Color);
		rec->setSize(glm::vec2(0.f, 80.f));
		PG_GuiItem::anchor(rec, PG_Anchor::Bottom, window, PG_Anchor::Bottom);
		PG_GuiItem::anchor(rec, PG_Anchor::Left, window, PG_Anchor::Left);
		PG_GuiItem::anchor(rec, PG_Anchor::Right, window, PG_Anchor::Right);
	}
	return window;
}

PG_PopUpDialog* PG_PopUpDialog::createSelectFile(PG_GuiItem* parentItem, const std::string& titleStr, const std::string& text, const std::string& tooltipCofirm, const std::string& tooltipCancel) {
	auto popup = PG_new<PG_PopUpDialog>("N_PopupMenu", parentItem);
	popup->setCreator([titleStr, text, tooltipCofirm, tooltipCancel](PG_GuiPopupInstance* parent) {

		auto window = createWindow(parent);
			
		auto title = PG_new<PG_GuiText>("N_Title", window);
		title->setFontSize(30.f);
		title->setColor(PG_Color::hentaiBlueFrgb);
		title->setHorizontalAlignment(PG_GuiText::HorizontalCenter);
		title->setPosition(glm::vec2(0.f, 45.f));
		title->setWrapMode(PG_GuiText::None);
		title->setText(titleStr);
		PG_GuiItem::anchor(title, PG_Anchor::Top, window, PG_Anchor::Top);
		PG_GuiItem::anchor(title, PG_Anchor::Left, window, PG_Anchor::Left);
		PG_GuiItem::anchor(title, PG_Anchor::Right, window, PG_Anchor::Right);

		auto infoText = PG_new<PG_GuiText>("N_Text", window);
		infoText->setFontSize(20.f);
		infoText->setColor(PG_Color::hentaiBlueFrgb);
		infoText->setHorizontalAlignment(PG_GuiText::ETextAlignment::HorizontalCenter);
		infoText->setPosition(glm::vec2(0.f, 100.f));
		infoText->setWrapMode(PG_GuiText::Word);
		infoText->setText(text);
		infoText->setMargin(PG_Anchor::Left, 20.f);
		infoText->setMargin(PG_Anchor::Right, 20.f);
		PG_GuiItem::anchor(infoText, PG_Anchor::Top, window, PG_Anchor::Top);
		PG_GuiItem::anchor(infoText, PG_Anchor::Left, window, PG_Anchor::Left);
		PG_GuiItem::anchor(infoText, PG_Anchor::Right, window, PG_Anchor::Right);


		auto textFiledContainer = PG_new<PG_GuiItem>("N_TextFiledContainer", window);
		textFiledContainer->setSize(glm::vec2(569.f, 30.f));
		textFiledContainer->setPosition(glm::vec2(0.f, 190.f));
		PG_GuiItem::anchor(textFiledContainer, PG_Anchor::Top, window, PG_Anchor::Top);
		PG_GuiItem::anchor(textFiledContainer, PG_Anchor::HorizontalCenter, window, PG_Anchor::HorizontalCenter);

		PG_GuiTextField* textField = PG_new<PG_GuiTextField>("N_textField", glm::vec2(0.f), glm::vec2(530.f, 30.f), textFiledContainer);
		textField->setBackground("M_GuiRoundedRectangleLeft5");
		textField->getBackgroundRectangle()->setColor(backgroundDarkColor);
		textField->setPlaceholderText("Path");
		textField->getTextItem()->setMaxChars(9000);

		PG_GuiButton* openButton = PG_new<PG_GuiButton>("N_OpenButton", glm::vec2(0.f), glm::vec2(39.f, 30.f), textFiledContainer);
		openButton->setIcon("gui/icon_folder.png");
		openButton->setBackground("M_GuiRoundedRectangleRight5");
		openButton->setTooltipText("Open");
		PG_GuiItem::anchor(openButton, PG_Anchor::Left, textField, PG_Anchor::Right);

		PG_GuiButton* confirm = PG_new<PG_GuiButton>("N_ConfirmButton", glm::vec2(0.f), glm::vec2(180.f, 42.f), window);
		confirm->setText("Confirm");
		confirm->setTooltipText(tooltipCofirm);
		confirm->setPosition(glm::vec2(0.f, -20.f));
		confirm->setBackground("M_GuiRoundedRectangleAll5");
		confirm->centerText();
		PG_GuiItem::anchor(confirm, PG_Anchor::Bottom, window, PG_Anchor::Bottom);
		PG_GuiItem::anchor(confirm, PG_Anchor::Left, textFiledContainer, PG_Anchor::Left);

		PG_GuiButton* cancel = PG_new<PG_GuiButton>("N_CancelButton", glm::vec2(0.f), glm::vec2(180.f, 42.f), window);
		cancel->setText("Cancel");
		cancel->setTooltipText(tooltipCancel);
		cancel->setPosition(glm::vec2(0.f, -20.f));
		cancel->setBackground("M_GuiRoundedRectangleAll5");
		cancel->centerText();
		PG_GuiItem::anchor(cancel, PG_Anchor::Bottom, window, PG_Anchor::Bottom);
		PG_GuiItem::anchor(cancel, PG_Anchor::Right, textFiledContainer, PG_Anchor::Right);

		//logic
		connect(openButton, &PG_GuiButton::clicked, textField, [textField] {
			if (auto result = selectFile("exe")) {
				textField->setText(result.getPath());
			}
		});

		auto dialog = dynamic_cast<PG_PopUpDialog*>(parent->getOrigin());
		assert(dialog);
		connect(cancel, &PG_GuiButton::clicked, dialog, &PG_PopUpDialog::cancelClicked);
		connect(confirm, &PG_GuiButton::clicked, dialog, &PG_PopUpDialog::confirmClicked);
		connect(confirm, &PG_GuiButton::clicked, textField, [dialog, textField]() {
			PG_File file = textField->getText();
			emit dialog->confirmClickedPath(file);
			});

		});
	connect(popup, &PG_PopUpDialog::activeChanged, popup, [popup](bool active) {
		if (!active) popup->deleteLater();
		});
	return popup;
}

PG_PopUpDialog* PG_PopUpDialog::createOk(PG_GuiItem* parentItem, int type, const std::string& titleStr, const std::string& text) {
	auto popup = PG_new<PG_PopUpDialog>("N_PopupMenu", parentItem);
	popup->setCreator([titleStr, text, type](PG_GuiPopupInstance* parent) {

		auto window = createWindow(parent);

		PG_GuiImage* img = nullptr;
		if (type != 0) {
			img = PG_new<PG_GuiImage>("N_Img", window);
			img->setSize(glm::vec2(127.f));
			img->setPosition(glm::vec2(60.f, -20.f));
			img->setImage(type == 1? "gui/icon_success_127.png": "gui/icon_error_127.png");
			PG_GuiItem::anchor(img, PG_Anchor::Left, window, PG_Anchor::Left);
			PG_GuiItem::anchor(img, PG_Anchor::VerticalCenter, window, PG_Anchor::VerticalCenter);
		}

		auto title = PG_new<PG_GuiText>("N_Title", window);
		title->setFontSize(30.f);
		title->setColor(PG_Color::hentaiBlueFrgb);
		title->setHorizontalAlignment(PG_GuiText::HorizontalCenter);
		title->setPosition(glm::vec2(0.f, 45.f));
		title->setWrapMode(PG_GuiText::None);
		title->setText(titleStr);
		PG_GuiItem::anchor(title, PG_Anchor::Top, window, PG_Anchor::Top);
		PG_GuiItem::anchor(title, PG_Anchor::Left, window, PG_Anchor::Left);
		PG_GuiItem::anchor(title, PG_Anchor::Right, window, PG_Anchor::Right);

		auto infoText = PG_new<PG_GuiText>("N_Text", window);
		infoText->setFontSize(20.f);
		infoText->setColor(PG_Color::hentaiBlueFrgb);
		infoText->setWrapMode(PG_GuiText::Word);
		infoText->setText(text);
		infoText->setMargin(PG_Anchor::Right, 20.f);

		if (img) {
			infoText->setMargin(PG_Anchor::Left, 40.f);
			infoText->setHorizontalAlignment(PG_GuiText::ETextAlignment::Left);
			infoText->setVerticalAlignment(PG_GuiText::ETextAlignment::VerticalCenter);
			PG_GuiItem::anchor(infoText, PG_Anchor::Top, img, PG_Anchor::Top);
			PG_GuiItem::anchor(infoText, PG_Anchor::Bottom, img, PG_Anchor::Bottom);
			PG_GuiItem::anchor(infoText, PG_Anchor::Left, img, PG_Anchor::Right);
			PG_GuiItem::anchor(infoText, PG_Anchor::Right, window, PG_Anchor::Right);
		}
		else {
			infoText->setMargin(PG_Anchor::Left, 20.f);
			infoText->setHorizontalAlignment(PG_GuiText::ETextAlignment::HorizontalCenter);
			infoText->setPosition(glm::vec2(0.f, 100.f));
			PG_GuiItem::anchor(infoText, PG_Anchor::Top, window, PG_Anchor::Top);
			PG_GuiItem::anchor(infoText, PG_Anchor::Left, window, PG_Anchor::Left);
			PG_GuiItem::anchor(infoText, PG_Anchor::Right, window, PG_Anchor::Right);
		}

		PG_GuiButton* confirm = PG_new<PG_GuiButton>("N_OkButton", glm::vec2(0.f), glm::vec2(180.f, 42.f), window);
		confirm->setText("Ok");
		confirm->setPosition(glm::vec2(0.f, -20.f));
		confirm->setBackground("M_GuiRoundedRectangleAll5");
		confirm->centerText();
		PG_GuiItem::anchor(confirm, PG_Anchor::Bottom, window, PG_Anchor::Bottom);
		PG_GuiItem::anchor(confirm, PG_Anchor::HorizontalCenter, window, PG_Anchor::HorizontalCenter);


		//logic
		auto dialog = dynamic_cast<PG_PopUpDialog*>(parent->getOrigin());
		assert(dialog);
		connect(confirm, &PG_GuiButton::clicked, dialog, &PG_PopUpDialog::confirmClicked);
		connect(confirm, &PG_GuiButton::clicked, parent, &PG_GuiPopupInstance::close);
	});

	connect(popup, &PG_PopUpDialog::activeChanged, popup, [popup](bool active) {
		if (!active) popup->deleteLater();
		});
	return popup;
}

PG_PopUpDialog* PG_PopUpDialog::createNewEntry(PG_GuiItem* parentItem, const std::string& titleStr, const std::string& text) {

	auto popup = PG_new<PG_PopUpDialog>("N_PopupMenu", parentItem);
	popup->setCreator([titleStr, text](PG_GuiPopupInstance* parent) {

		auto window = createWindow(parent);

		auto title = PG_new<PG_GuiText>("N_Title", window);
		title->setFontSize(30.f);
		title->setColor(PG_Color::hentaiBlueFrgb);
		title->setHorizontalAlignment(PG_GuiText::HorizontalCenter);
		title->setPosition(glm::vec2(0.f, 45.f));
		title->setWrapMode(PG_GuiText::None);
		title->setText(titleStr);
		PG_GuiItem::anchor(title, PG_Anchor::Top, window, PG_Anchor::Top);
		PG_GuiItem::anchor(title, PG_Anchor::Left, window, PG_Anchor::Left);
		PG_GuiItem::anchor(title, PG_Anchor::Right, window, PG_Anchor::Right);

		auto infoText = PG_new<PG_GuiText>("N_Text", window);
		infoText->setFontSize(20.f);
		infoText->setColor(PG_Color::hentaiBlueFrgb);
		infoText->setHorizontalAlignment(PG_GuiText::ETextAlignment::HorizontalCenter);
		infoText->setPosition(glm::vec2(0.f, 100.f));
		infoText->setWrapMode(PG_GuiText::Word);
		infoText->setText(text);
		infoText->setMargin(PG_Anchor::Left, 20.f);
		infoText->setMargin(PG_Anchor::Right, 20.f);
		PG_GuiItem::anchor(infoText, PG_Anchor::Top, window, PG_Anchor::Top);
		PG_GuiItem::anchor(infoText, PG_Anchor::Left, window, PG_Anchor::Left);
		PG_GuiItem::anchor(infoText, PG_Anchor::Right, window, PG_Anchor::Right);


		PG_GuiTextField* textField = PG_new<PG_GuiTextField>("N_textField", glm::vec2(0.f, 55.f), glm::vec2(569.f, 30.f), window);
		PG_GuiItem::anchor(textField, PG_Anchor::Top, infoText, PG_Anchor::Bottom);
		PG_GuiItem::anchor(textField, PG_Anchor::HorizontalCenter, window, PG_Anchor::HorizontalCenter);
		textField->getTextItem()->setValidator(PG_SimpleWordSpaceValidator());
		textField->setBackground("M_GuiRoundedRectangleAll5");
		textField->getBackgroundRectangle()->setColor(backgroundDarkColor);
		textField->setPlaceholderText("Name");
		textField->getTextItem()->setMaxChars(40);


		PG_GuiButton* confirm = PG_new<PG_GuiButton>("N_ConfirmButton", glm::vec2(0.f), glm::vec2(180.f, 42.f), window);
		confirm->setText("Create");
		confirm->setPosition(glm::vec2(0.f, -20.f));
		confirm->setBackground("M_GuiRoundedRectangleAll5");
		confirm->centerText();
		confirm->setEnabled(false);
		PG_GuiItem::anchor(confirm, PG_Anchor::Bottom, window, PG_Anchor::Bottom);
		PG_GuiItem::anchor(confirm, PG_Anchor::Left, textField, PG_Anchor::Left);

		PG_GuiButton* cancel = PG_new<PG_GuiButton>("N_CancelButton", glm::vec2(0.f), glm::vec2(180.f, 42.f), window);
		cancel->setText("Cancel");
		cancel->setPosition(glm::vec2(0.f, -20.f));
		cancel->setBackground("M_GuiRoundedRectangleAll5");
		cancel->centerText();
		PG_GuiItem::anchor(cancel, PG_Anchor::Bottom, window, PG_Anchor::Bottom);
		PG_GuiItem::anchor(cancel, PG_Anchor::Right, textField, PG_Anchor::Right);

		//logic
		auto dialog = dynamic_cast<PG_PopUpDialog*>(parent->getOrigin());
		assert(dialog);
		connect(cancel, &PG_GuiButton::clicked, dialog, &PG_PopUpDialog::cancelClicked);
		connect(confirm, &PG_GuiButton::clicked, dialog, &PG_PopUpDialog::confirmClicked);
		connect(confirm, &PG_GuiButton::clicked, textField, [dialog, textField]() {
			PG_File file = textField->getText();
			emit dialog->confirmClickedPath(file);
			});
		connect(textField, &PG_GuiTextField::textChanged, confirm, [textField, confirm]() {
			confirm->setEnabled(!textField->getText().empty());
			});

		});
	connect(popup, &PG_PopUpDialog::activeChanged, popup, [popup](bool active) {
		if (!active) popup->deleteLater();
		});
	return popup;
}

PG_PopUpDialog* PG_PopUpDialog::selectCompression(PG_GuiItem* parentItem) {
	if (!parentItem) return nullptr;
	auto popup = PG_new<PG_PopUpDialog>("N_PopupMenu", parentItem);

	popup->setCreator([](PG_GuiPopupInstance* parent) {

		auto window = createWindow(parent);
		window->setSize(glm::vec2(450.f, 330.f));

		auto title = PG_new<PG_GuiText>("N_Title", window);
		title->setFontSize(30.f);
		title->setColor(PG_Color::hentaiBlueFrgb);
		title->setHorizontalAlignment(PG_GuiText::HorizontalCenter);
		title->setPosition(glm::vec2(0.f, 45.f));
		title->setWrapMode(PG_GuiText::None);
		title->setText("Compression");
		PG_GuiItem::anchor(title, PG_Anchor::Top, window, PG_Anchor::Top);
		PG_GuiItem::anchor(title, PG_Anchor::Left, window, PG_Anchor::Left);
		PG_GuiItem::anchor(title, PG_Anchor::Right, window, PG_Anchor::Right);

		auto infoText = PG_new<PG_GuiText>("N_Text", window);
		infoText->setFontSize(20.f);
		infoText->setColor(PG_Color::hentaiBlueFrgb);
		infoText->setHorizontalAlignment(PG_GuiText::ETextAlignment::HorizontalCenter);
		infoText->setPosition(glm::vec2(0.f, 100.f));
		infoText->setWrapMode(PG_GuiText::Word);
		infoText->setText("Choose target format");
		infoText->setMargin(PG_Anchor::Left, 20.f);
		infoText->setMargin(PG_Anchor::Right, 20.f);
		PG_GuiItem::anchor(infoText, PG_Anchor::Top, window, PG_Anchor::Top);
		PG_GuiItem::anchor(infoText, PG_Anchor::Left, window, PG_Anchor::Left);
		PG_GuiItem::anchor(infoText, PG_Anchor::Right, window, PG_Anchor::Right);

		PG_GuiDropDown* dropDown = PG_new<PG_GuiDropDown>("N_compressionDropDown", glm::vec2(0.f, 55.f), glm::vec2(369.f, 30.f), window);
		auto listModel = PG_new<PG_ListModel<std::string>>(dropDown);

		listModel->set({
				{ "RGBA8888"},
				{ "DXT1"},
				{ "DXT5"},
			});
		dropDown->setModel(listModel);
		dropDown->setIndex(0);

		PG_GuiItem::anchor(dropDown, PG_Anchor::Top, infoText, PG_Anchor::Bottom);
		PG_GuiItem::anchor(dropDown, PG_Anchor::HorizontalCenter, window, PG_Anchor::HorizontalCenter);


		PG_GuiButton* confirm = PG_new<PG_GuiButton>("N_ConfirmButton", glm::vec2(0.f), glm::vec2(180.f, 42.f), window);
		confirm->setText("Confirm");
		confirm->setPosition(glm::vec2(0.f, -20.f));
		confirm->setBackground("M_GuiRoundedRectangleAll5");
		confirm->centerText();
		PG_GuiItem::anchor(confirm, PG_Anchor::Bottom, window, PG_Anchor::Bottom);
		PG_GuiItem::anchor(confirm, PG_Anchor::Left, dropDown, PG_Anchor::Left);

		PG_GuiButton* cancel = PG_new<PG_GuiButton>("N_CancelButton", glm::vec2(0.f), glm::vec2(180.f, 42.f), window);
		cancel->setText("Cancel");
		cancel->setPosition(glm::vec2(0.f, -20.f));
		cancel->setBackground("M_GuiRoundedRectangleAll5");
		cancel->centerText();
		PG_GuiItem::anchor(cancel, PG_Anchor::Bottom, window, PG_Anchor::Bottom);
		PG_GuiItem::anchor(cancel, PG_Anchor::Right, dropDown, PG_Anchor::Right);

		//logic
		auto dialog = dynamic_cast<PG_PopUpDialog*>(parent->getOrigin());
		assert(dialog);
		connect(cancel, &PG_GuiButton::clicked, dialog, &PG_PopUpDialog::cancelClicked);
		connect(confirm, &PG_GuiButton::clicked, dialog, &PG_PopUpDialog::confirmClicked);
		connect(confirm, &PG_GuiButton::clicked, dialog, [dialog, dropDown]() {
			emit dialog->confirmClickedIndex(dropDown->getIndex());
			});

		connect(cancel, &PG_GuiButton::clicked, parent, &PG_GuiPopupInstance::close, ConnectionType_Event);
		connect(confirm, &PG_GuiButton::clicked, parent, &PG_GuiPopupInstance::close, ConnectionType_Event);

		});
	connect(popup, &PG_PopUpDialog::activeChanged, popup, [popup](bool active) {
		if (!active) popup->deleteLater();
		});
	return popup;
}

PG_PopUpDialog* PG_PopUpDialog::openProgress(PG_GuiItem* parentItem, const std::string& titleStr, const std::string& text, float progress) {
	auto popup = parentItem->findFirstChild<PG_PopUpDialog>("N_PopupProgressDialog");
	constexpr float progressWidth = 362.f;
	if (popup && popup->getInstance()) {
		if (text.empty()) {
			popup->close();
			popup->deleteLater();
			return nullptr;
		}
		auto window = popup->getInstance()->findFirstChild("N_BgRec")->findFirstChild("N_PupUpWindowRec");
		auto progressBar = window->findFirstChild("ProgressBarBg")->findFirstChild<PG_GuiRectangle>("ProgressBar");
		progressBar->setWidth(progress > 0.0F? progressWidth * progress: 0.f);
		return popup;
	}

	if (text.empty()) return nullptr;

	popup = PG_new<PG_PopUpDialog>("N_PopupProgressDialog", parentItem);
	popup->setCreator([titleStr, text, progress](PG_GuiPopupInstance* parent) {
		auto window = createWindow(parent);

		auto title = PG_new<PG_GuiText>("N_Title", window);
		title->setFontSize(30.f);
		title->setColor(PG_Color::hentaiBlueFrgb);
		title->setHorizontalAlignment(PG_GuiText::HorizontalCenter);
		title->setPosition(glm::vec2(0.f, 45.f));
		title->setWrapMode(PG_GuiText::None);
		title->setText(titleStr);
		PG_GuiItem::anchor(title, PG_Anchor::Top, window, PG_Anchor::Top);
		PG_GuiItem::anchor(title, PG_Anchor::Left, window, PG_Anchor::Left);
		PG_GuiItem::anchor(title, PG_Anchor::Right, window, PG_Anchor::Right);

		auto infoText = PG_new<PG_GuiText>("N_Text", window);
		infoText->setFontSize(20.f);
		infoText->setColor(PG_Color::hentaiBlueFrgb);
		infoText->setHorizontalAlignment(PG_GuiText::ETextAlignment::HorizontalCenter);
		infoText->setPosition(glm::vec2(0.f, 100.f));
		infoText->setWrapMode(PG_GuiText::Word);
		infoText->setText(text);
		infoText->setMargin(PG_Anchor::Left, 20.f);
		infoText->setMargin(PG_Anchor::Right, 20.f);
		PG_GuiItem::anchor(infoText, PG_Anchor::Top, window, PG_Anchor::Top);
		PG_GuiItem::anchor(infoText, PG_Anchor::Left, window, PG_Anchor::Left);
		PG_GuiItem::anchor(infoText, PG_Anchor::Right, window, PG_Anchor::Right);


		auto progressBarBg = PG_new<PG_GuiRectangle>("ProgressBarBg", glm::vec2(0.f, 25.f), glm::vec2(progressWidth, 32.f), window);
		progressBarBg->setColor(PG_Gui::backgroundColor * 0.9f);
		progressBarBg->setBorderWidth(1.f);
		progressBarBg->setBorderColor(PG_Gui::blueColor);
		progressBarBg->setMargin({ 0.f, 50.f, 35.f, 35.f });
		progressBarBg->setMaterial("M_GuiRoundedRectangleAll5");
		progressBarBg->setClip(true);
		PG_GuiItem::anchor(progressBarBg, PG_Anchor::Bottom, window, PG_Anchor::Bottom);
		PG_GuiItem::anchor(progressBarBg, PG_Anchor::HorizontalCenter, window, PG_Anchor::HorizontalCenter);

		auto progressBar = PG_new<PG_GuiRectangle>("ProgressBar", glm::vec2(0.f), glm::vec2(0.f, 30.f), progressBarBg);
		progressBar->setColor(PG_Gui::blueColor);

		PG_GuiItem::anchor(progressBar, PG_Anchor::Left, progressBarBg, PG_Anchor::Left);
		PG_GuiItem::anchor(progressBar, PG_Anchor::Top, progressBarBg, PG_Anchor::Top);
		PG_GuiItem::anchor(progressBar, PG_Anchor::Bottom, progressBarBg, PG_Anchor::Bottom);

		progressBar->setWidth(progress > 0.0F ? progressWidth * progress : 0.f);
	});
	popup->open();

	return popup;
}