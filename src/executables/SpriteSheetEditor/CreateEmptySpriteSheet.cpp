/*
 * CreateEmptySpriteSheet.cpp
 *
 *  Created on: 25.08.2016
 *      Author: ProgSys
 */

#include <CreateEmptySpriteSheet.h>

#include <QDebug>

CreateEmptySpriteSheet::CreateEmptySpriteSheet(QWidget* parent) :
	QDialog(parent)
{
	setupUi(this);
	setWindowTitle("Create empty sprite sheet");

	init();

	pushButton_delete->close();
	checkBox_resizeSprites->close();

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejected()));
}

CreateEmptySpriteSheet::CreateEmptySpriteSheet(int width, int height, int power, int externalId, QWidget* parent) :
	QDialog(parent), m_width(width), m_height(height), m_colorTablePower(power) {
	setupUi(this);
	setWindowTitle("Edit sprite sheet");

	init();
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

	// == width ==
	comboBox_width->setCurrentIndex(getSizeIndex(width));

	// == height ==
	comboBox_height->setCurrentIndex(getSizeIndex(height));

	// == color ==
	if (power == 5)
		comboBox_colors->setCurrentIndex(1); //32
	else if (power == 6)
		comboBox_colors->setCurrentIndex(2); //64
	else if (power == 8)
		comboBox_colors->setCurrentIndex(3); //256
	else
		comboBox_colors->setCurrentIndex(0); //16

	// == external ==
	if (externalId >= 0) {
		switch (externalId)
		{
		case 2: comboBox_external->setCurrentIndex(1); break;
		case 3: comboBox_external->setCurrentIndex(2); break;
		case 4: comboBox_external->setCurrentIndex(3); break;
		case 5: comboBox_external->setCurrentIndex(4); break;
		case 6: comboBox_external->setCurrentIndex(5); break;
		case 7: comboBox_external->setCurrentIndex(6); break;
		default: comboBox_external->setCurrentIndex(7); break;
		}
		spinBox_external->setValue(externalId);
	}

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejected()));
	connect(pushButton_delete, SIGNAL(clicked()), this, SLOT(remove()));
}

void CreateEmptySpriteSheet::init() {
	// == width ==
	comboBox_width->addItem("32");
	comboBox_width->addItem("64");
	comboBox_width->addItem("128");
	comboBox_width->addItem("256");
	comboBox_width->addItem("512");
	comboBox_width->setCurrentIndex(3);

	// == height ==
	comboBox_height->addItem("32");
	comboBox_height->addItem("64");
	comboBox_height->addItem("128");
	comboBox_height->addItem("256");
	comboBox_height->addItem("512");
	comboBox_height->setCurrentIndex(3);

	// == color ==
	comboBox_colors->addItem("16");
	comboBox_colors->addItem("32");
	comboBox_colors->addItem("64");
	comboBox_colors->addItem("256");

	// == external ==
	comboBox_external->addItem("No");
	comboBox_external->addItem("2 (Swort)");
	comboBox_external->addItem("3 (Speer)");
	comboBox_external->addItem("4 (Bow)");
	comboBox_external->addItem("5 (Unknown)");
	comboBox_external->addItem("6 (Axe)");
	comboBox_external->addItem("7 (Staff)");
	comboBox_external->addItem("Custom ID");
	spinBox_external->setValue(-1);

	connect(comboBox_external, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int i) {
		const bool settingsEnabled = (i == 0);
		comboBox_width->setEnabled(settingsEnabled);
		comboBox_height->setEnabled(settingsEnabled);
		comboBox_colors->setEnabled(settingsEnabled);
		
		spinBox_external->setEnabled(i == 7);
		switch (i)
		{
		case 1: spinBox_external->setValue(2); break;
		case 2: spinBox_external->setValue(3); break;
		case 3: spinBox_external->setValue(4); break;
		case 4: spinBox_external->setValue(5); break;
		case 5: spinBox_external->setValue(6); break;
		case 6: spinBox_external->setValue(7); break;
		case 7: spinBox_external->setValue(0); break;
		default: spinBox_external->setValue(-1); break;
		}
	});
}

bool CreateEmptySpriteSheet::isAccepted() const {
	return m_accepted;
}

bool  CreateEmptySpriteSheet::isDelete() const {
	return m_accepted == 2;
}

int CreateEmptySpriteSheet::getWidth() const {
	return m_width;
}

int CreateEmptySpriteSheet::getHeight() const {
	return m_height;
}

int CreateEmptySpriteSheet::getColorTablePower() const {
	return m_colorTablePower;
}

int CreateEmptySpriteSheet::getColorTableSize() const {
	return 2 ^ m_colorTablePower;
}

bool CreateEmptySpriteSheet::getResizeSprites() const {
	return m_resizeSprites;
}

int CreateEmptySpriteSheet::getExternalID() const {
	return spinBox_external->value();
}

void CreateEmptySpriteSheet::accepted() {
	m_width = comboBox_width->currentText().toInt();
	m_height = comboBox_height->currentText().toInt();

	int colors = comboBox_colors->currentText().toInt();
	if (colors == 16) m_colorTablePower = 4;
	else if (colors == 32) m_colorTablePower = 5;
	else if (colors == 64) m_colorTablePower = 6;
	else if (colors == 256) m_colorTablePower = 8;
	else {
		qInfo() << "Invalid color table size!";
		m_width = 0;
		m_height = 0;
		m_colorTablePower = 0;
		m_accepted = false;
		emit cancel();
		close();
	}
	m_resizeSprites = checkBox_resizeSprites->isChecked();
	m_accepted = 1;

	emit ok(m_width, m_height);
	close();
}

void CreateEmptySpriteSheet::rejected() {
	m_width = 0;
	m_height = 0;
	m_colorTablePower = 0;
	m_accepted = 0;
	emit cancel();
	close();
}

void CreateEmptySpriteSheet::remove() {
	m_accepted = 2;
	m_width = 0;
	m_height = 0;
	m_colorTablePower = 0;
	emit removed();
	close();
}


CreateEmptySpriteSheet::~CreateEmptySpriteSheet() {
	// TODO Auto-generated destructor stub
}

