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
	CreateEmptySpriteSheet(int width, int height, int power, QWidget *parent = 0);

	bool isAccepted() const;
	bool isDelete() const;
	int getWidth() const;
	int getHeight() const;
	int getColorTablePower() const;
	int getColorTableSize() const;
	bool getResizeSprites() const;

	virtual ~CreateEmptySpriteSheet();
public slots:
	void accepted();
	void remove();
	void rejected();
signals:
	void ok(int width, int height);
	void removed();
	void cancel();
private:
	 unsigned char m_accepted = 0;
	 int m_width = 0;
	 int m_height = 0;
	 int m_colorTablePower = 0;
	 bool m_resizeSprites = true;
};

#endif /* INCLUDE_SPRITESHEETEDITOR_CREATEEMPTYSPRITESHEET_H_ */
