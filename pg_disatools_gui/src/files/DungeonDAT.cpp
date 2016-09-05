/*
 * DungeonDAT.cpp
 *
 *  Created on: 04.09.2016
 *      Author: ProgSys
 */

#include <files/DungeonDAT.h>
#include <QDebug>
#include <QFileInfo>
#include <QIntValidator>
#include <QTextCodec>
#include <QDataStream>
#include <util/ShiftJISEncoder.h>


DungeonDAT::DungeonDAT(QObject *parent): DataFile(parent) {
	QList<QVariant> rootData;
	rootData << "Map Name" << "Banus Rank" << "Map ID" << "Script ID";
	m_root = new TreeItem(rootData);
}


DungeonDAT::~DungeonDAT() {

}


bool DungeonDAT::open(const QString& filepath){
	if(filepath.isEmpty()) return false;

	QFile qfile(filepath);
	if(!qfile.open(QIODevice::ReadOnly)) return false;

	beginRemoveRows(QModelIndex(),0,m_root->childCount());
	m_root->clear();
	endRemoveRows();

	QDataStream in(&qfile);
	in.setByteOrder(QDataStream::LittleEndian);
	quint32 size;
	in >> size;
	quint32 sizeAgain;
	in >> sizeAgain;
	if(size > 9000){
		qDebug()<<"Size is too big! ("<<size<<")";
		return false;
	}
	if(size != sizeAgain){
		qDebug()<<"Both sizes should be the same! ("<<size<<" != "<<sizeAgain<<")";
		return false;
	}
	qDebug()<<"Found "<<size<<" maps!";
	QTextCodec * strcodec = QTextCodec::codecForName("Shift-JIS");
	QTextDecoder *decoder = strcodec->makeDecoder();


	beginInsertRows(QModelIndex(),0,size);
	for(unsigned int i = 0; i < size; i++){
		char* shift_JIS_String = new char[34];
		in.readRawData(shift_JIS_String, 34);
		int find_zero = 0;
		for(; find_zero < 34; find_zero++)
			if(shift_JIS_String[find_zero] == 0) break;
		if(find_zero > 34) find_zero = 34;
		QString mapName = decoder->toUnicode(shift_JIS_String, find_zero);
		delete shift_JIS_String;
		quint16 bonusRank,mapID,scriptID;
		in.setByteOrder(QDataStream::BigEndian);
		in >> bonusRank;
		in.setByteOrder(QDataStream::LittleEndian);
		in >> mapID >> scriptID;
		QList<QVariant> testData;
		testData<< mapName<< bonusRank<<mapID<<scriptID;
		m_root->appendChild(new TreeItem(testData, m_root));
	}
	delete decoder;
	endInsertRows();
	qfile.close();
	return true;
}



bool DungeonDAT::save(const QString& filepath){
	if(filepath.isEmpty()) return false;

	QFile qfile(filepath);
	if(!qfile.open(QIODevice::WriteOnly)) return false;
	QDataStream out(&qfile);
	out.setByteOrder(QDataStream::LittleEndian);

	out << (quint32) m_root->childCount();
	out << (quint32) m_root->childCount();

	//QTextCodec * strcodec = QTextCodec::codecForName("Shift-JIS");
	//QTextEncoder* encoder = strcodec->makeEncoder();

	for(const TreeItem* child: m_root->getChilderen()){
		QString mapName = child->data(0).toString();
		if(mapName.size() > 17) mapName = mapName.left(17);
		//out << mapName;
		//QByteArray bites = encoder->fromUnicode(mapName);
		//if(encoder->hasFailure())
			//qDebug()<<"Encoder failed on string '"<<mapName<<"'!";
		QByteArray bites = encodeToShiftJIS(mapName);// encoder->fromUnicode(mapName);
		out.writeRawData(bites.constData(), bites.size());
		for(unsigned int i = bites.size(); i < 34; i++)
			out<<(quint8) 0;
		out.setByteOrder(QDataStream::BigEndian);
		out << (quint16) child->data(1).toInt();
		out.setByteOrder(QDataStream::LittleEndian);
		out << (quint16) child->data(2).toInt() << (quint16) child->data(3).toInt();
	}

	//delete encoder;

	for(unsigned int i = 0; i < 16; i++)
		out<<(quint8) 0;
	qfile.close();
	return true;
}


bool DungeonDAT::setData(const QModelIndex &index, const QVariant &value, int role){
	if(!index.isValid() || role != Qt::EditRole) return false;
	qDebug()<<"Set Data: "<<value<<" role: "<<role;

	TreeItem *parentItem = static_cast<TreeItem*>(index.internalPointer());
	if(!parentItem) return false;

	if(index.column()){
		if(value.type() != QVariant::String) return false;
		QString val = value.toString();

		QIntValidator validator(0, 10000);
		int pos = 0;
		if( validator.validate(val,pos) != QValidator::Acceptable)
			return false;

		if(!parentItem->setData(index.column(), value)) return false;
	}else{
		if(value.type() != QVariant::String) return false;
		QString mapName = value.toString();
		if(mapName.isEmpty()) return false;
		if(mapName.size() > 17){
			if(!parentItem->setData(index.column(), mapName.left(17))) return false;
		}else
			if(!parentItem->setData(index.column(), mapName)) return false;
	}


	//if(!parentItem->setData(index.column(), value)) return false;

	emit dataChanged(index, index, {Qt::EditRole});
	return true;
}
