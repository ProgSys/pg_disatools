/*
 *  Copyright (C) 2016  ProgSys
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef TREESORT_H_
#define TREESORT_H_

#include <QtCore>
#include <QSortFilterProxyModel>


class TreeSort : public QSortFilterProxyModel{
	Q_OBJECT
public:
	TreeSort(QObject *parent = 0);

	void setFilterFileExtention(const QString& fileExt, bool filter = true);

	virtual ~TreeSort();
protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:

	QList<QString> m_filterList;

};

#endif /* TREESORT_H_ */
