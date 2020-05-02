/*
 * CreateEmptySpriteSheet.cpp
 *
 *  Created on: 25.08.2020
 *      Author: ProgSys
 */

#include <ResizeDialog.h>

#include <QDebug>
#include <qcombobox.h>

ResizeDialog::ResizeDialog(int sheetId, int fromWidth, int fromHeight, int toWidth, int toHeight, QWidget* parent) :
	QDialog(parent), m_sheetId(sheetId), m_fromWidth(fromWidth), m_fromHeight(fromHeight), m_toWidth(toWidth), m_toHeight(toHeight) {
	setupUi(this);
	setWindowTitle("Scale up sprite sheet");

	init();

	spinBox_sheetId->setValue(sheetId);
	spinBox_sheetId->setEnabled(true);

	auto getSizeIndex = [](int size) {
		switch (size)
		{
		case 32: return 0;
		case 64: return 1;
		case 128: return 2;
		case 256: return 3;
		case 512: return 4;
		}
		return 3;
	};

	
	comboBox_from_width->setCurrentIndex(getSizeIndex(fromWidth));
	comboBox_from_height->setCurrentIndex(getSizeIndex(fromHeight));
	comboBox_to_width->setCurrentIndex(getSizeIndex(toWidth));
	comboBox_to_height->setCurrentIndex(getSizeIndex(toHeight));

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejected()));
}

void ResizeDialog::init() {
	auto addSizeOptions = [](QComboBox* box) {
		box->addItem("32");
		box->addItem("64");
		box->addItem("128");
		box->addItem("256");
		box->addItem("512");
		box->setCurrentIndex(3);
	};
	addSizeOptions(comboBox_from_width);
	addSizeOptions(comboBox_from_height);
	addSizeOptions(comboBox_to_width);
	addSizeOptions(comboBox_to_height);
}

void ResizeDialog::accepted() {
	m_sheetId = spinBox_sheetId->value();
	m_fromWidth = comboBox_from_width->currentText().toInt();
	m_fromHeight = comboBox_from_height->currentText().toInt();
	m_toWidth = comboBox_to_width->currentText().toInt();
	m_toHeight = comboBox_to_height->currentText().toInt();

	m_accepted = true;

	close();
}

void ResizeDialog::rejected() {
	m_accepted = false;
	m_sheetId = 0;
	m_fromWidth = 0;
	m_fromHeight = 0;
	m_toWidth = 0;
	m_toHeight = 0;
	emit cancel();
	close();
}

