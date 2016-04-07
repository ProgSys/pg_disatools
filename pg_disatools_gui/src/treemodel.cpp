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
#include <pg/files/PG_StartDAT.h>
#include <pg/files/PG_SpriteSheetDAT.h>
#include <pg/files/PG_FileTests.h>
#include <pg/files/PG_IMY.h>


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
	m_fileExtractor = nullptr;
    for(QTemporaryFile* temp: m_tempFiles)
    	delete temp;
    m_tempFiles.clear();
	QAbstractItemModel::layoutAboutToBeChanged();

	QFileInfo fileInfo(file);
	const QString ext = fileInfo.suffix().toUpper();
	if(ext == "DAT"){
		m_openedFileType = "DAT";
		if(PG::FILE::isPSPFS(file.toStdString())){
			m_fileExtractor = new PG::FILE::PSPFS();
		}else if(PG::FILE::isSpriteSheetPackage(file.toStdString())){
			m_fileExtractor = new PG::FILE::SpriteSheetDAT();
		}else{
			m_fileExtractor = new PG::FILE::StartDAT();
		}

	}else if(ext == "MPP"){
		m_openedFileType = "MPP";
		m_fileExtractor = new PG::FILE::MPP();
	}else{
		qInfo() << "Unknown file format: '"<<ext<<"' file: '"<< file<<"'!";
		return false;
	}

	if(!m_fileExtractor) return false;

	if(m_fileExtractor->open(file.toStdString())){
		qInfo() << "Couldn't open: '"<<file<<"'!";
		delete m_fileExtractor;
		m_fileExtractor = nullptr;
		QAbstractItemModel::layoutChanged();
		return false;
	}
	QAbstractItemModel::layoutChanged();

	return true;
}

bool TreeModel::extractFileName(const QModelIndex& index, const QString &filepath) const{
	if(!m_fileExtractor || !index.isValid()) return true;
	PG::FILE::fileInfo *item = static_cast<PG::FILE::fileInfo*>(index.internalPointer());

	if(m_fileExtractor->extract(*item, filepath.toStdString())){
		qInfo() << "Couldn't extract "<<QString::fromStdString(item->name.getPath());
		return true;
	}
	return false;

}

bool TreeModel::extractFile(const QModelIndex& index, const QString &dir) const{
	if(!m_fileExtractor || !index.isValid()) return true;

	PG::FILE::fileInfo *item = static_cast<PG::FILE::fileInfo*>(index.internalPointer());
	const std::string targetFile = dir.toStdString() +"/"+item->name.getPath();
	if(m_fileExtractor->extract(*item, targetFile)){
		qInfo() << "Couldn't extract "<<QString::fromStdString(item->name.getPath());
		return true;
	}
	return false;
}

int TreeModel::extract(const QModelIndexList& indeces, const QString &dir) const{

	int extracted = 0;
	for(const QModelIndex& index: indeces){
		PG::FILE::fileInfo *item = static_cast<PG::FILE::fileInfo*>(index.internalPointer());
		const std::string targetFile = dir.toStdString() +"/"+item->name.getPath();
		if(m_fileExtractor->extract(*item, targetFile)){
			qInfo() << "Couldn't extract "<<QString::fromStdString(item->name.getPath());
		}else
			extracted++;

		 m_percentIndicator.percent = (extracted/float(indeces.size()))*100;
	}

	return extracted;
}

bool TreeModel::addFile(const QString &file){
	if(!m_fileExtractor) return false;

	bool added = false;
	QAbstractItemModel::layoutAboutToBeChanged();
	if(m_fileExtractor->insert(file.toStdString())){
		qInfo() << "Couldn't add file '"<<file<<"'";
	}else
		added = true;
	qInfo() << "Added file '"<<file<<"'";
	QAbstractItemModel::layoutChanged();
	return added;
}

int TreeModel::add(const QStringList &files){
	if(!m_fileExtractor || files.empty()) return 0;
	QAbstractItemModel::layoutAboutToBeChanged();

	int filesAddedOrChanged = 0;
	 for(const QString& str: files){
		 if(m_fileExtractor->insert(str.toStdString())){
		 		qInfo() << "Couldn't add file: '"<<str<<"'";
		 }else
			 filesAddedOrChanged++;
		 m_percentIndicator.percent = (filesAddedOrChanged/float(files.size()))*100;
	 }
	qInfo() << "Added files";
	QAbstractItemModel::layoutChanged();
	return filesAddedOrChanged;
}

bool TreeModel::replace(const QModelIndex& index, const QString &file, bool keepName){
	if(!m_fileExtractor || file.isEmpty() || !index.isValid()) return false;

	PG::FILE::fileInfo *item = static_cast<PG::FILE::fileInfo*>(index.internalPointer());
	if(m_fileExtractor->replace(*item, file.toStdString(), keepName)){
		qInfo() << "Couldn't remove file: '"<<QString::fromStdString(item->name.getPath())<<"'";
		return true;
	}
	return false;
}

