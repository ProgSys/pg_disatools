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

#include <treesort.h>

TreeSort::TreeSort(QObject *parent):
QSortFilterProxyModel(parent){


}

TreeSort::~TreeSort() {
	// TODO Auto-generated destructor stub
}

bool TreeSort::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const{
	QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

	const QString& data = sourceModel()->data(index0).toString();

	if(!filterRegExp().isEmpty() && data.contains(filterRegExp()) ) return false;

	QModelIndex index2 = sourceModel()->index(sourceRow, 2, sourceParent);
	const QString& ext = sourceModel()->data(index2).toString();

	for(const QString& exp: m_filterList){
		if(ext.contains(exp)) return false;
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

