/*
 * CreateEmptySpriteSheet.h
 *
 *  Created on: 25.08.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_SPRITESHEETEDITOR_CREATEEMPTYSPRITESHEET_H_
#define INCLUDE_SPRITESHEETEDITOR_CREATEEMPTYSPRITESHEET_H_

#include "ui_createNewSpriteSheet.h"

#include <QWidget>
#include <QDialog>
#include <QValidator>

class CreateEmptySpriteSheet: public QDialog, public Ui::NewSpriteSheetDialog  {
	 Q_OBJECT
public:
	CreateEmptySpriteSheet(QWidget *parent = 0);

	bool isAccepted() const;
	int getWidth() const;
	int getHeight() const;


	virtual ~CreateEmptySpriteSheet();
public slots:
	void accepted();
	void rejected();
signals:
	void ok(int width, int height);
	void cancel();
private:
	 bool m_accepted = false;
	 int m_width = 0;
	 int m_height = 0;
};

#endif /* INCLUDE_SPRITESHEETEDITOR_CREATEEMPTYSPRITESHEET_H_ */