bool TreeModel::remove(const QModelIndex& index){
	if(!m_fileExtractor || !index.isValid()) return true;

	QAbstractItemModel::layoutAboutToBeChanged();
	PG::FILE::fileInfo *item = static_cast<PG::FILE::fileInfo*>(index.internalPointer());
	if(m_fileExtractor->remove(*item)){
		qInfo() << "Couldn't remove file: '"<<QString::fromStdString(item->name.getPath())<<"'";
		return true;
	}
	QAbstractItemModel::layoutChanged();

	return false;
}

unsigned int TreeModel::remove(QList<PG::FILE::fileInfo*>& indices ){
	if(!m_fileExtractor) return true;
	QAbstractItemModel::layoutAboutToBeChanged();
	qSort(indices.begin(), indices.end(), [](const PG::FILE::fileInfo* a,const PG::FILE::fileInfo* b){
		return a > b;
	});

	unsigned int removed = 0;
	for(PG::FILE::fileInfo* index: indices){
		if(!m_fileExtractor->remove(*index))
			removed++;
		m_percentIndicator.percent = (removed/float(indices.size()))*100;
	}

	QAbstractItemModel::layoutChanged();
	return removed;
}

bool TreeModel::decompresIMYPack(const QModelIndex& index){
	if(!m_fileExtractor || !index.isValid()) return false;
	const PG::FILE::fileInfo *item = static_cast<const PG::FILE::fileInfo*>(index.internalPointer());
	if(!item || !item->isCompressed() || !item->isPackage() ){
		qInfo() << "File isn't a IMY pack '"<<QString::fromStdString(item->name.getPath())<<"'";
		return false;
	}

	QTemporaryFile* temp = new QTemporaryFile(QString::fromStdString(item->name.getPath()) , this);
	if(!temp->open()){
		delete temp;
		qInfo() << "Couldn't create temp file for IMY pack '"<<QString::fromStdString(item->name.getPath())<<"'";
		return false;
	}
	temp->close();
	m_tempFiles.push_back(temp);

	char * c = nullptr;
	unsigned int sile_size = 0;
	if( (sile_size = m_fileExtractor->extract(*item, c)) == 0){
		if(c) delete c;
		qInfo() << "Couldn't extract IMY pack: '"<<QString::fromStdString(item->name.getPath())<<"'";
		return false;
	}

	const std::string fileName =  temp->fileName().toStdString();
	PG::FILE::decompressIMYPackage(c,sile_size, fileName, &m_percentIndicator);

	return replace(index, temp->fileName(), true);
}

bool TreeModel::decompresIMYPack(const QModelIndex& index, const QString &filepath) const{
	if(!m_fileExtractor || !index.isValid() || filepath.isEmpty()) return false;
	const PG::FILE::fileInfo *item = static_cast<const PG::FILE::fileInfo*>(index.internalPointer());
	if(!item || !item->isCompressed() || !item->isPackage() ){
		qInfo() << "File isn't a IMY pack '"<<QString::fromStdString(item->name.getPath())<<"'";
		return false;
	}

	char * c = nullptr;
	unsigned int sile_size = 0;
	if( (sile_size = m_fileExtractor->extract(*item, c)) == 0){
		if(c) delete c;
		qInfo() << "Couldn't extract IMY pack: '"<<QString::fromStdString(item->name.getPath())<<"'";
		return false;
	}

	PG::FILE::decompressIMYPackage(c,sile_size, filepath.toStdString(), &m_percentIndicator);
	return true;
}


bool TreeModel::hasDataChanged() const{
	return m_fileExtractor->isChanged();
}

bool TreeModel::save(){
	if(!m_fileExtractor) return false;

	QAbstractItemModel::layoutAboutToBeChanged();
	PG::UTIL::File file = m_fileExtractor->getOpendFile();
	if(m_fileExtractor->save(&m_percentIndicator)){
		QAbstractItemModel::layoutChanged();
		return false;
	}
    for(QTemporaryFile* temp: m_tempFiles)
    	delete temp;
  		m_tempFiles.clear();
	//bool success = open(QString::fromStdString(file.getPath()));
	QAbstractItemModel::layoutChanged();
	return true;
}

bool TreeModel::saveAs(const QString& filepath){
	if(!m_fileExtractor) return false;
	 QAbstractItemModel::layoutAboutToBeChanged();
    if(!m_fileExtractor->save(filepath.toStdString(), &m_percentIndicator)){
    	m_fileExtractor->open(filepath.toStdString(), &m_percentIndicator);
    	QAbstractItemModel::layoutChanged();
        return true;
    }else{
        //if saving faled try to reopen it
        PG::UTIL::File file = m_fileExtractor->getOpendFile();
        const bool success = open(QString::fromStdString(file.getPath()));
        QAbstractItemModel::layoutChanged();
        return success;
    }

    return false;
}

