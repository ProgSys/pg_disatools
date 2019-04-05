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

#include <Util/Misc/TreeItem.h>
#include <QString>
#include <DataFile.h>
#include <QList>

#include <QColor>

struct rowFormat{
	enum type{ ZERO, INT,UINT, SHIFT_JIS, UNICODE };
	rowFormat(){}
	rowFormat(type rowTypeIn, int byteSizeIn):rowType(rowTypeIn), byteSize(byteSizeIn){}

	type rowType = UINT;
	int byteSize = -1;
};

struct headerFormat{
	enum type{ KEEP, ROW_SIZE };
	headerFormat(){}
	headerFormat(rowFormat::type rowTypeIn, int byteSizeIn, type headerTypeIn): format(rowTypeIn,byteSizeIn),headerType(headerTypeIn) {}

	rowFormat format;
	type headerType = KEEP;
};

struct column{
	enum columnFLAGS{ NONE = Qt::ItemIsEditable, NOEDIT = Qt::NoItemFlags};
	enum columnType{ DEFAULT, INDEX};

	QString name = "Noname";
	columnFLAGS flag = NONE;
	columnType type = DEFAULT;
	int formatIndex = -1;

	bool hasColor = false;
	QColor colorMinBackground = QColor(255,255,255);
	QColor colorMaxBackground = QColor(255,255,255);
	int colorMin = -1;
	int colorMax = -1;

	column(){}
	column(const QString& nameIn): name(nameIn){}

	bool operator==(const QString& str) const{
		return name == str;
	}
};

struct parse{
	QList<column> header;
	QList<headerFormat> headerFormats;
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

	int getColumnWidth(int index) const;

	QVariant data(const QModelIndex &index, int role) const final;
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
	QList<QVariant> m_headerData;
};

