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
#include <string>


DungeonDAT::DungeonDAT(QObject *parent): DataFile(parent) {
	QList<QVariant> rootData;
	rootData << "Map Name" << "Banus Rank" << "Map ID" << "Script ID";
	m_root = new TreeItem(rootData);

	/*
	QList<QVariant> testData;
	testData<< "Test MAP"<< 1<<101<<0;
	m_root->appendChild(new TreeItem(testData, m_root));
	testData.clear();
	testData<< "Test MAP2"<< 2<<103<<1;
	m_root->appendChild(new TreeItem(testData, m_root));
	//setupModelData(data.split(QString("\n")), m_root);
	 */
}


DungeonDAT::~DungeonDAT() {
	delete m_root;
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
		QString mapName = decoder->toUnicode(shift_JIS_String, find_zero);
		delete shift_JIS_String;
		quint16 bonusRank,mapID,scriptID;
		in >> bonusRank >> mapID >> scriptID;
		QList<QVariant> testData;
		testData<< mapName<< bonusRank<<mapID<<scriptID;
		m_root->appendChild(new TreeItem(testData, m_root));
	}
	delete decoder;
	endInsertRows();
	qfile.close();
	return true;
}


inline short encodeToShiftJIS(const QChar& c){
	//http://unicode-table.com/de/#arabic-presentation-forms-b
	//http://www.rtpro.yamaha.co.jp/RT/docs/misc/kanji-sjis.html
	switch (c.row() << 8 | c.cell()) {
		//numbers
		case ' ':
		case 0x3000:
			return 0x8140;
			break;
		case '0':
		case 0xFF10: // 0
			return 0x824f;
			break;
		case '1':
		case 0xFF11: // 1
			return 0x8250;
			break;
		case '2':
		case 0xFF12: // 2
			return 0x8251;
			break;
		case '3':
		case 0xFF13: // 3
			return 0x8252;
			break;
		case '4':
		case 0xFF14: // 4
			return 0x8253;
			break;
		case '5':
		case 0xFF15: // 5
			return 0x8254;
			break;
		case '6':
		case 0xFF16: // 6
			return 0x8255;
			break;
		case '7':
		case 0xFF17: // 7
			return 0x8256;
			break;
		case '8':
		case 0xFF18: // 8
			return 0x8257;
			break;
		case '9':
		case 0xFF19: // 9
			return 0x8258;
			break;
		//Capital letters
		case 'A':
		case 0xFF21: //A
			return 0x8260;
			break;
		case 'B':
		case 0xFF22: //B
			return 0x8261;
			break;
		case 'C':
		case 0xFF23: //C
			return 0x8262;
			break;
		case 'D':
		case 0xFF24: //D
			return 0x8263;
			break;
		case 'E':
		case 0xFF25: //E
			return 0x8264;
			break;
		case 'F':
		case 0xFF26: //F
			return 0x8265;
			break;
		case 'G':
		case 0xFF27: //G
			return 0x8266;
			break;
		case 'H':
		case 0xFF28: //H
			return 0x8267;
			break;
		case 'I':
		case 0xFF29: //I
			return 0x8268;
			break;
		case 'J':
		case 0xFF2a: //J
			return 0x8269;
			break;
		case 'K':
		case 0xFF2b:
			return 0x826a;
			break;
		case 'L':
		case 0xFF2c:
			return 0x826b;
			break;
		case 'M':
		case 0xFF2d:
			return 0x826c;
			break;
		case 'N':
		case 0xFF2e:
			return 0x826d;
			break;
		case 'O':
		case 0xFF2f:
			return 0x826e;
			break;
		case 'P':
		case 0xFF30:
			return 0x826f;
			break;
		case 'Q':
		case 0xFF31:
			return 0x8270;
			break;
		case 'R':
		case 0xFF32:
			return 0x8271;
			break;
		case 'S':
		case 0xFF33:
			return 0x8272;
			break;
		case 'T':
		case 0xFF34:
			return 0x8273;
			break;
		case 'U':
		case 0xFF35:
			return 0x8274;
			break;
		case 'V':
		case 0xFF36:
			return 0x8275;
			break;
		case 'W':
		case 0xFF37:
			return 0x8276;
			break;
		case 'X':
		case 0xFF38:
			return 0x8277;
			break;
		case 'Y':
		case 0xFF39:
			return 0x8278;
			break;
		case 'Z':
		case 0xFF3a:
			return 0x8279;
			break;

		//small letters
		case 'a':
		case 0xFF41:
			return 0x8281;
			break;
		case 'b':
		case 0xFF42:
			return 0x8282;
			break;
		case 'c':
		case 0xFF43:
			return 0x8283;
			break;
		case 'd':
		case 0xFF44:
			return 0x8284;
			break;
		case 'e':
		case 0xFF45:
			return 0x8285;
			break;
		case 'f':
		case 0xFF46:
			return 0x8286;
			break;
		case 'g':
		case 0xFF47:
			return 0x8287;
			break;
		case 'h':
		case 0xFF48:
			return 0x8288;
			break;
		case 'i':
		case 0xFF49:
			return 0x8289;
			break;
		case 'j':
		case 0xFF4a:
			return 0x828a;
			break;
		case 'k':
		case 0xFF4b:
			return 0x828b;
			break;
		case 'l':
		case 0xFF4c:
			return 0x828c;
			break;
		case 'm':
		case 0xFF4d:
			return 0x828d;
			break;
		case 'n':
		case 0xFF4e:
			return 0x828e;
			break;
		case 'o':
		case 0xFF4f:
			return 0x828f;
			break;
		case 'p':
		case 0xFF50:
			return 0x8290;
			break;
		case 'q':
		case 0xFF51:
			return 0x8291;
			break;
		case 'r':
		case 0xFF52:
			return 0x8292;
			break;
		case 's':
		case 0xFF53:
			return 0x8293;
			break;
		case 't':
		case 0xFF54:
			return 0x8294;
			break;
		case 'u':
		case 0xFF55:
			return 0x8295;
			break;
		case 'v':
		case 0xFF56:
			return 0x8296;
			break;
		case 'w':
		case 0xFF57:
			return 0x8297;
			break;
		case 'x':
		case 0xFF58:
			return 0x8298;
			break;
		case 'y':
		case 0xFF59:
			return 0x8299;
			break;
		case 'z':
		case 0xFF5a:
			return 0x829a;
			break;
		//special
		case '°':
			return 0x8142;
			break;
		case ',':
			return 0x8143;
			break;
		case '.':
		case 0xFF0E:  //Fullwidth Full Stop
			return 0x8144;
			break;
		case ':':
		case  0xFF1A:
			return 0x8146;
			break;
		case '?':
		case  0xFF1F:
			return 0x8147;
			break;
		case '!':
		case 0xFF01:
			return 0x8148;
			break;
		case '"':
		case 0xFF02:
			return 0x8149;
			break;
		case '(':
		case  0xFF08: //Fullwidth Left Parenthesis
			return 0x8169;
			break;
		case ')':
		case  0xFF09: //Fullwidth Left Parenthesis
			return 0x816a;
			break;
		case '{':
		case 0xFF5B:
			return 0x816f;
			break;
		case '}':
		case 0xFF5D:
			return 0x8170;
			break;

		case 0x2018:  //Left single quotation mark
			return 0x8165;
			break;
		case 0x2019:  //Right single quotation mark
			return 0x8166;
			break;
		case '-':
		case 0xFF0D:  //Fullwidth Hyphen-Minus
			return 0x817c;
			break;
		case '@':
		case 0xFF97:
			return 0x816f;
			break;

		default:
		{
			qDebug() <<"Unknown ShiftJIS conversion of: '"<<c<<"' ("<<(c.row() << 8 | c.cell())<<")";
			return 0;
		}
			break;
	}
}