bool TreeModel::getImage(const QModelIndex& index, PG::UTIL::RGBAImage& imageOut, bool alpha) const{
	if(!m_fileExtractor) return false;

	const PG::FILE::fileInfo *item = static_cast<const PG::FILE::fileInfo*>(index.internalPointer());

	if(item->name.getFileExtension() != "TX2"){
		qInfo() << "File is not a TX2: '"<<QString::fromStdString(item->name.getPath())<<"'";
		return true;
	}

	char * c = nullptr;
	unsigned int sile_size = 0;
	if( (sile_size = m_fileExtractor->extract(*item, c)) == 0){
		if(c) delete c;
		qInfo() << "Couldn't extract image file: '"<<QString::fromStdString(item->name.getPath())<<"'";
		return true;
	}

	if(PG::FILE::decompressTX2(c, sile_size, imageOut)){
		qInfo() << "Couldn't decompress TX2 image file: '"<<QString::fromStdString(item->name.getPath())<<"'";
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

bool TreeModel::setGraphicsScene(const QModelIndex& index, QGraphicsScene* scene) const{
	if(!m_fileExtractor) return false;

	PG::UTIL::RGBAImage img;
	if(getImage(index, img, false)){
		qInfo() << "Couldn't open image file!";
		return true;
	}

	QImage qimg( &(img[0].r), img.getWidth() , img.getHeight(), QImage::Format_RGBA8888 );

	QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qimg));
	scene->clear();
	scene->addItem(item);

	return false;



}

QVariant TreeModel::data(const QModelIndex &index, int role) const{
    if (!m_fileExtractor || !index.isValid())
        return QVariant();


    const PG::FILE::fileInfo *item = static_cast<const PG::FILE::fileInfo*>(index.internalPointer());
    if(item == nullptr) return QVariant();

    QString exp = QString::fromStdString(item->getFileExtension());

    if(role == Qt::DecorationRole && index.column() == 0){
    		if(item->isCompressed())
    			if(item->isPackage())
    				return QPixmap("resources/compress.png");
    			else
    				return QPixmap("resources/compress_file.png");
    		else if(item->isPackage())
    			return QPixmap("resources/archive.png");
    		else if(exp == "PNG" || exp == "TX2")
				return QPixmap("resources/image.png");
			else if(exp == "OGG")
				return QPixmap("resources/note.png");
			else if(exp == "GEO" || exp == "MPP")
				return QPixmap("resources/geometry.png");
			else if(item->fileType == PG::FILE::fileInfo::SH)
				return QPixmap("resources/sprite.png");
			return QVariant();
    }else if(role == Qt::DisplayRole){
		switch (index.column()) {
				case 0:
				{
					if(item->isExternalFile())
						return QVariant(QString::fromStdString(item->name.getPath()).append('*'));
					else
						return QVariant(QString::fromStdString(item->name.getPath()));
				}
					break;
				case 1:
						return QVariant(item->size);
					break;
				case 2:
						return QVariant(exp);
					break;
				case 3:
						return QString("%1").arg(index.row(), 5, 10, QChar('0')); //QVariant(QString::number(item->index));
					break;
				default:
					return QVariant();
					break;
			}
    }else
    	return QVariant();
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
        case 3:
            return QVariant(QString("Index"));
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

bool TreeModel::saveImage(const QModelIndex& index, const QString& targetfile){
	if(!m_fileExtractor) return false;
	PG::UTIL::RGBAImage img;



	if(getImage(index, img, true)){
		qInfo() << "Couldn't extract image file.";
		return false;
	}

	qInfo() <<" Trying to export image to '"<<targetfile<<"'.";

	QFileInfo fInfo(targetfile);
	QString ext = fInfo.suffix();
	if( ext == "tga"){
		return PG::FILE::saveTGA(targetfile.toStdString(), img);
	}else if(ext == "pgm" || ext == "pnm"){
		return PG::FILE::saveNetPNM(targetfile.toStdString(), img);
	}else{
		QImage qimg( &(img[0].r), img.getWidth() , img.getHeight(), QImage::Format_RGBA8888 );
		return qimg.save(targetfile, 0, 100);
	}


}

bool TreeModel::checkIsValid(QString& outMessage) const{
	if(m_fileExtractor){
		std::string errorMSG;
		if(!m_fileExtractor->checkValid(errorMSG)){
			outMessage = QString::fromStdString(errorMSG);
			return false;
		}
		return true;
	}else{
		outMessage = "There is no file opened!";
		return false;
	}
}

QString TreeModel::getOpenedFileName() const{
	if(m_fileExtractor){
		return QString::fromStdString(m_fileExtractor->getOpendFile().getName());
	}else{
		return "";
	}
}

const QString& TreeModel::getOpenedType() const{
	return m_openedFileType;
}

int TreeModel::columnCount(const QModelIndex &parent) const{
	if (parent.isValid())
		 return 0;
	return 4;

}

float TreeModel::getProgress() const{
	return m_percentIndicator.percent;
}

void TreeModel::getFileProperties(PG::FILE::fileProperties& target) const{
	if(m_fileExtractor) m_fileExtractor->getFileProperties(target);
}

TreeModel::~TreeModel(){
	if(m_fileExtractor) delete m_fileExtractor;
    for(QTemporaryFile* temp: m_tempFiles)
    	delete temp;
}


