/*
 * CreateEmptySpriteSheet.cpp
 *
 *  Created on: 25.08.2016
 *      Author: ProgSys
 */

#include <CreateEmptySpriteSheet.h>

#include <QDebug>

CreateEmptySpriteSheet::CreateEmptySpriteSheet(QWidget *parent):
QDialog(parent)
{
	setupUi(this);
	setWindowTitle("Create empty sprite sheet");
	comboBox_width->addItem("128");

	comboBox_width->addItem("256");
	comboBox_width->addItem("512");
	comboBox_width->setCurrentIndex(1);

	comboBox_height->addItem("128");
	comboBox_height->addItem("256");
	comboBox_height->addItem("512");
	comboBox_height->setCurrentIndex(1);

	//comboBox_colors->setEnabled(false);
	comboBox_colors->addItem("16");
	comboBox_colors->addItem("32");
	comboBox_colors->addItem("64");
	comboBox_colors->addItem("256");

	pushButton_delete->close();
	checkBox_resizeSprites->close();

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejected()));
}

CreateEmptySpriteSheet::CreateEmptySpriteSheet(int width, int height, int power, QWidget *parent):
	QDialog(parent), m_width(width), m_height(height), m_colorTablePower(power){
	setupUi(this);
	setWindowTitle("Edit sprite sheet");
	comboBox_width->addItem("128");

	comboBox_width->addItem("256");
	comboBox_width->addItem("512");
	if(width == 256)
		comboBox_width->setCurrentIndex(1);
	else if(width == 512)
		comboBox_width->setCurrentIndex(2);

	comboBox_height->addItem("128");
	comboBox_height->addItem("256");
	comboBox_height->addItem("512");
	if(height == 256)
		comboBox_height->setCurrentIndex(1);
	else if(height == 512)
		comboBox_height->setCurrentIndex(2);

	//comboBox_colors->setEnabled(false);
	comboBox_colors->addItem("16");
	comboBox_colors->addItem("32");
	comboBox_colors->addItem("64");
	comboBox_colors->addItem("256");

	if(power == 32)
		comboBox_colors->setCurrentIndex(1);
	else if(power == 64)
		comboBox_colors->setCurrentIndex(2);
	else if(power == 256)
		comboBox_colors->setCurrentIndex(3);


	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejected()));
	connect(pushButton_delete, SIGNAL(clicked()), this, SLOT(remove()));
}

bool CreateEmptySpriteSheet::isAccepted() const{
	return m_accepted;
}

bool  CreateEmptySpriteSheet::isDelete() const{
	return m_accepted == 2;
}

int CreateEmptySpriteSheet::getWidth() const{
	return m_width;
}

int CreateEmptySpriteSheet::getHeight() const{
	return m_height;
}

int CreateEmptySpriteSheet::getColorTablePower() const{
	return m_colorTablePower;
}

int CreateEmptySpriteSheet::getColorTableSize() const{
	return 2^m_colorTablePower;
}

bool CreateEmptySpriteSheet::getResizeSprites() const{
	return m_resizeSprites;
}

void CreateEmptySpriteSheet::accepted(){
	m_width = comboBox_width->currentText().toInt();
	m_height = comboBox_height->currentText().toInt();

	int colors = comboBox_colors->currentText().toInt();
	if(colors == 16) m_colorTablePower = 4;
	else if(colors == 32) m_colorTablePower = 5;
	else if(colors == 64) m_colorTablePower = 6;
	else if(colors == 256) m_colorTablePower = 8;
	else{
		qInfo()<<"Invalid color table size!";
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

void CreateEmptySpriteSheet::rejected(){
	m_width = 0;
	m_height = 0;
	m_colorTablePower = 0;
	m_accepted = 0;
	emit cancel();
	close();
}

void CreateEmptySpriteSheet::remove(){
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

