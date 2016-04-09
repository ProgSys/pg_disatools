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
#ifndef SRC_ENTERVALUE_H_
#define SRC_ENTERVALUE_H_

namespace Ui {
class Dialog;
}

#include <QWidget>
#include <QDialog>
#include <QValidator>

class EnterValue: public QDialog {
    Q_OBJECT
public:
	EnterValue(QWidget *parent);

	EnterValue(
			QValidator* validator = nullptr,
			const QString& title = "Please enter a value",
			const QString& infotext = "Please enter a value:",
			QWidget *parent = nullptr
			);

	bool isAccepted() const;
	const QString& getResult() const;

	virtual ~EnterValue();
public slots:
	void accepted();
	void rejected();
signals:
	void ok(const QString& string);
	void cancel();
public:
	static bool openEnterIntDialog(
			int& outInt,
			int rangeStart = 0,
			int rangeEnd = 40000,
			const QString& title = "Please enter a number",
			const QString& infotext = "Please enter a number:",
			QWidget *parent = nullptr
			);
private:
	 Ui::Dialog *ui;
	 bool m_accepted = false;
	 QString m_enterValue = "";
};



#endif /* SRC_ENTERVALUE_H_ */
