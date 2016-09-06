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

#include <include/files/ParserDAT.h>
#include <QDataStream>
#include <QDebug>
#include <QFileInfo>
#include <QIntValidator>
#include <QTextStream>
#include <QMessageBox>
#include <QTextCodec>
#include <QByteArray>
#include <util/ShiftJISEncoder.h>


inline bool errorAt(const QString& info = ""){
	QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error",
					"Parsing error of definition file! \n" +info,
				 QMessageBox::Ok);
	return false;
}

struct defParther{
	QTextStream in;
	QChar currentChar = ' ';

	defParther(QFile& file): in(&file){

	}



	QChar getNextChar(){
		if(in.atEnd()) return '!';
		QChar c;
		in >> c;
		currentChar = c;
		return c;
	}

	void comment(){
		while(!in.atEnd()){
			if(currentChar == '\n') break;
			getNextChar();
		}
		getNextChar();
		skip();
	}

	void skip(const QString& skipChars = " \n\t\r"){
		if(!in.atEnd()){
			if(currentChar == '#'){
				comment();
				return;
			}
			for(const QChar& v: skipChars)
				if(currentChar == v){
					getNextChar();
					skip(skipChars);
				}
		}
	}

	bool compare(const QString& str){
		skip();
		int i = 0;
		while(i < str.size() && !in.atEnd()){
			if(currentChar.toLower() != str[i].toLower() ) return errorAt("Could not find '"+str+"'!");
			getNextChar();
			i++;
		}
		return true;
	}

	bool read(QString& out, const QString& endvalues = " \n\t\r,();{}"){
		skip();
		while(!in.atEnd()){
			for(const QChar& v: endvalues)
				if(currentChar == v) return true;
			out.push_back(currentChar);
			getNextChar();
		}
		return errorAt("Failed to read value!");
	}

	bool readStr(QString& out){
		skip();
		if( currentChar != '\"') return errorAt("A string needs to start with a '\"'!");
		getNextChar();

		while(!in.atEnd()){
			if(currentChar == '\"') {
				getNextChar();
				return true;
			}
			out.push_back(currentChar);
			getNextChar();
		}
		return errorAt("A string needs to end with a '\"'!");
	}

	bool readHeaderParamters(parse& data){
		skip();
		if( currentChar != '(') return errorAt("Parameters need to start with '('!");
		getNextChar();
		while(!in.atEnd()){
			QString str;
			if(!readStr(str)) return false;
			qDebug()<<" Header: "<<str;

			data.header.push_back(column(str));
			skip();
			if( currentChar == ')') {
				getNextChar();
				return true;
			}
			if(currentChar != ',') return errorAt("Parameters should be separated with a ',' or end with ')'!");
			getNextChar();
		}
		return errorAt("Parameters need to end with ')'!");
	}

	bool readSetColumnFlag(parse& data){
		skip();
		if( currentChar != '(') return errorAt("Parameters need to start with '('!");
		getNextChar();
		QString ColumnStr, flag;
		if(!readStr(ColumnStr)) return false;
		if(currentChar != ',') return errorAt("Parameters should be separated with a ',' or end with ')'!");
		getNextChar();
		if(!read(flag)) return false;
		skip();
		if( currentChar != ')') return errorAt("'SetColumnFlag' has 2 parameters!");
		getNextChar();

		flag = flag.toLower();
		for(column& col : data.header){
			if(col == ColumnStr){
				if(flag == "none"){
					col.flag = column::columnFLAGS::NONE;
				}else if(flag == "noedit"){
					col.flag = column::columnFLAGS::NOEDIT;
				}else{
					return errorAt("Unknown column flag '"+flag + "' is used!");
				}

				return true;
			}
		}
		return errorAt("Column with the name '"+ColumnStr + "' not found!");
	}

