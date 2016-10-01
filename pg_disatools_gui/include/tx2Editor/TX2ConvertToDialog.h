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
#include <ui_tx2ConvertTo.h>
#include <QWidget>
#include <QDialog>

#ifndef INCLUDE_TX2EDITOR_TX2CONVERTTODIALOG_H_
#define INCLUDE_TX2EDITOR_TX2CONVERTTODIALOG_H_

class TX2ConvertToDialog: public QDialog, Ui::Tx2ConvertToDialog  {
	 Q_OBJECT
public:
	TX2ConvertToDialog(QWidget *parent = 0);
	bool isAccepted() const;
	const QString& getResult() const;
	virtual ~TX2ConvertToDialog();
public slots:
	void accepted();
	void rejected();
signals:
	void ok(const QString& string);
	void cancel();
private:
	 bool m_accepted = false;
	 QString m_result= "";
};

#endif /* INCLUDE_TX2EDITOR_TX2CONVERTTODIALOG_H_ */
