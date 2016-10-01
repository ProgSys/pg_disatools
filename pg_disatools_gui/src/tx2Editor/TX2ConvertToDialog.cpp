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
#include <tx2Editor/TX2ConvertToDialog.h>

TX2ConvertToDialog::TX2ConvertToDialog(QWidget *parent): QDialog(parent) {
	setupUi(this);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejected()));
}


bool TX2ConvertToDialog::isAccepted() const{
	return m_accepted;
}
const QString& TX2ConvertToDialog::getResult() const{
	return m_result;
}

void TX2ConvertToDialog::accepted(){
	m_accepted = true;
	m_result = comboBox->currentText();
	emit ok(m_result);
}

void TX2ConvertToDialog::rejected(){
	m_accepted = false;
	m_result = "";

}

TX2ConvertToDialog::~TX2ConvertToDialog() {
	// TODO Auto-generated destructor stub
}

