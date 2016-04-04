/*
 * SpriteSheetEditorWindow.h
 *
 *  Created on: 31.03.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_SPRITESHEETEDITOR_H_
#define INCLUDE_SPRITESHEETEDITOR_H_

#include <QMainWindow>

namespace Ui {
	class SpriteSheetEditorWindow;
}


class SpriteSheetEditor: public QMainWindow {
	 Q_OBJECT
public:
	explicit SpriteSheetEditor(QWidget *parent = 0);
	virtual ~SpriteSheetEditor();
private:
	Ui::SpriteSheetEditorWindow *ui;
};

#endif /* INCLUDE_SPRITESHEETEDITOR_H_ */
