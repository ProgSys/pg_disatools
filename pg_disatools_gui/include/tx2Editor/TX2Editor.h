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

#include <QString>
#include <QMainWindow>
#include <ui_tx2Editor.h>
#include <pg/util/PG_Image.h>
#include <pg/files/PG_TX2.h>

#ifndef INCLUDE_TX2EDITOR_TX2EDITOR_H_
#define INCLUDE_TX2EDITOR_TX2EDITOR_H_

class TX2EditorModel: public QObject{
	Q_OBJECT
	Q_PROPERTY(unsigned int width READ getWidth NOTIFY imageChanged)
	Q_PROPERTY(unsigned int height READ getHeight NOTIFY imageChanged)
public:
	TX2EditorModel(QObject *parent = 0);
	virtual ~TX2EditorModel();

	unsigned int getWidth() const;
	unsigned int getHeight() const;

	PG::FILE::tx2Image* image;

public slots:
	bool openTX2(const QString& filepath);
	bool openImage(const QString& filepath);

	bool saveTX2(const QString& filepath);
	bool saveImage(const QString& filepath);

	void convertTo(PG::FILE::tx2Type type);

signals:
	void imageChanged();

};

class TX2Editor: public QMainWindow, public Ui::TX2EditorUI {
	Q_OBJECT
public:
	TX2Editor(QWidget *parent = 0);
	virtual ~TX2Editor();
public slots:
	void open();
	void open(const QString& filepath);

	void save();
	void saveAs();
	void save(const QString& filepath);

	void convertTo();

	void updateInfo();
signals:
	bool openTX2(const QString& filepath);
	bool openImage(const QString& filepath);
	bool saveTX2(const QString& filepath);
	bool saveImage(const QString& filepath);
	void convertTo(PG::FILE::tx2Type type);
private:
	void buttonsEnable(bool enable = true);

	TX2EditorModel* m_model;
	QString m_currentOpendFile;
};

#endif /* INCLUDE_TX2EDITOR_TX2EDITOR_H_ */
