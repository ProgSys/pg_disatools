/*
 * CreateEmptySpriteSheet.h
 *
 *  Created on: 25.08.2016
 *      Author: ProgSys
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
