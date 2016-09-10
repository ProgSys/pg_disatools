/*
 * CreateNewAnimation.h
 *
 *  Created on: 26.08.2016
 *      Author: ProgSys
 */

#ifndef SRC_SPRITESHEETEDITOR_CREATENEWANIMATION_H_
#define SRC_SPRITESHEETEDITOR_CREATENEWANIMATION_H_

#include "ui_createNewAnimation.h"

#include <QWidget>
#include <QDialog>
#include <QValidator>
#include <spriteSheetEditor/SpriteData.h>
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