	bool readSetColumnType(parse& data){
		skip();
		if( currentChar != '(') return errorAt("Parameters need to start with '('!");
		getNextChar();
		QString ColumnStr, type;
		if(!readStr(ColumnStr)) return false;
		if(currentChar != ',') return errorAt("Parameters should be separated with a ',' or end with ')'!");
		getNextChar();
		if(!read(type)) return false;
		skip();
		if( currentChar != ')') return errorAt("'SetColumnType' has 2 parameters!");
		getNextChar();

		type = type.toLower();
		for(column& col : data.header){
			if(col == ColumnStr){
				if(type == "default"){
					col.type = column::columnType::DEFAULT;
				}else if(type == "index"){
					col.type = column::columnType::INDEX;
				}else{
					return errorAt("Unknown column type '"+type + "' is used!");
				}

				return true;
			}
		}
		return errorAt("Column with the name '"+ColumnStr + "' not found!");
	}

	bool readSingleParamter(QString& paramter){
		skip();
		if( currentChar != '(') return errorAt("(single) Parameters need to start with '('!");
		getNextChar();

		if(!read(paramter)) return false;
		skip();
		if( currentChar != ')') return errorAt("There should be only one parameter!");
		getNextChar();
		return true;
	}

	bool readParamters(QStringList& paramter){
		skip();
		if( currentChar != '(') return errorAt("(multiple) Parameters need to start with '('!");
		getNextChar();
		while(!in.atEnd()){
			skip();
			QString para;
			if(!read(para)) return false;
			paramter.push_back(para);
			skip();
			if( currentChar == ')') {
				getNextChar();
				return true;
			}
			if(currentChar != ',') return errorAt("Parameters should be separated with a ',' or end with ')'!");
			getNextChar();
		}


		if( currentChar != ')') return errorAt("(multiple) Parameters need to end with ')'!");
		getNextChar();
		return true;
	}

	bool readRowFormat(parse& data){
		skip();
		if( currentChar != '{') return errorAt("A RowFormat black need to start with '{'!");
		getNextChar();
		while(!in.atEnd()){
			skip();
			if(currentChar == ';') {
				getNextChar();
				continue;
			}else if(currentChar == '}'){
				return true;
			}

			QString command;
			if(!read(command)) return false;
			if(command.isEmpty()) return errorAt("Given command is empty!");

			QStringList paras;
			if(!readParamters(paras)) return false;
			if(paras.empty()) return errorAt("RowFormat has zero parameters!");

			bool ok;
			int bytes = paras[0].toInt(&ok);
			if(!ok) return errorAt("Given parameter for a row format is not a number!");

			int numberOf = 1;
			if(paras.size() > 1){
				numberOf = paras[1].toInt(&ok);
				if(!ok || numberOf < 1) return errorAt("RowFormat 'number of' is invalid!");
			}

			qDebug()<<" Command: "<<command<<" number of "<<numberOf;
			command = command.toLower();
			if(command == "uint"){
				for(int i = 0; i < numberOf; i++)
					data.formats.push_back(rowFormat(rowFormat::UINT, bytes));
			}else if(command == "int"){
				for(int i = 0; i < numberOf; i++)
					data.formats.push_back(rowFormat(rowFormat::INT, bytes));
			}else if(command == "shiftjis"){
				for(int i = 0; i < numberOf; i++)
					data.formats.push_back(rowFormat(rowFormat::SHIFT_JIS, bytes));
			}else if(command == "zero"){
				for(int i = 0; i < numberOf; i++)
					data.formats.push_back(rowFormat(rowFormat::ZERO, bytes));
			}else{
				return errorAt("Row format '"+command +"' is unknown!");
			}


		}
		return errorAt("A RowFormat block need to end with '}'!");
	}

	bool readTableBlock(parse& data){
		skip();
		if( currentChar != '{') return errorAt("A table block need to start with '{'!");
		getNextChar();
		while(!in.atEnd()){
			skip();
			if(currentChar == ';') {
				getNextChar();
				continue;
			}else if(currentChar == '}'){
				return true;
			}
			QString command;
			if(!read(command)) return false;
			qDebug()<<" Command: "<<command;
			command = command.toLower();
			if(command == "setcolumnflag"){
				if(!readSetColumnFlag(data)) return false;
			}else if(command == "setcolumntype"){
				if(!readSetColumnType(data)) return false;
			}else if(command == "rowformat"){
				qDebug() <<"Read RowFormat block";
				if(!readRowFormat(data)) return false;
			}else{
				return errorAt("Command '"+command+"' is unknown!");
			}

		}
		return errorAt("A table block need to end with '}'!");
	}

