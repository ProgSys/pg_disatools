/*
 * HospitalDAT.cpp
 *
 *  Created on: 05.09.2016
 *      Author: ProgSys
 */

#include <files/HospitalDAT.h>
#include <QDataStream>
#include <QDebug>
#include <QFileInfo>
#include <QIntValidator>


HospitalDAT::HospitalDAT(QObject *parent): DataFile(parent) {
	QList<QVariant> rootData;
	rootData << "ID" << "Deceased" << "Item ID" << "HP" << "SP";
	m_root = new TreeItem(rootData);
}

HospitalDAT::~HospitalDAT() {
	// TODO Auto-generated destructor stub
}


bool HospitalDAT::open(const QString& filepath){
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
	qDebug()<<"Found "<<size<<" entrys!";



	beginInsertRows(QModelIndex(),0,size);
	for(unsigned int i = 0; i < size; i++){
		quint16 id,Deceased;
		quint32 itemID,HP,SP;
		in >> id >> Deceased>>itemID>>HP>>SP;

		QList<QVariant> testData;
		testData<< id<< Deceased<<itemID<<HP<<SP;
		m_root->appendChild(new TreeItem(testData, m_root));
	}
	endInsertRows();
	qfile.close();
	return true;
}

bool HospitalDAT::save(const QString& filepath){
	if(filepath.isEmpty()) return false;

	QFile qfile(filepath);
	if(!qfile.open(QIODevice::WriteOnly)) return false;
	QDataStream out(&qfile);
	out.setByteOrder(QDataStream::LittleEndian);

	out << (quint32) m_root->childCount();
	out << (quint32) m_root->childCount();


	for(const TreeItem* child: m_root->getChilderen()){
		out << (quint16) child->data(0).toInt() << (quint16) child->data(1).toInt()
		<< (quint32) child->data(2).toInt() << (quint32) child->data(3).toInt() << (quint32) child->data(4).toInt();
	}


	for(unsigned int i = 0; i < 16; i++)
		out<<(quint8) 0;
	qfile.close();
	return true;
}

Qt::ItemFlags HospitalDAT::flags(const QModelIndex &index) const{
    if (!m_root || !index.isValid())
        return 0;

    if(index.column())
    	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    else
    	return QAbstractItemModel::flags(index);
}

bool HospitalDAT::setData(const QModelIndex &index, const QVariant &value, int role){
	if(!index.isValid() || role != Qt::EditRole) return false;
	qDebug()<<"Set Data: "<<value<<" role: "<<role;

	TreeItem *parentItem = static_cast<TreeItem*>(index.internalPointer());
	if(!parentItem) return false;


	if(value.type() != QVariant::String) return false;
	QString val = value.toString();

	QIntValidator validator(0, 10000);
	if(index.column() > 2) validator.setRange(0, 2147483519);

	int pos = 0;
	if( validator.validate(val,pos) != QValidator::Acceptable)
		return false;

	if(!parentItem->setData(index.column(), value)) return false;

	emit dataChanged(index, index, {Qt::EditRole});
	return true;
}

bool HospitalDAT::insertFront(){
	if(!DataFile::insertFront()) return false;

	for(unsigned int i = 1; i < m_root->childCount(); i++)
		m_root->getChilderen()[i]->getData()[0] = m_root->getChilderen()[i]->getData()[0].toInt()+1;
	emit dataChanged(index(1,0), index(m_root->childCount(),0), {Qt::EditRole});

	return true;
}
bool HospitalDAT::insertBack(){
	if(!DataFile::insertBack()) return false;
	m_root->getChilderen()[m_root->childCount()-1]->getData()[0] = m_root->getChilderen()[m_root->childCount()-1]->getData()[0].toInt()+1;
	emit dataChanged(index(m_root->childCount(),0), index(m_root->childCount(),0), {Qt::EditRole});

	return true;
}

bool HospitalDAT::insertAt(int indexIn){
	if(!DataFile::insertAt(indexIn)) return false;

	for(unsigned int i = indexIn+1; i < m_root->childCount(); i++)
		m_root->getChilderen()[i]->getData()[0] = m_root->getChilderen()[i]->getData()[0].toInt()+1;
	emit dataChanged(index(indexIn+1,0), index(m_root->childCount(),0), {Qt::EditRole});

	return true;
}

bool HospitalDAT::removeAt(int indexIn){
	if(!DataFile::removeAt(indexIn)) return false;

	for(unsigned int i = indexIn; i < m_root->childCount(); i++)
		m_root->getChilderen()[i]->getData()[0] = m_root->getChilderen()[i]->getData()[0].toInt()-1;
	emit dataChanged(index(indexIn,0), index(m_root->childCount(),0), {Qt::EditRole});

	return true;
}
