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
#include <files/CharDAT.h>
#include <QDebug>
#include <QFileInfo>
#include <QIntValidator>
#include <QTextCodec>
#include <QDataStream>
#include <util/ShiftJISEncoder.h>

CharDAT::CharDAT(QObject *parent): DataFile(parent) {
	QList<QVariant> rootData;
	rootData << "Name" << "Title" << "Type"<< "Unknown" << "Unknown01"<<"Gender"<<"Unknown02"<<"Palette index"<<"Learn speed"<<"Jump power"
			<<"Fi"<<"Sw"<<"Sp"<<"Bo"<<"Gu"<<"Ax"<<"St" <<"Unknown13"
			<<"HP"<<"SP"<<"Atk"<<"Def"<<"Int"<<"Spd"<<"Hit"<<"Res"
			<< "Immunity"<<"Script ID"<<"Place"<<"Unknown03"<<"Unknown04"<<"Throwing range"<<"Help ID"
			<<"Base HP"<<"Base SP"<<"Base Atk"<<"Base Def"<<"Base Int"<<"Base Spd"<<"Base Hit"<<"Base Res"
			<<"Unknown05"<<"Movement"<<"Movement Type"<<"Unknown06"<<"Unknown07"<<"Unknown08"<<"Unknown09"<<"Unknown010"<<"Unknown11"<<"Unknown12"<<"Counter";
	for(int i = 0; i < 20; i++) rootData<< ("Specials ID "+QString::number(i+1));
	for(int i = 0; i < 20; i++) rootData<< ("Specials level "+QString::number(i+1));

	m_root = new TreeItem(rootData);
}


bool CharDAT::open(const QString& filepath){
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
	qDebug()<<"Found "<<size<<" characters!";
	QTextCodec * strcodec = QTextCodec::codecForName("Shift-JIS");
	QTextDecoder *decoder = strcodec->makeDecoder();


	beginInsertRows(QModelIndex(),0,size);
	for(unsigned int i = 0; i < size; i++){
		char* shift_JIS_String = new char[32];
		in.readRawData(shift_JIS_String, 32);
		int find_zero = 0;
		for(; find_zero < 34; find_zero++)
			if(shift_JIS_String[find_zero] == 0) break;
		if(find_zero > 32) find_zero = 32;
		QString characterName = decoder->toUnicode(shift_JIS_String, find_zero);
		delete shift_JIS_String;
		in.skipRawData(1);

		shift_JIS_String = new char[26];
		in.readRawData(shift_JIS_String, 26);
		find_zero = 0;
		for(; find_zero < 26; find_zero++)
			if(shift_JIS_String[find_zero] == 0) break;
		if(find_zero > 26) find_zero = 26;
		QString title = decoder->toUnicode(shift_JIS_String, find_zero);
		delete shift_JIS_String;
		in.skipRawData(1);

		quint16 immunity, scriptID, place, unknown03,unknown04, trowingRange,helpID,
			baseHP,baseSP,baseATK,baseDef,baseInt,baseSpd,baseHit,baseRes,
			unknown05,movement,movementType, unknown06,unknown07,unknown08,unknown09,unknown10,unknown11,unknown12, counter;
		quint8 type,unknown, unknown01,unknown13, gender, unknown02, palett, learnSpeed,jumpPower,
			Fi, Sw,Sp,Bo,Gu,Ax,St,
			HP,SP,Atk,Def,Int,Spd,Hit,Res;

		in >> type >> unknown>>unknown01 >> gender >> unknown02 >>  palett>>  learnSpeed>> jumpPower>>
				Fi>> Sw>> Sp>> Bo>> Gu>> Ax>> St>>unknown13
				>> HP>> SP>> Atk>> Def>> Int>> Spd>> Hit>> Res;

		in >> immunity >> scriptID>>  place>>  unknown03>> unknown04>>  trowingRange>> helpID>>
		baseHP>> baseSP>> baseATK>> baseDef>> baseInt>> baseSpd>> baseHit>> baseRes>>
		unknown05>> movement>> movementType>>  unknown06>> unknown07>> unknown08>> unknown09>> unknown10>> unknown11>> unknown12
		>>  counter;

		QList<QVariant> testData;
		testData<< characterName<< title;

		testData<< type << unknown << unknown01 <<gender <<unknown02 <<palett<<learnSpeed<< jumpPower<<
				Fi<<Sw<<Sp<<Bo<< Gu<<Ax<<St<<unknown13<<
				HP<<SP<<Atk<<Def<<Int<<Spd<<Hit<<Res;

		testData << immunity<< scriptID<< place<< unknown03<<unknown04<< trowingRange<<helpID<<
		baseHP<<baseSP<<baseATK<<baseDef<<baseInt<<baseSpd<<baseHit<<baseRes<<
		unknown05<<movement<<movementType<< unknown06<<unknown07<<unknown08<<unknown09<<unknown10<<unknown11<<unknown12
		<< counter;

		unsigned short* shorts = new unsigned short[20];
		in.readRawData((char*)shorts, 40);
		for(unsigned int i = 0; i < 20; i++)
			testData<<shorts[i];
		in.readRawData((char*)shorts, 40);
		for(unsigned int i = 0; i < 20; i++)
			testData<<shorts[i];
		delete shorts;

		m_root->appendChild(new TreeItem(testData,m_root));
	}
	delete decoder;
	endInsertRows();
	qfile.close();
	return true;
}



