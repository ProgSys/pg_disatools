/*
 * EnterValue.h
 *
 *  Created on: 08.04.2016
 *      Author: ProgSys
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
