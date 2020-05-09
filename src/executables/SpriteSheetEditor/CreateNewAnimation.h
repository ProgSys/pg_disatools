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

#ifndef SRC_SPRITESHEETEDITOR_CREATENEWANIMATION_H_
#define SRC_SPRITESHEETEDITOR_CREATENEWANIMATION_H_

#include "ui_createNewAnimation.h"

#include <QWidget>
#include <QDialog>
#include <QValidator>
#include <SpriteData.h>
#include <QString>

class CreateNewAnimation: public QDialog, public Ui::NewAnimationDialog {
	Q_OBJECT
public:
	CreateNewAnimation(SpriteData* spriteData, QWidget *parent = 0);

	bool isAccepted() const;
	QString getName() const;
	int getID() const;

	virtual ~CreateNewAnimation();

public slots:
	void accepted();
	void rejected();
signals:
	void ok(QString str, int id);
	void cancel();
private:
	bool m_accepted = false;
	SpriteData* m_spriteData = nullptr;
	QString m_name;
	int m_ID = 0;
};

#endif /* SRC_SPRITESHEETEDITOR_CREATENEWANIMATION_H_ */
