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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QItemSelection>
#include "fileMenager/treemodel.h"
#include "fileMenager/treesort.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnDelete_clicked();
    void on_treeView_activated(const QModelIndex &index);
    void on_btnSave_clicked();
    void on_btnSaveAs_clicked();

    void on_btnInsert_clicked();
    void on_btnExtract_clicked();
    void on_treeView_clicked(const QModelIndex &index);

    void on_btnAbout_clicked();
    void on_btnOpen_clicked();

    void on_btnExtractImage_clicked();

    void treeSelectionChanged (const QItemSelection & sel,const  QItemSelection & desel);
    void treeContextMenu(const QPoint &pos);
    void saveSelectedImage();

signals:
    bool openFile(const QString& filepath);
	bool saveFile();
	bool saveFileAs(const QString& filepath);
	bool saveImage(const QModelIndex &index, const QString& targetfile);

private:
    Ui::MainWindow *ui;

    TreeModel* m_treeModel;
    TreeSort* m_treeSort;
    QGraphicsScene* scene;
    QItemSelectionModel* selectionModel;
    QList<QTemporaryFile*> m_tempFiles;

    bool checkValid();
};

#endif // MAINWINDOW_H
