/*
 * CompileErrorBox.cpp
 *
 *  Created on: 27.06.2017
 *      Author: ProgSys
 */

#include <CompileErrorBox.h>

CompileErrorBox::CompileErrorBox(const QString& title, const QString& text, QWidget *parent): QDialog(parent) {
	setupUi(this);
	setWindowTitle(title);

	plainTextEdit->setPlainText(text);

	connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
}

CompileErrorBox::~CompileErrorBox() {
	// TODO Auto-generated destructor stub
}

