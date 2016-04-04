/*
 * SpriteSheetEditorWindow.cpp
 *
 *  Created on: 31.03.2016
 *      Author: ProgSys
 */

#include <SpriteSheetEditor.h>
#include "ui_spritesheeteditor.h"

SpriteSheetEditor::SpriteSheetEditor(QWidget *parent):
	QMainWindow(parent),
	ui(new Ui::SpriteSheetEditorWindow)
{
	ui->setupUi(this);

}

SpriteSheetEditor::~SpriteSheetEditor() {
	 delete ui;
}

