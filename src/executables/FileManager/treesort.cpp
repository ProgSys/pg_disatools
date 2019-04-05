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

#include <treesort.h>

TreeSort::TreeSort(QObject *parent):
QSortFilterProxyModel(parent){
	m_searchDelay.setSingleShot(true);
	m_searchDelay.setInterval(500);

	connect(&m_searchDelay, &QTimer::timeout, this, &TreeSort::invalidateFilter);
}

TreeSort::~TreeSort() {
	// TODO Auto-generated destructor stub
}

void TreeSort::setSearchText(const QString& text) {
	if (m_searchText != text) {
		m_searchText = text;
		m_searchDelay.start(); //restart search timer
	}
}

bool TreeSort::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const{
	QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

	const QString& data = sourceModel()->data(index0).toString();

	if(!filterRegExp().isEmpty() && data.contains(filterRegExp()) ) return false;

	QString fileExtention = sourceModel()->index(sourceRow, 2, sourceParent).data(Qt::DisplayRole).toString();
	for (const QString& exp : m_filterList) {
		if (fileExtention.contains(exp)) return false;
	}

	if (!m_searchText.isEmpty()) {
		QString name = sourceModel()->index(sourceRow, 0, sourceParent).data(Qt::DisplayRole).toString();
		return name.contains(m_searchText, Qt::CaseInsensitive);
	}

	return true;
}


bool TreeSort::lessThan(const QModelIndex &left, const QModelIndex &right) const{
	QVariant leftData = sourceModel()->data(left);
	QVariant rightData = sourceModel()->data(right);

	if(leftData.type() == QVariant::String){
		return leftData.toString() < rightData.toString();
	}else
		return leftData.toInt() < rightData.toInt();

}

void TreeSort::setFilterFileExtention(const QString& fileExt, bool filter){
	if(filter){
		m_filterList << fileExt;
	}else{
		m_filterList.removeAll(fileExt);
	}
	invalidateFilter();
}