bool CharDAT::save(const QString& filepath){
	if(filepath.isEmpty()) return false;

	QFile qfile(filepath);
	if(!qfile.open(QIODevice::WriteOnly)) return false;
	QDataStream out(&qfile);
	out.setByteOrder(QDataStream::LittleEndian);

	out << (quint32) m_root->childCount();
	out << (quint32) m_root->childCount();

	for(const TreeItem* child: m_root->getChilderen()){
		QString name = child->data(0).toString();
		if(name.size() > 16) name = name.left(16);
		QByteArray bites = encodeToShiftJIS(name);
		out.writeRawData(bites.constData(), bites.size());
		for(unsigned int i = bites.size(); i < 33; i++)
			out<<(quint8) 0;

		bites.clear();
		QString title = child->data(1).toString();
		if(title.size() > 13) title = title.left(13);
		bites = encodeToShiftJIS(title);
		out.writeRawData(bites.constData(), bites.size());
		for(unsigned int i = bites.size(); i < 27; i++)
			out<<(quint8) 0;

		for(unsigned int i = 2; i < 2+24; i++)
			out<<(quint8) child->data(i).toInt();
		for(unsigned int i = 26; i < child->columnCount(); i++)
			out<<(quint16) child->data(i).toInt();
	}

	qfile.close();
	return true;
}


bool CharDAT::setData(const QModelIndex &index, const QVariant &value, int role){
	if(!index.isValid() || role != Qt::EditRole) return false;

	TreeItem *parentItem = static_cast<TreeItem*>(index.internalPointer());
	if(!parentItem) return false;

	if(index.column() == 0 ){
		if(value.type() != QVariant::String) return false;
		QString str = value.toString();
		if(str.isEmpty()) return false;
		if(str.size() > 16){
			if(!parentItem->setData(index.column(), str.left(16))) return false;
		}else
			if(!parentItem->setData(index.column(), str)) return false;
	}else if(index.column() == 1){
		if(value.type() != QVariant::String) return false;
		QString str = value.toString();
		if(str.isEmpty()) return false;
		if(str.size() > 13){
			if(!parentItem->setData(index.column(), str.left(13))) return false;
		}else
			if(!parentItem->setData(index.column(), str)) return false;
	}else if(index.column() < 26){
		QIntValidator validator(0, 255);
		QString val = value.toString();
		int pos = 0;
		if( validator.validate(val,pos) != QValidator::Acceptable)
			return false;

		if(!parentItem->setData(index.column(), value)) return false;
	}else{
		QIntValidator validator(0, 65534);
		QString val = value.toString();
		int pos = 0;
		if( validator.validate(val,pos) != QValidator::Acceptable)
			return false;

		if(!parentItem->setData(index.column(), value)) return false;
	}

	return true;
}

CharDAT::~CharDAT() {
	// TODO Auto-generated destructor stub
}

