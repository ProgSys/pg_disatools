#pragma once

#include <Scene/Gui/PG_GuiPopup.h>
#include <Util/PG_File.h>
#include <Util/PG_Dir.h>

class PG_PopUpDialog: public PG_GuiPopup {
	PG_OBJECT(PG_PopUpDialog);
	PG_PARENT(PG_GuiPopup);
signals:
	void confirmClicked();
	void confirmClickedPath(PG_File path);
	void confirmClickedIndex(int i);
	void cancelClicked();
public:
	using PG_GuiPopup::PG_GuiPopup;

	static PG_PopUpDialog* createSelectFile(PG_GuiItem* parent, const std::string& title, const std::string& text, const std::string& tooltipCofirm, const std::string& tooltipCancel);
	static PG_PopUpDialog* createOk(PG_GuiItem* parent, int type, const std::string& title, const std::string& text);
	static PG_PopUpDialog* createSuccess(PG_GuiItem* parent, const std::string& title, const std::string& text) {
		return createOk(parent, 1, title, text);
	}
	static PG_PopUpDialog* createError(PG_GuiItem* parent, const std::string& title, const std::string& text) {
		return createOk(parent, 2, title, text);
	}
	/**
	* @brief Ast the user to confrim a operation
	* Will auto close on click
	*/
	static PG_PopUpDialog* createConfirm(PG_GuiItem* parent, const std::string& title, const std::string& text);

	static PG_PopUpDialog* createNewEntry(PG_GuiItem* parentItem, const std::string& titleStr, const std::string& text);
	static PG_PopUpDialog* openProgress(PG_GuiItem* parentItem, const std::string& titleStr, const std::string& text, float progress);
	static PG_PopUpDialog* selectCompression(PG_GuiItem* parentItem);

	static PG_File selectFile(const std::string& fileExtensions);
	/**
	* @brief Like selectFile, but the user can select multiple files
	* @returns The selected files, is empty when canceled
	*/
	static std::vector<PG_File> selectFiles(const std::string& fileExtensions);
	static PG_File saveAs(const std::string& fileName);
	static PG_Dir selectDir();
};