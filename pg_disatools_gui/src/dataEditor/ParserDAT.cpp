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

#include <dataEditor/ParserDAT.h>
#include <QDataStream>
#include <QDebug>
#include <QFileInfo>
#include <QIntValidator>
#include <QTextStream>
#include <QMessageBox>
#include <QTextCodec>
#include <QByteArray>
#include <util/ShiftJISEncoder.h>
#include <pg/util/PG_Exception.h>
#include <QBrush>

inline bool errorAt(const QString& info = ""){
	QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error",
					"Parsing error of definition file! \n" +info,
				 QMessageBox::Ok);
	return false;
}

struct defParser{
	QTextStream in;
	QChar currentChar = ' ';

	defParser(QFile& file): in(&file){

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
		bool isString = false;
		while(!in.atEnd()){
			if(currentChar == '\"'){
				isString = !isString;
				getNextChar();
				continue;
			}
			if(!isString)
				for(const QChar& v: endvalues)
					if(currentChar == v) return true;
			out.push_back(currentChar);
			getNextChar();
		}
		if(isString)
			return errorAt("A string needs to end with a '\"'!");
		return errorAt("Failed to read value!");
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

	inline QList<int> getColums(const QString& parameter, const parse& data){
		QList<int> list;
		if(parameter[0] == '$'){
			QStringList vals = parameter.mid(1).split('-');
			if(vals.isEmpty()) return list;
			if(vals.size() == 1){
				int num = vals[0].toUInt();
				if(num > 0 || num < data.header.size())
					list.push_back(num);
				return list;
			}
			if(vals.size() != 2) return list;
			int start = vals[0].toUInt();
			int end = vals[1].toUInt();
			for(unsigned int i = start; i < data.header.size() && i <= end; i++)
				list.push_back(i);
		}else{
			for(int i = 0; i < data.header.size(); i++)
				if(data.header[i] == parameter) list.push_back(i);
		}
		return list;
	}

	bool readSetColumnFlag(parse& data){
		skip();

		QStringList paras;
		if(!readParamters(paras)) return false;
		if(paras.size() != 2) errorAt("'SetColumnFlag(column, FLAG)': Has 2 parameters!");
		QList<int> columns = getColums(paras[0],data);

		const QString flag = paras[1].toLower();
		for(int i: columns){
			column& col = data.header[i];
			if(flag == "none"){
				col.flag = column::columnFLAGS::NONE;
			}else if(flag == "noedit"){
				col.flag = column::columnFLAGS::NOEDIT;
			}else{
				return errorAt("'SetColumnFlag(column, FLAG)': Unknown FLAG '"+flag + "' is used!");
			}
		}

		return true;
	}

	bool readSetColumnType(parse& data){
		skip();

		QStringList paras;
		if(!readParamters(paras)) return false;
		if(paras.size() != 2) errorAt("'SetColumnType(column, TYPE)': Has 2 parameters!");
		QList<int> columns = getColums(paras[0],data);


		const QString type = paras[1].toLower();
		for(int i: columns){
				column& col = data.header[i];
				if(type == "default"){
					col.type = column::columnType::DEFAULT;
				}else if(type == "index"){
					col.type = column::columnType::INDEX;
				}else{
					return errorAt("'SetColumnType(column, TYPE)': Unknown TYPE '"+type + "' is used!");
				}
		}

		return true;
	}

	bool readSetColumnColor(parse& data){
		skip();

		QStringList paras;
		if(!readParamters(paras)) return false;
		if(paras.isEmpty()) errorAt("SetColumnColor(column, r,g,b, [r2,g2,b2, [min, max]]): Can have 4, 7 or 9 parameters, but you have zero!");

		QList<int> colums = getColums(paras[0],data);

		if(paras.size() == 4){
			qDebug()<<"SetColumnColor(4) of "<<paras[0];
			for(int i: colums){
				column& col = data.header[i];
				col.hasColor = true;
				col.colorMinBackground.setRgb(paras[1].toUInt(), paras[2].toUInt(), paras[3].toUInt());
				col.colorMaxBackground = col.colorMinBackground;
			}
		}else if(paras.size() == 7){
			qDebug()<<"SetColumnColor(7) of "<<paras[0];
				for(int i: colums){
				column& col = data.header[i];
				col.hasColor = true;
				col.colorMinBackground.setRgb(paras[1].toUInt(), paras[2].toUInt(), paras[3].toUInt());
				col.colorMaxBackground.setRgb(paras[4].toUInt(), paras[5].toUInt(), paras[6].toUInt());
			}
		}else if(paras.size() == 9){
			qDebug()<<"SetColumnColor(9) of "<<paras[0];
			for(int i: colums){
				column& col = data.header[i];
				col.hasColor = true;
				col.colorMinBackground.setRgb(paras[1].toUInt(), paras[2].toUInt(), paras[3].toUInt());
				col.colorMaxBackground.setRgb(paras[4].toUInt(), paras[5].toUInt(), paras[6].toUInt());
				col.colorMin = paras[7].toUInt();
				col.colorMax = paras[8].toUInt();
			}
		}else{
			return errorAt("SetColumnColor(column, r,g,b, [r2,g2,b2, [min, max]]): Can have 4, 7 or 9 parameters!");
		}
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
				getNextChar();
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
			}else if(command == "unicode"){
				for(int i = 0; i < numberOf; i++)
					data.formats.push_back(rowFormat(rowFormat::UNICODE, bytes));
			}else if(command == "zero"){
				for(int i = 0; i < numberOf; i++)
					data.formats.push_back(rowFormat(rowFormat::ZERO, bytes));
			}else{
				return errorAt("Row format '"+command +"' is unknown!");
			}


		}
		return errorAt("A RowFormat block need to end with '}'!");
	}

	bool readHeaderFormat(parse& data){
		skip();
		if( currentChar != '{') return errorAt("A HeaderFormat black need to start with '{'!");
		getNextChar();
		while(!in.atEnd()){
			skip();
			if(currentChar == ';') {
				getNextChar();
				continue;
			}else if(currentChar == '}'){
				getNextChar();
				return true;
			}

			QString command;
			if(!read(command)) return false;
			if(command.isEmpty()) return errorAt("Given command is empty!");

			QStringList paras;
			if(!readParamters(paras)) return false;
			if(paras.empty()) return errorAt("HeaderFormat has zero parameters!");

			bool ok;
			int bytes = paras[0].toInt(&ok);
			if(!ok) return errorAt("Given parameter for a row format is not a number!");

			int numberOf = 1;
			if(paras.size() > 1){
				numberOf = paras[1].toInt(&ok);
				if(!ok || numberOf < 1) return errorAt("HeaderFormat 'number of' is invalid!");
			}

			headerFormat::type headerType = headerFormat::KEEP;
			if(paras.size() > 2 && paras[2].toLower() == "row_size"){
				headerType = headerFormat::ROW_SIZE;
			}

			qDebug()<<" Command: "<<command<<" number of "<<numberOf;
			command = command.toLower();
			if(command == "uint"){
				for(int i = 0; i < numberOf; i++)
					data.headerFormats.push_back(headerFormat(rowFormat::UINT, bytes,headerType));
			}else if(command == "int"){
				for(int i = 0; i < numberOf; i++)
					data.headerFormats.push_back(headerFormat(rowFormat::INT, bytes,headerType));
			}else if(command == "shiftjis"){
				for(int i = 0; i < numberOf; i++)
					data.headerFormats.push_back(headerFormat(rowFormat::SHIFT_JIS, bytes,headerType));
			}else if(command == "zero"){
				for(int i = 0; i < numberOf; i++)
					data.headerFormats.push_back(headerFormat(rowFormat::ZERO, bytes,headerType));
			}else{
				return errorAt("HeaderFormat '"+command +"' is unknown!");
			}


		}
		return errorAt("A HeaderFormat block need to end with '}'!");
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
			}else if(command == "setcolumncolor"){
				if(!readSetColumnColor(data)) return false;
			}else if(command == "rowformat"){
				qDebug() <<"Read RowFormat block";
				if(!readRowFormat(data)) return false;
			}else if(command == "headerformat"){
				qDebug() <<"Read HeaderFormat block";
				if(!readHeaderFormat(data)) return false;
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
		QStringList headers;
		if(!readParamters(headers)) return false;
		for(QString& header: headers){
			if(header[0] == '$')
				header[0] = '€';
			data.header.push_back(column(header));
		}
		qDebug() <<"Read table block";
		if(!readTableBlock(data)) return false;

		return true;
	}
};

