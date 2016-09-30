/*
 * TX2ConvertToDialog.h
 *
 *  Created on: 30.09.2016
 *      Author: ProgSys
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
