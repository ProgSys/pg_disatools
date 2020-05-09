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

#pragma once

#include "ui_createNewSpriteSheet.h"

#include <QWidget>
#include <QDialog>
#include <QValidator>

class CreateEmptySpriteSheet : public QDialog, public Ui::NewSpriteSheetDialog {
	Q_OBJECT
public:
	CreateEmptySpriteSheet(QWidget* parent = 0);
	CreateEmptySpriteSheet(int width, int height, int power, int externalId, QWidget* parent = 0);

	bool isAccepted() const;
	bool isDelete() const;
	inline int getWidth() const { return m_width; }
	inline int getHeight() const { return m_height; }

	inline int getOriginalWidth() const { return m_originalWidth; }
	inline int getOriginalHeight() const { return m_originalHeight; }
	inline bool isResized() const { return m_width != m_originalWidth || m_height != m_originalHeight; }

	inline int getColorTablePower() const { return m_colorTablePower; }
	inline int getColorTableSize() const { return std::pow(2, m_colorTablePower); }
	inline bool isResizeSprites() const { return m_resizeSprites;}
	int getExternalID() const;

	virtual ~CreateEmptySpriteSheet();
public slots:
	void accepted();
	void remove();
	void rejected();
signals:
	void ok(int width, int height);
	void removed();
	void cancel();
private slots:
	void externalIndexChanged(int i);
private:
	void init();

	bool m_isNew = false;
	unsigned char m_accepted = 0;
	int m_width = 0;
	int m_height = 0;	
	
	int m_originalWidth = 0;
	int m_originalHeight = 0;
	int m_colorTablePower = 0;
	bool m_resizeSprites = true;
};