#define UNICODESCRIPT false

#if UNICODESCRIPT
struct unicodeParser{
	QTextStream in;
	QChar currentChar = '?';

	QChar getNextChar(){
		if(in.atEnd()) return '!';
		QChar c;
		in >> c;
		currentChar = c;
		return c;
	}

	void skip(){
		while(!in.atEnd()){
			if(currentChar == ' ' || currentChar == '\n' || currentChar == '\t' ){
				getNextChar();
				continue;
			}
			break;
		}
	}

	QString read(){
		skip();
		QString out;
		if(currentChar == '\\'){
			getNextChar();
			for(unsigned int i = 0; i < 5; i++){
				out.push_back(currentChar);
				getNextChar();
			}
		}else{
			while(!in.atEnd()){
				if(currentChar == ' ' || currentChar == '\n' || currentChar == '\t'){
					getNextChar();
					return out;
				}
				out.push_back(currentChar);
				getNextChar();
			}
		}


		return out;
	}

	unicodeParser(QFile& file): in(&file){

	}

	bool parse(){

		unsigned int jis = 0x8140;
		QFile jisToUnicode("jisToUnicode.txt");
		QFile unicodeToJis("UnicodeToJis.txt");
		if(!jisToUnicode.open(QIODevice::WriteOnly)) return false;
		if(!unicodeToJis.open(QIODevice::WriteOnly)) return false;
		QTextStream jisToUnicodeSS(&jisToUnicode);
		QTextStream unicodeToJisSS(&unicodeToJis);
		//jisToUnicodeSS <<"{\n";
		unsigned int count = 0;
		getNextChar();
		while(!in.atEnd()){
			QString unicode = read();
			if(unicode[0] != 'u'){
				bool ok;
				jis = unicode.toUInt(&ok, 16);
				qDebug()<<"new jis: "<<QString::number(jis, 16);
				/*
				if(count < 15){
					jis += 16-count;
				}else if(count > 15){
					qDebug() << "ERROR count is bigger 15!";
				}
				*/
				continue;
			}
			unicode = unicode.mid(1, 4);
			if(unicode == "000d" || unicode == "000a") continue;
			if(unicode != "30fb"){
				//qDebug() << "{"<<QString::number(jis, 16) <<", 0x"<<unicode<<"}";
				//jisToUnicodeSS << "{ 0x"<<QString::number(jis, 16) <<", 0x"<<unicode<<"},\n";
				jisToUnicodeSS << "map[0x"<<QString::number(jis, 16) <<"] = 0x"<<unicode<<";\n";
				unicodeToJisSS  << "map[0x"<< unicode<<"] = 0x"<<QString::number(jis, 16)<<";\n";
			}
			jis++;
			count++;
		}
		//jisToUnicodeSS <<"}";
		jisToUnicode.close();
		unicodeToJis.close();

		return true;
	}
};

