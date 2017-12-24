/*
 * SceneModel.cpp
 *
 *  Created on: 23.11.2017
 *      Author: ProgSys
 */

#include <dataEditor/SceneModel.h>
#include <iterator>
#include <QDebug>

SceneModel::SceneModel(QObject *parent): QAbstractItemModel(parent) {
	qDebug()<<__LINE__<<"HERE!";
	mItems.resize(TYPE_MAX);
	qDebug()<<__LINE__<<"HERE!";
	mItems[Type_A]["Dog1"] = {"Dog1", Type_A} ;
	mItems[Type_A]["Dog2"] = {"Dog2", Type_A} ;
	mItems[Type_B]["Cat1"] = {"Cat1", Type_B} ;
	mItems[Type_B]["Etna"] = {"Etna", Type_B} ;
	mItems[Type_C]["Flonne"] = {"Flonne", Type_C} ;
	qDebug()<<__LINE__<<"HERE!";
}

SceneModel::~SceneModel() {
	// TODO Auto-generated destructor stub
}

void SceneModel::add(TreeTypes type, const QString& name){
	if(name.isEmpty()) return;
	QAbstractItemModel::layoutAboutToBeChanged();
	beginInsertRows(createIndex(type,0,(quintptr)0),0,mItems[type].size());
	const std::string key =name.toStdString();
	mItems[type][key] = {key, type} ;

	endInsertRows();
	QAbstractItemModel::layoutChanged();
}

void SceneModel::remove(TreeTypes type, const QString& name){
	if(name.isEmpty()) return;
	const std::string key =name.toStdString();
	auto it = mItems[type].find(key);
	if(it == mItems[type].end()) return;

	beginRemoveRows(createIndex(type,0,(quintptr)0),0,mItems[type].size());
	mItems[type].erase(it);
	endRemoveRows();
}

QVariant SceneModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())  return QVariant();
    if (role != Qt::DisplayRole) return QVariant();
    if(index.parent().isValid()){
    	auto it = mItems[index.internalId()-1].begin();
        std::advance(it, index.row());
    	const Item& item = it->second;
        //std::advance(it, index.row());
    	return QVariant::fromValue(QString::fromStdString(item.name));
    }else
    	return QVariant::fromValue(QString::fromStdString(toString(TreeTypes(index.row()))));
}
Qt::ItemFlags SceneModel::flags(const QModelIndex &index) const{
    if (!index.isValid())return 0;
    return QAbstractItemModel::flags(index);
}
QVariant SceneModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QVariant::fromValue(QString("Header"));

    return QVariant();
}
QModelIndex SceneModel::index(int row, int column, const QModelIndex &parent) const{
    if (column > 0 || !hasIndex(row, column, parent))
        return QModelIndex();
    if(parent.isValid()){
    	return createIndex(row, column, (int)(parent.row()+1));
    }else{
    	return createIndex(row, column, (quintptr)0);
    }
}

QModelIndex SceneModel::parent(const QModelIndex &index) const{
    if (!index.isValid() || !index.internalId()) return QModelIndex();
    return createIndex( index.internalId()-1, 0, (quintptr)0);
}
int SceneModel::rowCount(const QModelIndex &parent) const{
	if(parent.isValid()){
		return (parent.internalId())? 0 : mItems[parent.row()].size();
	}else
		return mItems.size();
}

int SceneModel::columnCount(const QModelIndex&) const{
	return 1;
}