inline QByteArray encodeToShiftJIS(const QString& str){
	QByteArray bytes;
	for(const QChar& c: str){
		short s = encodeToShiftJIS(c);
		s = ((s >> 8) & 0x00FF) | (s << 8) ;
		bytes.append((char*) &s, 2);
	}
	return bytes;
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
		out << (quint16) child->data(1).toInt() << (quint16) child->data(2).toInt() << (quint16) child->data(3).toInt();
	}

	//delete encoder;

	for(unsigned int i = 0; i < 16; i++)
		out<<(quint8) 0;
	qfile.close();
	return true;
}


QVariant DungeonDAT::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    return item->data(index.column());

}

Qt::ItemFlags DungeonDAT::flags(const QModelIndex &index) const{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant DungeonDAT::headerData(int section, Qt::Orientation orientation,
                    int role) const{

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_root->data(section);

    return QVariant();

}

QModelIndex DungeonDAT::index(int row, int column,
                  const QModelIndex &parent) const{

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_root;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex DungeonDAT::parent(const QModelIndex &index) const{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parentItem();

    if (parentItem == m_root)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);

}

int DungeonDAT::rowCount(const QModelIndex &parent) const{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_root;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int DungeonDAT::columnCount(const QModelIndex &parent) const{
	   if (parent.isValid())
	        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	    else
	        return m_root->columnCount();
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
