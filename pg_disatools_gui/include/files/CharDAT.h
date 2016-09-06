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
#ifndef INCLUDE_FILES_CHARDAT_H_
#define INCLUDE_FILES_CHARDAT_H_

#include <files/DataFile.h>

class CharDAT: public DataFile {
	Q_OBJECT
public:
	CharDAT(QObject *parent = 0);
	virtual ~CharDAT();

bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

public slots:
	bool open(const QString& filepath) final;
	bool save(const QString& filepath) final;
};

#endif /* INCLUDE_FILES_CHARDAT_H_ */
