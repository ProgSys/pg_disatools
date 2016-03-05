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


#include "treemodel.h"

#include <iostream>
#include <QGraphicsPixmapItem>
#include <QFileInfo>
#include <pg/files/PG_ImageFiles.h>
#include <pg/files/PG_TX2.h>

#include <pg/files/PG_MPP.h>
#include <pg/files/PG_PSPFS.h>



TreeModel::TreeModel(QObject *parent)
    :QAbstractItemModel(parent), m_fileExtractor(nullptr){

}

TreeModel::TreeModel(const QString &data, QObject *parent)
    :QAbstractItemModel(parent), m_fileExtractor(nullptr){

	m_fileExtractor = nullptr;
	open(data);

}


bool TreeModel::open(const QString &file){
	if(m_fileExtractor) delete m_fileExtractor;

	QFileInfo fileInfo(file);
	const QString ext = fileInfo.suffix().toUpper();
	if(ext == "DAT"){
		m_fileExtractor = new PG::FILE::PSPFS();
	}else if(ext == "MPP"){
		m_fileExtractor = new PG::FILE::MPP();
	}else{
		qInfo() << "Unknown file format: '"<<ext<<"' file: '"<< file<<"'!";
		return false;
	}

	if(m_fileExtractor->open(file.toStdString())){
		qInfo() << "Couldn't open: '"<<file<<"'!";
		delete m_fileExtractor;
		m_fileExtractor = nullptr;
		return false;
	}

	return true;

}

bool TreeModel::openFile(const QString &file){
	QAbstractItemModel::layoutAboutToBeChanged();
	const bool result = open(file);
	QAbstractItemModel::layoutChanged();
	return result;

}

void TreeModel::extractFile(const QString &file, const QString &dir) const{
	if(!m_fileExtractor) return;

	const std::string targetFile = dir.toStdString() +"/"+file.toStdString();
	if(m_fileExtractor->extract(file.toStdString(), targetFile)){
		qInfo() << "Couldn't extract "<<file;
	}
}

bool TreeModel::addFile(const QString &file){
	if(!m_fileExtractor) return false;

	bool added = false;
	QAbstractItemModel::layoutAboutToBeChanged();
	if(m_fileExtractor->insert(file.toStdString())){
		qInfo() << "Couldn't add file '"<<file<<"'";
	}else
		added = true;
	QAbstractItemModel::layoutChanged();
	return added;
}

int TreeModel::addFiles(const QStringList &files){
	if(!m_fileExtractor || files.empty()) return 0;
	QAbstractItemModel::layoutAboutToBeChanged();
	int filesAddedOrChanged = 0;
	 for(const QString& str: files){
		 if(m_fileExtractor->insert(str.toStdString())){
		 		qInfo() << "Couldn't add file: '"<<str<<"'";
		 }else
			 filesAddedOrChanged++;
	 }
	QAbstractItemModel::layoutChanged();
	return filesAddedOrChanged;
}

int TreeModel::removeFiles(const QStringList &files){
	if(!m_fileExtractor || files.empty()) return 0;
	QAbstractItemModel::layoutAboutToBeChanged();
	int filesRemoved = 0;
	for(const QString& str: files){
		 if(m_fileExtractor->remove(str.toStdString())){
		 		qInfo() << "Couldn't remove file: '"<<str<<"'";
		 }else
			 filesRemoved++;
	}
	QAbstractItemModel::layoutChanged();
	return filesRemoved;
}

bool TreeModel::hasDataChanged() const{
	return m_fileExtractor->isChanged();
}

bool TreeModel::save(){
	if(!m_fileExtractor) return false;
	if(!m_fileExtractor->save())
		return true;
	else{
		//if saving faled try to reopen it
		QAbstractItemModel::layoutAboutToBeChanged();
		PG::UTIL::File file = m_fileExtractor->getOpendFile();
		return open(QString::fromStdString(file.getPath()));
		QAbstractItemModel::layoutChanged();
	}
}

