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
#ifndef SRC_FILES_PARSERDAT_H_
#define SRC_FILES_PARSERDAT_H_

#include <util/TreeItem.h>
#include <QString>
#include <files/DataFile.h>
#include <QList>

struct rowFormat{
	enum type{ ZERO, INT,UINT, SHIFT_JIS };
	rowFormat(){}
	rowFormat(type rowTypeIn, int byteSizeIn):rowType(rowTypeIn), byteSize(byteSizeIn){}

	type rowType = UINT;
	int byteSize = -1;
};

struct column{
	enum columnFLAGS{ NONE = Qt::ItemIsEditable, NOEDIT = Qt::NoItemFlags};
	enum columnType{ DEFAULT, INDEX};
	QString name = "Noname";
	columnFLAGS flag = NONE;
	columnType type = DEFAULT;

	column(){}
	column(const QString& nameIn): name(nameIn){}

	bool operator==(const QString& str){
		return name == str;
	}
};

struct parse{
	QList<column> header;
	QList<rowFormat> formats;

	void clear(){
		header.clear();
		formats.clear();
	}
};

class ParserDAT: public DataFile  {
	Q_OBJECT
public:
	ParserDAT(const QString& defFile, QObject *parent = 0);
	virtual ~ParserDAT();
    Qt::ItemFlags flags(const QModelIndex &index) const final;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) final;

public slots:
	bool open(const QString& filepath) final;
	bool save(const QString& filepath) final;

	bool insertFront() final;
	bool insertBack() final;
	bool insertAt(int index) final;
	bool removeAt(int index) final;
private:
	parse m_dataStructure;
};

#endif /* SRC_FILES_PARSERDAT_H_ */
