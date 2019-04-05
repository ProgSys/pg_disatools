/*
 * CompileErrorBox.h
 *
 *  Created on: 27.06.2017
 *      Author: ProgSys
 */

#ifndef INCLUDE_SPRITESHEETEDITOR_COMPILEERRORBOX_H_
#define INCLUDE_SPRITESHEETEDITOR_COMPILEERRORBOX_H_

#include "ui_compileErrorBox.h"

/*!
 * @brief A simple message box with a plain text widget to support longer text.
 */
class CompileErrorBox: public QDialog, public Ui::CompileErrorBox {
public:
	CompileErrorBox(const QString& title, const QString& text, QWidget *parent = 0);
	virtual ~CompileErrorBox();
};

#endif /* INCLUDE_SPRITESHEETEDITOR_COMPILEERRORBOX_H_ */