	bool parseDef(parse& data){
		qDebug() <<"Read Header";
		if(!compare("TABLE")) return false;
		qDebug() <<"Read Header paras";
		if(!readHeaderParamters(data)) return false;
		qDebug() <<"Read table block";
		if(!readTableBlock(data)) return false;

		return true;
	}
};

ParserDAT::ParserDAT(const QString& defFile, QObject *parent): DataFile(parent) {
	QFile qfile(defFile);
	if(!qfile.open(QIODevice::ReadOnly)) return;

	defParther p(qfile);
	if(p.parseDef(m_dataStructure)){
		qDebug() <<"Parsing SUCCESSFUL!";

		QList<QVariant> rootData;
		for(const column& col: m_dataStructure.header){
			rootData << col.name;
		}
		m_root = new TreeItem(rootData);


	}else{
		qDebug() <<"Parsing FAILED!";
	}

	qfile.close();



}

ParserDAT::~ParserDAT() {
	// TODO Auto-generated destructor stub
}

bool ParserDAT::open(const QString& filepath){
	if(!m_root || m_dataStructure.header.empty() || m_dataStructure.formats.empty() || filepath.isEmpty()) return false;

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
	qDebug()<<"Found "<<size<<" Entry's!";

	QTextCodec * strcodec = QTextCodec::codecForName("Shift-JIS");
	QTextDecoder *decoder = strcodec->makeDecoder();

	beginInsertRows(QModelIndex(),0,size);
	for(unsigned int i = 0; i < size; i++){
		QList<QVariant> testData;
		for(const rowFormat& format: m_dataStructure.formats){
			if(format.rowType == rowFormat::INT){
				switch (format.byteSize) {
					case 0:
					case 1:
					{
						qint8 v;
						in >> v;
						testData<< v;
					}
						break;
					case 2:
					case 16:
					{
						qint16 v;
						in >> v;
						testData<< v;
					}
						break;
					case 4:
					case 32:
					{
						qint32 v;
						in >> v;
						testData<< v;
					}
						break;
					case 8:
					case 64:
					{
						qint64 v;
						in >> v;
						testData<< v;
					}
						break;
					default:
					{
						qDebug() <<"(Int) Invalid byteSize: "<<format.byteSize;
						qint8 v;
						in >> v;
						testData<< v;
					}
						break;
				}
			}else if(format.rowType == rowFormat::UINT){
				switch (format.byteSize) {
					case 0:
					case 1:
					{
						quint8 v;
						in >> v;
						testData<< v;
					}
						break;
					case 2:
					case 16:
					{
						quint16 v;
						in >> v;
						testData<< v;
					}
						break;
					case 4:
					case 32:
					{
						quint32 v;
						in >> v;
						testData<< v;
					}
						break;
					case 8:
					case 64:
					{
						quint64 v;
						in >> v;
						testData<< v;
					}
						break;
					default:
					{
						qDebug() <<"(UInt) Invalid byteSize: "<<format.byteSize;
						quint8 v;
						in >> v;
						testData<< v;
					}
						break;
				}
			}else if(format.rowType == rowFormat::ZERO){
				if(format.byteSize > 0)
					in.skipRawData(format.byteSize);
			}else if(format.rowType == rowFormat::SHIFT_JIS){

				char* shift_JIS_String = new char[format.byteSize];
				in.readRawData(shift_JIS_String, format.byteSize);
				int find_zero = 0;
				for(; find_zero < format.byteSize; find_zero++)
					if(shift_JIS_String[find_zero] == 0) break;
				if(find_zero > format.byteSize) find_zero = format.byteSize;
				QString str = decoder->toUnicode(shift_JIS_String, find_zero);
				delete shift_JIS_String;
				testData<< str;
			}

		}
		m_root->appendChild(new TreeItem(testData, m_root));
	}
	delete decoder;
	endInsertRows();

	qfile.close();
	return true;
}

