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

void FileInfoBox::setModel(const PG::FILE::fileProperties& info){
	ui->label_Name->setText(QString::fromStdString(info.info.name.getPath()));
	ui->label_Size->setText(QString::number(info.info.size));
	ui->label_offset->setText(QString::number(info.info.offset));

	switch (info.info.fileType) {
		case PG::FILE::fileInfo::PSPFS_V1:
				ui->label_type->setText("PSPFS_V1");
			break;
		case PG::FILE::fileInfo::OLA:
				ui->label_type->setText("Offset list archive");
			break;
		case PG::FILE::fileInfo::COLA:
				ui->label_type->setText("IMY compressed offset list archive");
			break;
		case PG::FILE::fileInfo::SOLA:
				ui->label_type->setText("Sprites offset list archive");
			break;
		case PG::FILE::fileInfo::TX2:
				ui->label_type->setText("TX2");
			break;
		case PG::FILE::fileInfo::IMY:
				ui->label_type->setText("IMY");
			break;
		case PG::FILE::fileInfo::MPP:
				ui->label_type->setText("MPP");
			break;
		case PG::FILE::fileInfo::SH:
				ui->label_type->setText("Sprite");
			break;
		case PG::FILE::fileInfo::GEO:
				ui->label_type->setText("Geomety");
			break;
		default:
			ui->label_type->setText("Unknown");
			break;
	}

	if(info.info.isExternalFile())
		ui->label_external->setText("Yes");

	if(info.info.isPackage())
		ui->label_pack->setText("Yes");

	if(info.info.isCompressed())
		ui->label_imy->setText("Yes");

	if(info.info.isTexture())
		ui->label_img->setText(QString::fromStdString(info.textureCompression));

}

void FileInfoBox::on_btnClose_clicked()
{
    close();
}
