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

#include "ui_ResizeDialog.h"

#include <QWidget>
#include <QDialog>
#include <QValidator>

class ResizeDialog : public QDialog, public Ui::ResizeDialog {
	Q_OBJECT
public:
	ResizeDialog(int sheetId, int fromWidth, int fromHeight, int toWidth, int toHeight, QWidget* parent = 0);

	inline bool isAccepted() const { return m_accepted; }
	inline int getFromWidth() const { return m_fromWidth; }
	inline int getFromHeight() const { return m_fromHeight; }
	inline int getToWidth() const { return m_toWidth; }
	inline int getToHeight() const { return m_toHeight; }

public slots:
	void accepted();
	void rejected();
signals:
	void cancel();
private:
	void init();

	bool m_accepted = false;
	int m_sheetId = 0;
	int m_fromWidth = 0;
	int m_fromHeight = 0;
	int m_toWidth = 0;
	int m_toHeight = 0;
};