#endif

ParserDAT::ParserDAT(const QString& defFile, QObject *parent): DataFile(parent) {
#if UNICODESCRIPT
	QFile unicodes("resources/dataFiles/shiftJisAsUnicode.txt");
	if(!unicodes.open(QIODevice::ReadOnly)) return;
	unicodeParser par(unicodes);
	par.parse();
	unicodes.close();
#endif

	QFile qfile(defFile);
	if(!qfile.open(QIODevice::ReadOnly)) return;

	defParser p(qfile);
	if(p.parseDef(m_dataStructure)){
		qDebug() <<"Parsing SUCCESSFUL!";
		//set column format index
		int a = 0;
		for(unsigned int i = 0; i < m_dataStructure.formats.size() && a < m_dataStructure.header.size(); i++){
			const rowFormat& format = m_dataStructure.formats[i];
			if(format.rowType == rowFormat::ZERO)
				continue;
			m_dataStructure.header[a].formatIndex = i;
			a ++;
		}


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

int ParserDAT::getColumnWidth(int index) const{
	if(index < 0 || index >= m_dataStructure.header.size()) return 30;

	const column& col = m_dataStructure.header[index];
	if(col.formatIndex < 0) return 30;
	assert_Test("Format index is out of bounds!", col.formatIndex >= m_dataStructure.formats.size());

	const rowFormat& format = m_dataStructure.formats[col.formatIndex];

	if(format.rowType == rowFormat::SHIFT_JIS){
		return format.byteSize*4;
	}else{
		if(format.byteSize >= 4) return 100;
		if(format.byteSize >= 2) return 50;
	}

	return 30;
}

inline bool readFormat(const rowFormat& format, QDataStream& in, QList<QVariant>& dataOut){
	if(format.rowType == rowFormat::INT){
		switch (format.byteSize) {
			case 0:
			case 1:
			{
				qint8 v;
				in >> v;
				dataOut<< v;
			}
				break;
			case 2:
			case 16:
			{
				qint16 v;
				in >> v;
				dataOut<< v;
			}
				break;
			case 4:
			case 32:
			{
				qint32 v;
				in >> v;
				dataOut<< v;
			}
				break;
			case 8:
			case 64:
			{
				qint64 v;
				in >> v;
				dataOut<< v;
			}
				break;
			default:
			{
				qDebug() <<"(Int) Invalid byteSize: "<<format.byteSize;
				qint8 v;
				in >> v;
				dataOut<< v;
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
				dataOut<< v;
			}
				break;
			case 2:
			case 16:
			{
				quint16 v;
				in >> v;
				dataOut<< v;
			}
				break;
			case 4:
			case 32:
			{
				quint32 v;
				in >> v;
				dataOut<< v;
			}
				break;
			case 8:
			case 64:
			{
				quint64 v;
				in >> v;
				dataOut<< v;
			}
				break;
			default:
			{
				qDebug() <<"(UInt) Invalid byteSize: "<<format.byteSize;
				quint8 v;
				in >> v;
				dataOut<< v;
			}
				break;
		}
	}else if(format.rowType == rowFormat::ZERO){
		if(format.byteSize > 0)
			in.skipRawData(format.byteSize);
		//dataOut<< 0;
		return false;
	}else if(format.rowType == rowFormat::SHIFT_JIS){

		char* shift_JIS_String = new char[format.byteSize];
		in.readRawData(shift_JIS_String, format.byteSize);
		int find_zero = 0;
		for(; find_zero < format.byteSize; find_zero++)
			if(shift_JIS_String[find_zero] == 0) break;
		if(find_zero > format.byteSize) find_zero = format.byteSize;
		QString str = encodeShiftJisToUnicode(shift_JIS_String, find_zero); //decoder->toUnicode(shift_JIS_String, find_zero);
		delete shift_JIS_String;
		dataOut<< str;
	}else if(format.rowType == rowFormat::UNICODE){
		char* string = new char[format.byteSize];
		in.readRawData(string, format.byteSize);

		int find_zero = 0;
		for(; find_zero+1 < format.byteSize ; find_zero+=2)
			if(string[find_zero] == 0 && string[find_zero+1] == 0) break;

		if(find_zero > format.byteSize) find_zero = format.byteSize;
		QString str;
		for(unsigned int i = 0; i < find_zero; i+=2)
			str.push_back(QChar(string[i],string[i+1]));

		delete string;
		dataOut<< str;
	}

	return true;
}

bool ParserDAT::open(const QString& filepath){
	if(!m_root || m_dataStructure.header.empty() || m_dataStructure.formats.empty() || filepath.isEmpty()) return false;

	QFile qfile(filepath);
	if(!qfile.open(QIODevice::ReadOnly)) return false;

	beginRemoveRows(QModelIndex(),0,m_root->childCount());
	m_root->clear();
	m_headerData.clear();
	endRemoveRows();


	QDataStream in(&qfile);
	in.setByteOrder(QDataStream::LittleEndian);
	quint32 size = 0;

	for(const headerFormat& format: m_dataStructure.headerFormats){
		readFormat(format.format, in, m_headerData);
		if(format.headerType == headerFormat::ROW_SIZE && format.format.rowType != rowFormat::ZERO){
			quint32 testSize = m_headerData.last().toUInt();
			if(size == 0) {
				size = testSize;
			}else if(size != testSize){
				qDebug()<<"Both sizes should be the same! ("<<size<<" != "<<testSize<<")";
				return false;
			}
		}
	}

	if(size > 9000){
		qDebug()<<"Size is too big! ("<<size<<")";
		return false;
	}

	//size--;
	qDebug()<<"Found "<<size<<" Entry's!";


	beginInsertRows(QModelIndex(),0,size);
	for(unsigned int i = 0; i < size; i++){
		QList<QVariant> testData;
		for(const rowFormat& format: m_dataStructure.formats){
			readFormat(format, in, testData);
		}
		m_root->appendChild(new TreeItem(testData, m_root));
	}
	endInsertRows();

	qfile.close();
	return true;
}

inline bool writeFormat(const rowFormat& format, QDataStream& out, const QVariant& dataOut){
	if(format.rowType == rowFormat::INT){
		switch (format.byteSize) {
			case 0:
			case 1:
			{
				out <<(qint8) dataOut.toInt();
			}
				break;
			case 2:
			case 16:
			{
				out <<(qint16) dataOut.toInt();
			}
				break;
			case 4:
			case 32:
			{
				out <<(qint32) dataOut.toInt();
			}
				break;
			case 8:
			case 64:
			{
				out <<(qint64) dataOut.toLongLong();
			}
				break;
			default:
			{
				qDebug() <<"(Int w) Invalid byteSize: "<<format.byteSize;
				out <<(qint8) dataOut.toInt();
			}
				break;
		}
	}else if(format.rowType == rowFormat::UINT){
		switch (format.byteSize) {
			case 0:
			case 1:
			{
				out <<(quint8) dataOut.toUInt();
			}
				break;
			case 2:
			case 16:
			{
				out <<(quint16) dataOut.toUInt();
			}
				break;
			case 4:
			case 32:
			{
				out <<(quint32) dataOut.toUInt();
			}
				break;
			case 8:
			case 64:
			{
				out <<(quint64) dataOut.toULongLong();
			}
				break;
			default:
			{
				qDebug() <<"(UInt w) Invalid byteSize: "<<format.byteSize;
				out <<(quint8) dataOut.toUInt();
			}
				break;
		}
	}else if(format.rowType == rowFormat::ZERO){
		for(unsigned int i = 0; i < format.byteSize; i++)
			out <<(quint8) 0;
		return false;
	}else if(format.rowType == rowFormat::SHIFT_JIS){
		QString str = dataOut.toString();
		if(str.size() > format.byteSize/2) str = str.left( format.byteSize/2);

		QByteArray bytes = encodeUnicodeToShiftJis(str);// encoder->fromUnicode(mapName);
		out.writeRawData(bytes.constData(), bytes.size());
		for(unsigned int i = bytes.size(); i < format.byteSize; i++)
			out<<(quint8) 0;
		bytes.clear();
	}else if(format.rowType == rowFormat::UNICODE){
		QString str = dataOut.toString();
		if(str.size() > format.byteSize/2) str = str.left( format.byteSize/2);

		QByteArray bytes;
		for(const QChar& c: str){
			bytes.append(c.cell());
			bytes.append(c.row());
		}
		out.writeRawData(bytes.constData(), bytes.size());
		for(unsigned int i = bytes.size(); i < format.byteSize; i++)
			out<<(quint8) 0;
		bytes.clear();
	}
	return true;
}

bool ParserDAT::save(const QString& filepath){
	if(filepath.isEmpty()) return false;

	QFile qfile(filepath);
	if(!qfile.open(QIODevice::WriteOnly)) return false;
	QDataStream out(&qfile);
	out.setByteOrder(QDataStream::LittleEndian);

	unsigned int a = 0;
	for(const headerFormat& format: m_dataStructure.headerFormats){
		if(format.headerType == headerFormat::ROW_SIZE)
			writeFormat(format.format,out,m_root->childCount());
		else
			writeFormat(format.format,out,m_headerData[a]);
		a++;
	}

	for(const TreeItem* child: m_root->getChilderen()){
		unsigned int i = 0;
		for(unsigned int a = 0; a < m_dataStructure.formats.size(); a++){
			const rowFormat& format =  m_dataStructure.formats[a];
			if(writeFormat(format,out,child->data(i)))
				i++;
		}
	}

	for(unsigned int i = 0; i < 16; i++)
		out<<(quint8) 0;
	qfile.close();
	return true;
}

inline QColor interpolate(const QColor& start, const QColor& end, float value){
    if(value > 1.0) value = 1.0;
    else if(value < 0) value = 0;

    return QColor(
    		start.red()*(1.0f-value)+end.red()*value,
			start.green()*(1.0f-value)+end.green()*value,
			start.blue()*(1.0f-value)+end.blue()*value
    		);
}

QVariant ParserDAT::data(const QModelIndex &index, int role) const{
    if (!m_root || m_dataStructure.header.isEmpty() || !index.isValid())
        return QVariant();


    if( role == Qt::BackgroundRole ){ //ForegroundRole
    	 const column& c = m_dataStructure.header[index.column()];
    	 if(!c.hasColor || c.formatIndex < 0 ) return QVariant();
    	 const rowFormat& f = m_dataStructure.formats[c.formatIndex];

    	 if(f.rowType == rowFormat::INT || f.rowType == rowFormat::UINT){
    		 if(c.colorMinBackground == c.colorMaxBackground)
    			 return QBrush( c.colorMinBackground);
    		 else{
    		     TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    		     int v = item->data(index.column()).toInt();
    		      float p = (v-c.colorMin) / (float)(c.colorMax - c.colorMin);

    		      return QBrush( interpolate(c.colorMinBackground, c.colorMaxBackground, p));
    		 }
    	 }
    	 return QVariant();

    }else if(role == Qt::EditRole || role == Qt::DisplayRole){
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        return item->data(index.column());
    }else
    	return QVariant();
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


	//if(value.type() != QVariant::String) return false;
	QString val = value.toString();


	int a  = m_dataStructure.header[index.column()].formatIndex;
	if(a < 0) return false;
	assert_Test("Format index is out of bounds!", a >= m_dataStructure.formats.size());
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
