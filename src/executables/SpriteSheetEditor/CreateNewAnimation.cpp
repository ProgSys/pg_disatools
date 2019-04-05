/*
 * CreateNewAnimation.cpp
 *
 *  Created on: 26.08.2016
 *      Author: ProgSys
 */

#include <CreateNewAnimation.h>
#include <QIntValidator>
#include <QMessageBox>

CreateNewAnimation::CreateNewAnimation(SpriteData* spriteData, QWidget *parent):
QDialog(parent){
	setupUi(this);
	m_spriteData = spriteData;

	this->setWindowTitle("Create new animation");

	lineEdit_ID->setValidator(new QIntValidator(0, 99999,this));
	//textEdit_2
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejected()));
}

bool CreateNewAnimation::isAccepted() const{
	return m_accepted;
}

QString CreateNewAnimation::getName() const{
	return m_name;
}

int CreateNewAnimation::getID() const{
	return m_ID;
}

void CreateNewAnimation::accepted(){
	if(!m_spriteData){
		m_ID = 0;
		m_name = "";
		m_accepted = false;
		emit cancel();
		close();
	}

	m_ID = lineEdit_ID->text().toInt();
	m_name = lineEdit_name->text();

	for(const SpriteAnimation* ani:  m_spriteData->getAnimations()){
		if(ani->getID() == m_ID){
			m_accepted = false;
			m_ID = 0;
			m_name = "";
			QMessageBox::critical(0,"ID is not unique!","ID is not unique! Animation '"+ani->getName()+"' is using this ID!");
			return;
		}
	}
	m_accepted = true;
	emit ok(m_name, m_ID);
	close();

}

void CreateNewAnimation::rejected(){
	m_ID = 0;
	m_name = "";
	m_accepted = false;
	emit cancel();
	close();
}

CreateNewAnimation::~CreateNewAnimation() {
	// TODO Auto-generated destructor stub
}

