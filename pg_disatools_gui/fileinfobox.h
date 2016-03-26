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
#ifndef FILEINFOBOX_H
#define FILEINFOBOX_H

#include <QDialog>
#include <pg/files/PG_ExtractorBase.h>

namespace Ui {
class FileInfoBox;
}

class FileInfoBox : public QDialog
{
    Q_OBJECT

public:
    explicit FileInfoBox(QWidget *parent = 0);
    void setModel(const PG::FILE::fileProperties& info);
    ~FileInfoBox();

private slots:
    void on_btnClose_clicked();

private:
    Ui::FileInfoBox *ui;
};

#endif // FILEINFOBOX_H