bool ParserDAT::save(const QString& filepath){
	if(filepath.isEmpty()) return false;

	QFile qfile(filepath);
	if(!qfile.open(QIODevice::WriteOnly)) return false;
	QDataStream out(&qfile);
	out.setByteOrder(QDataStream::LittleEndian);

	out << (quint32) m_root->childCount();
	out << (quint32) m_root->childCount();



	for(const TreeItem* child: m_root->getChilderen()){
		unsigned int i = 0;
		for(unsigned int a = 0; a < m_dataStructure.formats.size(); a++){
			const rowFormat& format =  m_dataStructure.formats[a];

			if(format.rowType == rowFormat::INT){
				switch (format.byteSize) {
					case 0:
					case 1:
					{
						out <<(qint8) child->data(i).toInt();
					}
						break;
					case 2:
					case 16:
					{
						out <<(qint16) child->data(i).toInt();
					}
						break;
					case 4:
					case 32:
					{
						out <<(qint32) child->data(i).toInt();
					}
						break;
					case 8:
					case 64:
					{
						out <<(qint64) child->data(i).toLongLong();
					}
						break;
					default:
					{
						qDebug() <<"(Int w) Invalid byteSize: "<<format.byteSize;
						out <<(qint8) child->data(i).toInt();
					}
						break;
				}
				i++;
			}else if(format.rowType == rowFormat::UINT){
				switch (format.byteSize) {
					case 0:
					case 1:
					{
						out <<(quint8) child->data(i).toUInt();
					}
						break;
					case 2:
					case 16:
					{
						out <<(quint16) child->data(i).toUInt();
					}
						break;
					case 4:
					case 32:
					{
						out <<(quint32) child->data(i).toUInt();
					}
						break;
					case 8:
					case 64:
					{
						out <<(quint8) child->data(i).toULongLong();
					}
						break;
					default:
					{
						qDebug() <<"(UInt w) Invalid byteSize: "<<format.byteSize;
						out <<(quint8) child->data(i).toUInt();
					}
						break;
				}
				i++;
			}else if(format.rowType == rowFormat::ZERO){
				for(unsigned int i = 0; i < format.byteSize; i++)
					out <<(quint8) 0;
			}else if(format.rowType == rowFormat::SHIFT_JIS){
				QString str = child->data(i).toString();
				if(str.size() > format.byteSize/2) str = str.left( format.byteSize/2);

				QByteArray bites = encodeToShiftJIS(str);// encoder->fromUnicode(mapName);
				out.writeRawData(bites.constData(), bites.size());
				for(unsigned int i = bites.size(); i < format.byteSize; i++)
					out<<(quint8) 0;
				bites.clear();
				i++;
			}
		}
	}

	for(unsigned int i = 0; i < 16; i++)
		out<<(quint8) 0;
	qfile.close();
	return true;
}


Qt::ItemFlags ParserDAT::flags(const QModelIndex &index) const{
    if (!m_root || !index.isValid())
        return 0;

    if(index.column() < m_dataStructure.header.size() )
     	return QAbstractItemModel::flags(index) | Qt::ItemFlag(m_dataStructure.header[index.column()].flag);
     else
     	return QAbstractItemModel::flags(index);
}

