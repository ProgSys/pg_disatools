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

#include <QString>
#include <QMainWindow>
#include <ui_tx2Editor.h>
#include <Util/PG_Image.h>
#include <Files/PG_TX2.h>


class TX2EditorModel: public QObject{
	Q_OBJECT
	Q_PROPERTY(unsigned int width READ getWidth NOTIFY imageChanged)
	Q_PROPERTY(unsigned int height READ getHeight NOTIFY imageChanged)
	Q_PROPERTY(unsigned int numberOfColors READ getNumberOfColors NOTIFY imageChanged)
	Q_PROPERTY(unsigned int numberOfColorTables READ getNumberOfColorTables NOTIFY imageChanged)
	Q_PROPERTY(int currColorTable READ getCurrentColorTable WRITE setCurrentColorTable NOTIFY imageChanged)
public:
	TX2EditorModel(QObject *parent = 0);
	virtual ~TX2EditorModel();

	unsigned int getWidth() const;
	unsigned int getHeight() const;
	unsigned int getNumberOfColors() const;
	unsigned int getNumberOfColorTables() const;

	int getCurrentColorTable() const;
	void setCurrentColorTable(int current);

	Q_INVOKABLE QColor getColor(unsigned int index ) const;
	Q_INVOKABLE QColor getColor(unsigned int index, unsigned int colorTableIndex) const;

	Q_INVOKABLE void setColor(const QColor& color, unsigned int index );
	Q_INVOKABLE void setColor(const QColor& color, unsigned int index, unsigned int colorTableIndex);

	Q_INVOKABLE void addColorTable(unsigned int atColorTableIndex, bool copy = false );
	Q_INVOKABLE void removeColorTable(unsigned int atColorTableIndex );

	Q_INVOKABLE void exportColorTable(unsigned int colorTableIndex) const;
	Q_INVOKABLE void exportColorTable(unsigned int colorTableIndex, const QString filePath) const;
	Q_INVOKABLE void importColorTable(unsigned int colorTableIndex);
	Q_INVOKABLE void importColorTable(unsigned int colorTableIndex, const QString filePath);

	PG::FILE::tx2Image* image;

public slots:
	bool openTX2(const QString& filepath);
	bool openImage(const QString& filepath);

	bool saveTX2(const QString& filepath);
	bool saveImage(const QString& filepath);

	void convertTo(PG::FILE::tx2Type type);

signals:
	void imageChanged();

private:
	int m_currentColorTable = 0;

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
	void setTitel();
	void setTitel(const QString& filename);

	TX2EditorModel* m_model;
	QString m_currentOpendFile;
};

