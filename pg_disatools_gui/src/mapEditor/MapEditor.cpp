/*
 * MapEditor.cpp
 *
 *  Created on: 10.09.2016
 *      Author: ProgSys
 */

#include <mapEditor/MapEditor.h>
#include <include/TitleDefine.h>

MapEditor::MapEditor(QWidget *parent): QMainWindow(parent) {
	setupUi(this);

	setWindowIcon(QIcon("resources/map_editor_icon.ico"));
	setWindowTitle(MapEditorTITLE);

}

MapEditor::~MapEditor() {
	// TODO Auto-generated destructor stub
}

