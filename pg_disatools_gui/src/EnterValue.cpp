/*
 * EnterValue.cpp
 *
 *  Created on: 08.04.2016
 *      Author: ProgSys
 */

#include <EnterValue.h>
#include "ui_entervaluedialog.h"

#include <QIntValidator>
#include <QMessageBox>

EnterValue::EnterValue(QWidget *parent):
EnterValue(nullptr, "Please enter a value", "Please enter a value:", parent)
{}

EnterValue::EnterValue(
		QValidator* validator,
		const QString& title,
		const QString& infotext,
		QWidget *parent
		):QDialog(parent),
	ui(new Ui::Dialog)
{
	ui->setupUi(this);

	//setWindowFlags(Qt::WindowStaysOnTopHint);

	this->setWindowTitle(title);
	ui->label_info->setText(infotext);
	ui->lineEdit->setValidator(validator);
	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(rejected()));
}

bool EnterValue::isAccepted() const{
	return m_accepted;
}

const QString& EnterValue::getResult() const{
	return m_enterValue;
}


void EnterValue::accepted(){
	m_enterValue = ui->lineEdit->text();

	if(ui->lineEdit->validator()){
		int pos = 0;
		if(ui->lineEdit->validator()->validate(m_enterValue, pos) == QValidator::State::Acceptable){
			m_accepted = true;
			emit ok(m_enterValue);
			close();
		}else{
			QMessageBox::critical(0,"Value invalid!","Entered value is invalid!");
		}
	}else{
		emit ok(m_enterValue);
		close();
	}

}

void EnterValue::rejected(){
	m_enterValue = "";
	m_accepted = false;
	emit cancel();
	close();
}

bool EnterValue::openEnterIntDialog(
		int& outInt,
		int rangeStart,
		int rangeEnd,
		const QString& title,
		const QString& infotext,
		QWidget *parent
		){
	EnterValue enterVal(new QIntValidator(rangeStart, rangeEnd), title,infotext,parent);
	enterVal.exec();

	if(enterVal.isAccepted()){
		outInt = enterVal.getResult().toInt();
		return true;
	}else{
		return false;
	}

	//QIntValidator tester(rangeStart, rangeEnd);
	//tester.validate()


}

EnterValue::~EnterValue() {

}

