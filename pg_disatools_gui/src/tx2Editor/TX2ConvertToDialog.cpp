/*
 * TX2ConvertToDialog.cpp
 *
 *  Created on: 30.09.2016
 *      Author: ProgSys
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

