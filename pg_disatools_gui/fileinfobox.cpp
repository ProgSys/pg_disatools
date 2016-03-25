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
#include "fileinfobox.h"
#include "ui_fileinfobox.h"

FileInfoBox::FileInfoBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileInfoBox)
{
    ui->setupUi(this);
}

FileInfoBox::~FileInfoBox()
{
    delete ui;
}

void FileInfoBox::setModel(const PG::FILE::fileInfo* info){
	if(!info) return;
	ui->label_Name->setText(QString::fromStdString(info->name.getPath()));
	ui->label_Size->setText(QString::number(info->size));
	ui->label_offset->setText(QString::number(info->offset));

	if(info->isExternalFile())
		ui->label_external->setText("Yes");

	if(info->isPackage())
		ui->label_pack->setText("Yes");

	if(info->isCompressed())
		ui->label_imy->setText("Yes");

	if(info->isTexture())
		ui->label_img->setText("Yes");
}

void FileInfoBox::on_btnClose_clicked()
{
    close();
}