bool ParserDAT::setData(const QModelIndex &index, const QVariant &value, int role){
	if(!m_root || !index.isValid() || role != Qt::EditRole) return false;


	TreeItem *parentItem = static_cast<TreeItem*>(index.internalPointer());
	if(!parentItem) return false;


	if(value.type() != QVariant::String) return false;
	QString val = value.toString();

	int a = 0;
	for(unsigned int i = 0; i < index.column(); i++){
		if(m_dataStructure.formats[a].rowType == rowFormat::ZERO)
			a ++;
		a ++;
	}
	if(m_dataStructure.formats[a].rowType == rowFormat::ZERO)
		a ++;

	const rowFormat& format = m_dataStructure.formats[a];

	qDebug()<<"Set Data: "<<value<<" role: "<<role << " byte size: "<<QString::number(format.byteSize) << " at column: "<<QString::number(index.column());
	if(format.rowType == rowFormat::INT){
		QIntValidator validator(0, 0);
		switch (format.byteSize) {
			case 0:
			case 1:
			{
				validator.setRange(	-127, 127);
			}
				break;
			case 2:
			case 16:
			{
				validator.setRange(-32767, 32767);
			}
				break;
			case 4:
			case 32:
			{
				validator.setRange(-2147483647, 2147483647);
			}
				break;
			case 8:
			case 64:
			{
				validator.setRange(-2147483647, 2147483647);
			}
				break;
			default:
			break;
		}
		int pos = 0;
		if( validator.validate(val,pos) != QValidator::Acceptable)
			return false;
	}else if(format.rowType == rowFormat::UINT){
		QIntValidator validator(0, 0);
		switch (format.byteSize) {
			case 0:
			case 1:
			{
				validator.setRange(	0, 255);
			}
				break;
			case 2:
			case 16:
			{
				validator.setRange(	0, 65535);
			}
				break;
			case 4:
			case 32:
			{
				validator.setRange(	0, 4294967295);
			}
				break;
			case 8:
			case 64:
			{
				validator.setRange(	0, 4294967295);
			}
				break;
			default:
			break;
		}
		int pos = 0;
		if( validator.validate(val,pos) != QValidator::Acceptable)
			return false;
	}else if(format.rowType == rowFormat::SHIFT_JIS){
		if(value.type() != QVariant::String) return false;
		QString str = value.toString();
		if(str.size() > format.byteSize/2){
			if(!parentItem->setData(index.column(), str.left(format.byteSize/2))) return false;
		}else{
			if(!parentItem->setData(index.column(), value)) return false;
		}

		emit dataChanged(index, index, {Qt::EditRole});
		return true;

	}else{
		return false;
	}


	if(!parentItem->setData(index.column(), value)) return false;
	emit dataChanged(index, index, {Qt::EditRole});
	return true;
}

bool ParserDAT::insertFront(){
	if(!DataFile::insertFront()) return false;

	int a = 0;
	for(const column& col: m_dataStructure.header){
		if(col.type == column::INDEX){
			for(unsigned int i = 1; i < m_root->childCount(); i++)
				m_root->getChilderen()[i]->getData()[a] = m_root->getChilderen()[i]->getData()[a].toInt()+1;
			emit dataChanged(index(1,a), index(m_root->childCount(),a), {Qt::EditRole});
		}
		a++;
	}


	return true;
}
bool ParserDAT::insertBack(){
	if(!DataFile::insertBack()) return false;


	int i = 0;
	for(const column& col: m_dataStructure.header){
		if(col.type == column::INDEX){
			m_root->getChilderen()[m_root->childCount()-1]->getData()[i] = m_root->getChilderen()[m_root->childCount()-1]->getData()[i].toInt()+1;
			emit dataChanged(index(m_root->childCount(),i), index(m_root->childCount(),i), {Qt::EditRole});
		}
		i++;
	}

	return true;
}

bool ParserDAT::insertAt(int indexIn){
	if(!DataFile::insertAt(indexIn)) return false;


	int a = 0;
	for(const column& col: m_dataStructure.header){
		if(col.type == column::INDEX){
			for(unsigned int i = indexIn+1; i < m_root->childCount(); i++)
				m_root->getChilderen()[i]->getData()[a] = m_root->getChilderen()[i]->getData()[a].toInt()+1;
			emit dataChanged(index(indexIn+1,a), index(m_root->childCount(),a), {Qt::EditRole});
		}
		a++;
	}



	return true;
}

bool ParserDAT::removeAt(int indexIn){
	if(!DataFile::removeAt(indexIn)) return false;



	int a = 0;
	for(const column& col: m_dataStructure.header){
		if(col.type == column::INDEX){
			for(unsigned int i = indexIn; i < m_root->childCount(); i++)
				m_root->getChilderen()[i]->getData()[a] = m_root->getChilderen()[i]->getData()[a].toInt()-1;
			emit dataChanged(index(indexIn,a), index(m_root->childCount(),a), {Qt::EditRole});
		}
		a++;
	}

	return true;
}
