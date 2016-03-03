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


#include "treemodel.h"

#include <iostream>
#include <QGraphicsPixmapItem>
#include <pg/util/PG_Image.h>

TreeModel::TreeModel(QObject *parent)
    :QAbstractItemModel(parent){

}

TreeModel::TreeModel(const QString &data, QObject *parent)
    :QAbstractItemModel(parent){

	m_pspfsFile = PG::FILE::PSPFS(data.toStdString());

}

TreeModel::~TreeModel(){

}

void TreeModel::openFile(const QString &file){
	QAbstractItemModel::layoutAboutToBeChanged();
	m_pspfsFile.open(file.toStdString());
	QAbstractItemModel::layoutChanged();

}

void TreeModel::extractFile(const QString &file, const QString &dir) const{
	const std::string targetFile = dir.toStdString() +"/"+file.toStdString();
	if(m_pspfsFile.extract(file.toStdString(), targetFile)){
		qInfo() << "Couldn't extract "<<file;
	}
}

bool TreeModel::addFile(const QString &file){
	bool added = false;
	QAbstractItemModel::layoutAboutToBeChanged();
	if(m_pspfsFile.insert(file.toStdString())){
		qInfo() << "Couldn't add file '"<<file<<"'";
	}else
		added = true;
	QAbstractItemModel::layoutChanged();
	return added;
}

int TreeModel::addFiles(const QStringList &files){
	if(files.empty()) return 0;
	QAbstractItemModel::layoutAboutToBeChanged();
	int filesAddedOrChanged = 0;
	 for(const QString& str: files){
		 if(m_pspfsFile.insert(str.toStdString())){
		 		qInfo() << "Couldn't add file: '"<<str<<"'";
		 }else
			 filesAddedOrChanged++;
	 }
	QAbstractItemModel::layoutChanged();
	return filesAddedOrChanged;
}

int TreeModel::removeFiles(const QStringList &files){
	if(files.empty()) return 0;
	QAbstractItemModel::layoutAboutToBeChanged();
	int filesRemoved = 0;
	for(const QString& str: files){
		 if(m_pspfsFile.remove(str.toStdString())){
		 		qInfo() << "Couldn't remove file: '"<<str<<"'";
		 }else
			 filesRemoved++;
	}
	QAbstractItemModel::layoutChanged();
	return filesRemoved;
}

bool TreeModel::hasDataChanged() const{
	return m_pspfsFile.isChanged();
}

bool TreeModel::save(){
	if(!m_pspfsFile.save())
		return true;
	else{
		//if saving faled try to reopen it
		QAbstractItemModel::layoutAboutToBeChanged();
		PG::UTIL::File file = m_pspfsFile.getFile();
		return m_pspfsFile.open(file);
		QAbstractItemModel::layoutChanged();
	}
}

bool TreeModel::setGraphicsScene(const QString &file, QGraphicsScene* scene) const{
	PG::UTIL::RGBAImage img;
	if(m_pspfsFile.extractImage(file.toStdString(), img, false)){
		qInfo() << "Couldn't extract image file: '"<<file<<"'";
		return true;
	}

	if(img.getWidth() == 0 || img.getHeight() == 0){
		qInfo() << "PG_Image width ot height is zero!";
		return true;
	}

	QImage qimg( &(img[0].r), img.getWidth() , img.getHeight(), QImage::Format_RGBA8888 );

	QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qimg));
	scene->clear();
	scene->addItem(item);

	return false;



}

void TreeModel::hideFiles(const QString &extention, bool hide){
	if(hide){
		auto it = std::find(m_hideExtentions.begin(), m_hideExtentions.end(), extention.toStdString());
		if(it == m_hideExtentions.end()){
			m_hideExtentions.push_back(extention.toStdString());
			QAbstractItemModel::layoutChanged();
		}
	}else{
		auto it = std::find(m_hideExtentions.begin(), m_hideExtentions.end(), extention.toStdString());
		if(it != m_hideExtentions.end()){
			m_hideExtentions.erase(it);
			QAbstractItemModel::layoutChanged();
		}
	}
}

QVariant TreeModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    const PG::FILE::filePSPFSInfo *item = static_cast<const PG::FILE::filePSPFSInfo*>(index.internalPointer());

    if(item == nullptr)
    	 return QVariant();

    switch (index.column()) {
    		case 0:
    				return QVariant(QString::fromStdString(item->name));
    			break;
    		case 1:
    				return QVariant(item->size);
    			break;
    		case 2:
    				return QVariant(QString::fromStdString(item->getFileExtention()));
    			break;
    		default:
    			return QModelIndex();
    			break;
    	}
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole){
        switch (section) {
        case 0:
            return QVariant(QString("File"));
            break;
        case 1:
            return QVariant(QString("Size"));
            break;
        case 2:
            return QVariant(QString("Format"));
            break;
        default:
             return QVariant();
            break;
        }
    }
       // return rootItem->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if(row >= m_pspfsFile.getFileInfos().size())
    	return QModelIndex();

    PG::FILE::filePSPFSInfo* info = m_pspfsFile.getDataPointer(row);

    return createIndex(row, column, info); //QModelIndex( file );

}

QModelIndex TreeModel::parent(const QModelIndex &index) const{
    if (!index.isValid())
         return QModelIndex();

    return QModelIndex();

}

int TreeModel::rowCount(const QModelIndex &parent) const{
	if (parent.isValid())
		 return 0;

	return m_pspfsFile.getFileInfos().size();
}

bool TreeModel::saveFile(){
    return save();
}

bool TreeModel::saveFileAs(const QString& filepath){
    if(!m_pspfsFile.save(filepath.toStdString()))
        return true;
    else{
        //if saving faled try to reopen it
        QAbstractItemModel::layoutAboutToBeChanged();
        PG::UTIL::File file = m_pspfsFile.getFile();
        return m_pspfsFile.open(file);
        QAbstractItemModel::layoutChanged();
    }

    return false;
}

int TreeModel::columnCount(const QModelIndex &parent) const{
	if (parent.isValid())
		 return 0;
	return 3;

}

