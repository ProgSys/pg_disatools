/*
 * CreateEmptySpriteSheet.cpp
 *
 *  Created on: 25.08.2016
 *      Author: ProgSys
 */

#include <CreateEmptySpriteSheet.h>

#include <QDebug>

constexpr auto g_auto3xStr = "Auto3x";

CreateEmptySpriteSheet::CreateEmptySpriteSheet(QWidget* parent) :
	QDialog(parent)
{
	setupUi(this);
	setWindowTitle("Create empty sprite sheet");
	m_isNew = true;

	init(false);

	pushButton_delete->close();
	checkBox_resizeSprites->close();
	externalIndexChanged(0);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejected()));
}

CreateEmptySpriteSheet::CreateEmptySpriteSheet(int width, int height, int power, int externalId, QWidget* parent) :
	QDialog(parent), m_width(width), m_height(height), m_originalWidth(width), m_originalHeight(height), m_colorTablePower(power) {
	setupUi(this);
	setWindowTitle("Edit sprite sheet");

	init(true);
	auto getSizeIndex = [](int size) {
		switch (size)
		{
		case 32: return 0;
		case 64: return 1;
		case 128: return 2;
		case 256: return 3;
		case 512: return 4;
		case 1024: return 5;
		case 2048: return 6;
		}
		return 3;
	};

	// == width ==
	comboBox_width->setCurrentIndex(getSizeIndex(width));
	comboBox_OriginalWidth->setCurrentIndex(comboBox_width->currentIndex());

	// == height ==
	comboBox_height->setCurrentIndex(getSizeIndex(height));
	comboBox_OriginalHeight->setCurrentIndex(comboBox_height->currentIndex());

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
	externalIndexChanged(externalId);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejected()));
	connect(pushButton_delete, SIGNAL(clicked()), this, SLOT(remove()));
}

void CreateEmptySpriteSheet::init(bool isEdit) {
	auto addSizeOptions = [isEdit](QComboBox* box, bool isSize = false) {
		box->addItem("32");
		box->addItem("64");
		box->addItem("128");
		box->addItem("256");
		box->addItem("512");
		box->addItem("1024");
		box->addItem("2048");
		if(isEdit && isSize) box->addItem(QString(g_auto3xStr) + " (HD)");
		box->setCurrentIndex(3);
	};

	// == width ==
	addSizeOptions(comboBox_width, true);
	addSizeOptions(comboBox_OriginalWidth);

	// == height ==
	addSizeOptions(comboBox_height, true);
	addSizeOptions(comboBox_OriginalHeight);

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

	connect(spinBox_external, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CreateEmptySpriteSheet::externalIndexChanged);

	connect(comboBox_external, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int i) {
		switch (i)
		{
		case 1: spinBox_external->setValue(2); break;
		case 2: spinBox_external->setValue(3); break;
		case 3: spinBox_external->setValue(4); break;
		case 4: spinBox_external->setValue(5); break;
		case 5: spinBox_external->setValue(6); break;
		case 6: spinBox_external->setValue(7); break;
		case 7: spinBox_external->setValue(1); break;
		default: spinBox_external->setValue(-1); break;
		}
		spinBox_external->setEnabled(i == 7);
	});
}

void CreateEmptySpriteSheet::externalIndexChanged(int i) {
	const bool showOriginalSize = !m_isNew && (i > 0);
	comboBox_OriginalWidth->setEnabled(showOriginalSize);
	comboBox_OriginalWidth->setVisible(showOriginalSize);
	label_OriginalWidth->setVisible(showOriginalSize);

	comboBox_OriginalHeight->setEnabled(showOriginalSize);
	comboBox_OriginalHeight->setVisible(showOriginalSize);
	label_OriginalHeight->setVisible(showOriginalSize);
}

bool CreateEmptySpriteSheet::isAccepted() const {
	return m_accepted;
}

bool  CreateEmptySpriteSheet::isDelete() const {
	return m_accepted == 2;
}

int CreateEmptySpriteSheet::getExternalID() const {
	return spinBox_external->value();
}

void CreateEmptySpriteSheet::accepted() {
	const auto getSize = [](const QComboBox& box) {
		if (box.currentText().startsWith(g_auto3xStr)) {
			return auto3xResizeMarker;
		}
		else {
			return box.currentText().toInt();
		}
	};

	m_width = getSize(*comboBox_width);
	m_height = getSize(*comboBox_height);

	m_originalWidth = comboBox_OriginalWidth->currentText().toInt();
	m_originalHeight = comboBox_OriginalHeight->currentText().toInt();

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

