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
#ifndef INCLUDE_SPRITESHEETEDITOR_H_
#define INCLUDE_SPRITESHEETEDITOR_H_

#include <QString>
#include <QMainWindow>
#include <QColor>

namespace Ui {
	class SpriteSheetEditorWindow;
}


class SpriteSheetEditor: public QMainWindow {
	 Q_OBJECT
public:
	explicit SpriteSheetEditor(QWidget *parent = 0);
	virtual ~SpriteSheetEditor();
public slots:
	void open();
	void open(const QString& file);
	void addAnimation(const QString& animation);
	void addAnimations(const QStringList& animations);

	void dump();
	void exportSprites(const QString& filetype);

	void setCurrentFrame(unsigned int currFrame);
	void setTotalFrames(unsigned int totalFrames);

signals:
	bool openSprite(const QString& filepath);
	bool dumpSprite(const QString& filepath);
	int exportSprites(const QString& folder, const QString& filetype);

	void backgroundColorSelected(const QColor& color);

private:
	Ui::SpriteSheetEditorWindow *ui;

	void setTitel();
	void setTitel(const QString& filename);

private slots:
	void clickPlayPause();
	void pickBackgroundColor();
};

#endif /* INCLUDE_SPRITESHEETEDITOR_H_ */