bool TreeModel::getImage(const QString &file, PG::UTIL::RGBAImage& imageOut, bool alpha) const{
	if(!m_fileExtractor) return false;

	QFileInfo info(file);
	if(info.suffix() != "TX2"){
		qInfo() << "File is not a TX2: '"<<file<<"'";
		return true;
	}

	char * c = nullptr;
	unsigned int sile_size = 0;
	if( (sile_size = m_fileExtractor->extract(file.toStdString(), c)) == 0){
		qInfo() << "Couldn't extract image file: '"<<file<<"'";
		return true;
	}

	if(PG::FILE::decompressTX2(c, sile_size, imageOut)){
		qInfo() << "Couldn't decompress TX2 image file: '"<<file<<"'";
		if(c) delete[] c;
		return true;
	}
	if(c) delete[] c;

	if(imageOut.getWidth() == 0 || imageOut.getHeight() == 0){
		qInfo() << "Image has width or height is zero!";
		return true;
	}

	if(!alpha){
		for(PG::UTIL::rgba& rgba: imageOut){
			rgba.a = 255;
		}
	}

	return false;
}

bool TreeModel::setGraphicsScene(const QString &file, QGraphicsScene* scene) const{
	if(!m_fileExtractor) return false;

	PG::UTIL::RGBAImage img;
	if(getImage(file, img, false)){
		qInfo() << "Couldn't open image file: '"<<file<<"'";
		return true;
	}

	QImage qimg( &(img[0].r), img.getWidth() , img.getHeight(), QImage::Format_RGBA8888 );

	QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qimg));
	scene->clear();
	scene->addItem(item);

	return false;



}

/*
void TreeModel::hideFiles(const QString &extention, bool hide){
	if(!m_fileExtractor) return false;

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
}*/

QVariant TreeModel::data(const QModelIndex &index, int role) const{
    if (!m_fileExtractor || !index.isValid())
        return QVariant();


    const PG::FILE::fileInfo *item = static_cast<const PG::FILE::fileInfo*>(index.internalPointer());
    if(item == nullptr) return QVariant();

    QString exp = QString::fromStdString(item->getEileExtension());

    if(index.column() == 0 && role == Qt::DecorationRole){
    	if(exp == "PNG" || exp == "TX2")
    		return QPixmap("resources/image.png");
    	else if(exp == "OGG")
    		return QPixmap("resources/note.png");
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (index.column()) {
    		case 0:
    				return QVariant(QString::fromStdString(item->name.getPath()));
    			break;
    		case 1:
    				return QVariant(item->size);
    			break;
    		case 2:
    				return QVariant(exp);
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
    if (!m_fileExtractor || !hasIndex(row, column, parent))
        return QModelIndex();

    if(row >= m_fileExtractor->size())
    	return QModelIndex();

    PG::FILE::fileInfo* info = m_fileExtractor->getDataPointer(row);

    return createIndex(row, column, info); //QModelIndex( file );

}

QModelIndex TreeModel::parent(const QModelIndex &index) const{
    if (!index.isValid())
         return QModelIndex();

    return QModelIndex();

}

int TreeModel::rowCount(const QModelIndex &parent) const{
	if (!m_fileExtractor || parent.isValid())
		 return 0;

	return m_fileExtractor->size();
}

bool TreeModel::saveFile(){
    return save();
}

bool TreeModel::saveFileAs(const QString& filepath){
	if(!m_fileExtractor) return false;
    if(!m_fileExtractor->save(filepath.toStdString()))
        return true;
    else{
        //if saving faled try to reopen it
        QAbstractItemModel::layoutAboutToBeChanged();
        PG::UTIL::File file = m_fileExtractor->getOpendFile();
        return open(QString::fromStdString(file.getPath()));
        QAbstractItemModel::layoutChanged();
    }

    return false;
}

bool TreeModel::saveImage(const QString& imagename, const QString& targetfile){
	if(!m_fileExtractor) return false;
	PG::UTIL::RGBAImage img;
	if(getImage(imagename, img, true)){
		qInfo() << "Couldn't extract image file: '"<<imagename<<"'";
		return false;
	}

	qDebug() <<" Trying to export image '"<<imagename<<"' to '"<<targetfile<<"'.";

	QFileInfo fInfo(targetfile);
	QString ext = fInfo.suffix();
	if( ext == "tga"){
		return PG::FILE::saveTGA(targetfile.toStdString(), img);
	}else if(ext == "pgm"){
		return PG::FILE::savePGM(targetfile.toStdString(), img);
	}else{
		QImage qimg( &(img[0].r), img.getWidth() , img.getHeight(), QImage::Format_RGBA8888 );
		return qimg.save(targetfile, 0, 100);
	}


}

int TreeModel::columnCount(const QModelIndex &parent) const{
	if (parent.isValid())
		 return 0;
	return 3;

}

TreeModel::~TreeModel(){
	if(m_fileExtractor) delete m_fileExtractor;
}


