/*
 * MapEditor.h
 *
 *  Created on: 10.09.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_MAPEDITOR_MAPEDITOR_H_
#define INCLUDE_MAPEDITOR_MAPEDITOR_H_

#include <QString>
#include <QMainWindow>
#include <ui_mapEditor.h>

class MapEditor: public QMainWindow, public Ui::MapEditorWindow  {
	Q_OBJECT;
public:
	MapEditor(QWidget *parent = 0);
	virtual ~MapEditor();
};

#endif /* INCLUDE_MAPEDITOR_MAPEDITOR_H_ */
