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

#include <QtCore>
#include <QSortFilterProxyModel>
#include <QTimer>

class TreeSort : public QSortFilterProxyModel{
	Q_OBJECT
public:
	TreeSort(QObject *parent = 0);
	virtual ~TreeSort();

public slots:
	void setFilterFileExtention(const QString& fileExt, bool filter = true);
	void setSearchText(const QString& text);

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:

	QList<QString> m_filterList;
	QString m_searchText;
	QTimer m_searchDelay;
};
