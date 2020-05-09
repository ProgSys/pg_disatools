/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2016  ProgSys
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, version 3 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with this program.  If not, see http://doc.qt.io/qt-5/lgpl.html
 *	or http://www.gnu.org/licenses/
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
