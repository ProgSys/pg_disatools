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

#include <ParserDAT.h>
#include <QDataStream>
#include <QDebug>
#include <QFileInfo>
#include <QIntValidator>
#include <QTextStream>
#include <QMessageBox>
#include <QTextCodec>
#include <QByteArray>
#include <Util/Misc/ShiftJISEncoder.h>
#include <Util/Misc/ResourcePath.h>
#include <Util/PG_Exception.h>
#include <QBrush>
#include <QException>

class DefParseException : public QException
{
public:
	DefParseException(const QString& _msg) : msg(_msg){}
	DefParseException(QString&& _msg) : msg(std::move(_msg)){}
	void raise() const override { throw *this; }
	DefParseException* clone() const override { return new DefParseException(msg); }

	QString msg;
};

inline bool errorAt(const QString& info = ""){
	throw DefParseException(info);
	return false;
}


rowFormat rowFormat::create(const QString& name, const QStringList& args) {
	assert(!name.isEmpty());

	auto format = rowFormat{ rowFormat::INVALID }; 
	if (name.compare("uint", Qt::CaseInsensitive) == 0) {
		format.type = rowFormat::UINT;
	}
	else if (name.compare("int", Qt::CaseInsensitive) == 0) {
		format.type = rowFormat::INT;
	}
	else if (name.compare("shiftjis", Qt::CaseInsensitive) == 0) {
		format.type = rowFormat::SHIFT_JIS;
	}
	else if (name.compare("unicode", Qt::CaseInsensitive) == 0) {
		format.type = rowFormat::UNICODE;
	}
	else if (name.compare("zero", Qt::CaseInsensitive) == 0) {
		format.type = rowFormat::ZERO;
	}
	else if (name.compare("hex", Qt::CaseInsensitive) == 0) {
		format.type = rowFormat::HEX;
	}
	else {
		qDebug() << "Unknown row format " << name;
		throw DefParseException("Unknown row format " + name + "!");
	}

	const auto paseInt = [&](int index, unsigned short& target) {
		if (args.size() > index) {
			bool ok;
			int bytes = args[index].toInt(&ok);
			if (!ok) throw DefParseException("Argument "+ QString::number(index+1) + " in " + name + " is not a number!");
			target = bytes;
		}
	};

	switch (format.type)
	{
	case rowFormat::ZERO:
	case rowFormat::UINT: 
	case rowFormat::INT: {
		paseInt(0, format.byteSize);
	}break;

	case rowFormat::SHIFT_JIS: 
	case rowFormat::UNICODE: 
	case rowFormat::HEX: {
		if (args.size() > 0 && args[0].compare("dynamic", Qt::CaseInsensitive) == 0) {
			if (args.size() < 1) {
				throw DefParseException("The byte size for the lenght of a dynamic "+ name + " must be provided!");
			}
			paseInt(1, format.dynamicLength);
		}
		else {
			paseInt(0, format.byteSize);
		}
	}break;
	default: break;
	}

	if (format.type == rowFormat::INVALID) {
		throw DefParseException("Failed to parse rowFormat " + name + "!");
	}

	qDebug() << " Command: " << name <<"("<< args.join(", ") << ");";
	return format;
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

	template<typename T>
	void repeatLastComamnd(QList<T>& from) {
		skip();
		auto c = currentChar.toLatin1();
		if (currentChar == ':') {
			getNextChar();
			skip();
			QString str;
			read(str);

			bool ok;
			int count = str.toInt(&ok);
			if (!ok || count <= 1) throw DefParseException("Repeat command extension is invalid! Must be a number and bigger then 1!");
			const auto lastFormat = from.back();
			--count;
			for (int i = 0; i < count; ++i) {
				from.push_back(lastFormat);
			}
		}
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
			else if (currentChar == ')' || currentChar == ',' || currentChar == '"') {
				return errorAt(QString("Unexpected char '") + currentChar + "'!");
			}

			QString command;
			if(!read(command)) return false;
			if(command.isEmpty()) return errorAt("Failed to parse command!");

			QStringList commandArgs;
			if(!readParamters(commandArgs)) return false;
			if(commandArgs.empty()) return errorAt("RowFormat has zero parameters!");

			data.formats.push_back(rowFormat::create(command, commandArgs));
			
			//do we repeat the command?
			repeatLastComamnd(data.formats);

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

			QStringList commandArgs;
			if(!readParamters(commandArgs)) return false;
			if(commandArgs.empty()) return errorAt("HeaderFormat has zero parameters!");

			auto fromat = rowFormat::create(command, commandArgs);

			headerFormat::Type headerType = headerFormat::KEEP;
			if(commandArgs.size() > 1 && commandArgs.back().toLower() == "row_size") {
				headerType = headerFormat::ROW_SIZE;
			}

			data.headerFormats.push_back(headerFormat{ std::move(fromat), headerType });

			//do we repeat the command?
			repeatLastComamnd(data.headerFormats);
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
	QFile unicodes(getResourcePath() + "/dataFiles/shiftJisAsUnicode.txt");
	if(!unicodes.open(QIODevice::ReadOnly)) return;
	unicodeParser par(unicodes);
	par.parse();
	unicodes.close();
#endif

	QFile qfile(defFile);
	if(!qfile.open(QIODevice::ReadOnly)) return;

	defParser p(qfile);
	try
	{
		p.parseDef(m_dataStructure);
		qDebug() << "Parsing SUCCESSFUL!";
	}
	catch (const DefParseException& ex) {
		m_dataStructure.clear();
		qDebug() << "Parsing FAILED!";
		QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error",
			"Parsing error in definition file "+ QFileInfo(defFile).fileName() + " at char number " + QString::number(p.in.pos()) + "! \n" + ex.msg,
			QMessageBox::Ok);
	}

	//set column format index
	for (unsigned int i = 0, a = 0; i < m_dataStructure.formats.size() && a < m_dataStructure.header.size(); ++i) {
		const rowFormat& format = m_dataStructure.formats[i];
		if (format.type == rowFormat::ZERO)
			continue;
		m_dataStructure.header[a].formatIndex = i;
		++a;
	}


	QList<QVariant> rootData;
	for (const column& col : m_dataStructure.header) {
		rootData << col.name;
	}
	if (m_root) delete m_root;
	m_root = new TreeItem(rootData);


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

	if(format.type == rowFormat::SHIFT_JIS){
		return format.byteSize*4;
	}else{
		if(format.byteSize >= 4) return 100;
		if(format.byteSize >= 2) return 50;
	}

	return 30;
}

inline bool readFormat(const rowFormat& format, QDataStream& in, QList<QVariant>& dataOut){
	switch (format.type)
	{

	case rowFormat::INT: {
		switch (format.byteSize) {
		case 0:
		case 1:
		{
			qint8 v;
			in >> v;
			dataOut << v;
		}
		break;
		case 2:
		case 16:
		{
			qint16 v;
			in >> v;
			dataOut << v;
		}
		break;
		case 4:
		case 32:
		{
			qint32 v;
			in >> v;
			dataOut << v;
		}
		break;
		case 8:
		case 64:
		{
			qint64 v;
			in >> v;
			dataOut << v;
		}
		break;
		default:
		{
			qDebug() << "(Int) Invalid byteSize: " << format.byteSize;
			qint8 v;
			in >> v;
			dataOut << v;
		}
		break;
		}
	}break;
	case rowFormat::UINT: {
		switch (format.byteSize) {
		case 0:
		case 1:
		{
			quint8 v;
			in >> v;
			dataOut << v;
		}
		break;
		case 2:
		case 16:
		{
			quint16 v;
			in >> v;
			dataOut << v;
		}
		break;
		case 4:
		case 32:
		{
			quint32 v;
			in >> v;
			dataOut << v;
		}
		break;
		case 8:
		case 64:
		{
			quint64 v;
			in >> v;
			dataOut << v;
		}
		break;
		default:
		{
			qDebug() << "(UInt) Invalid byteSize: " << format.byteSize;
			quint8 v;
			in >> v;
			dataOut << v;
		}
		break;
		}
	}break;
	case rowFormat::ZERO: {
		if (format.byteSize > 0)
			in.skipRawData(format.byteSize);
		//dataOut<< 0;
		return false;
	}break;
	case rowFormat::SHIFT_JIS: {
		int size = 0;
		if (format.dynamicLength > 0) {
			char* c = (char*)&size;
			in.readRawData(c, format.dynamicLength);
		}
		else {
			size = format.byteSize;
		}

		char* shift_JIS_String = new char[size];
		in.readRawData(shift_JIS_String, size);

		//crop string if not dynamic
		if (format.dynamicLength <= 0) {
			int find_zero = 0;
			for (; find_zero < size; find_zero++)
				if (shift_JIS_String[find_zero] == 0) break;
			size = std::min(size, find_zero);
		}

		QString str = encodeShiftJisToUnicode(shift_JIS_String, size); //decoder->toUnicode(shift_JIS_String, find_zero);
		delete[] shift_JIS_String;
		dataOut << str;
	} break;
	case rowFormat::HEX:
	case rowFormat::UNICODE: {
		int size = 0;
		if (format.dynamicLength > 0) {
			char* c = (char*)&size;
			in.readRawData(c, format.dynamicLength);
		}
		else {
			size = format.byteSize;
		}

		char* string = new char[size];
		in.readRawData(string, size);

		//crop string if not dynamic
		if (format.dynamicLength <= 0) {
			int find_zero = 0;
			for (; find_zero + 1 < format.byteSize; find_zero += 2)
				if (string[find_zero] == 0 && string[find_zero + 1] == 0) break;
			size = std::min(size, find_zero);
		}

		QString str;
		if (format.type == rowFormat::HEX) {
			QByteArray data(string, size);
			str = data.toHex();
		}
		else {
			for (unsigned int i = 0; i < size; i += 2)
				str.push_back(QChar(string[i], string[i + 1]));
		}


		delete[] string;
		dataOut << str;
	} break;

	default: assert(false); break;
	}
	return true;
}

bool ParserDAT::open(const QString& filepath){
	if(!m_root || m_dataStructure.header.empty() || m_dataStructure.formats.empty() || filepath.isEmpty()) return false;

	QFile qfile(filepath);
	if(!qfile.open(QIODevice::ReadOnly)) return false;

	if (m_root->childCount() > 0) {
		beginRemoveRows(QModelIndex(),0,m_root->childCount()-1);
		m_root->clear();
		m_headerData.clear();
		endRemoveRows();
	}
	else {
		m_headerData.clear();
	}


	QDataStream in(&qfile);
	in.setByteOrder(QDataStream::LittleEndian);
	quint32 size = 0;

	for(const headerFormat& format: m_dataStructure.headerFormats){
		readFormat(format.format, in, m_headerData);
		if(format.headerType == headerFormat::ROW_SIZE && format.format.type != rowFormat::ZERO){
			quint32 testSize = m_headerData.last().toUInt();
			if(size == 0) {
				size = testSize;
			}else if(size != testSize){
				qDebug()<<"Both sizes should be the same! ("<<size<<" != "<<testSize<<")";
				return false;
			}
		}
	}

	qDebug()<<"Found "<<size<<" Entry's!";
	if(size > 9000){
		qDebug()<<"Size is too big! ("<<size<<")";
		return false;
	}
	else if (size == 0) {
		return true;
	}

	beginInsertRows(QModelIndex(),0,size-1);
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

	switch (format.type) {
	case rowFormat::INT: {
		switch (format.byteSize) {
		case 0:
		case 1:
		{
			out << (qint8)dataOut.toInt();
		}
		break;
		case 2:
		case 16:
		{
			out << (qint16)dataOut.toInt();
		}
		break;
		case 4:
		case 32:
		{
			out << (qint32)dataOut.toInt();
		}
		break;
		case 8:
		case 64:
		{
			out << (qint64)dataOut.toLongLong();
		}
		break;
		default:
		{
			qDebug() << "(Int w) Invalid byteSize: " << format.byteSize;
			out << (qint8)dataOut.toInt();
		}
		break;
		}
	}break;
	case rowFormat::UINT: {
		switch (format.byteSize) {
		case 0:
		case 1:
		{
			out << (quint8)dataOut.toUInt();
		}
		break;
		case 2:
		case 16:
		{
			out << (quint16)dataOut.toUInt();
		}
		break;
		case 4:
		case 32:
		{
			out << (quint32)dataOut.toUInt();
		}
		break;
		case 8:
		case 64:
		{
			out << (quint64)dataOut.toULongLong();
		}
		break;
		default:
		{
			qDebug() << "(UInt w) Invalid byteSize: " << format.byteSize;
			out << (quint8)dataOut.toUInt();
		}
		break;
		}
	}break;
	case rowFormat::ZERO: {
		for (unsigned int i = 0; i < format.byteSize; i++)
			out << (quint8)0;
		return false;
	}break;
	case rowFormat::SHIFT_JIS:
	case rowFormat::UNICODE: {
		QString str = dataOut.toString();

		if (format.dynamicLength > 0) {
			int size = str.size();
			out.writeRawData((char*)&size, format.dynamicLength);
		}
		else if (str.size() > format.byteSize / 2) {
			str = str.left(format.byteSize / 2);
		}

		QByteArray bytes;
		switch (format.type)
		{
		case rowFormat::SHIFT_JIS:
			bytes = encodeUnicodeToShiftJis(str);// encoder->fromUnicode(mapName);
			break;
		case rowFormat::UNICODE:
			for (const QChar& c : str) {
				bytes.append(c.cell());
				bytes.append(c.row());
			}
			break;
		case rowFormat::HEX:
			bytes = QByteArray::fromHex(str.toUtf8());
			break;
		default: assert(false); break;
		}

		out.writeRawData(bytes.constData(), bytes.size());

		//fill rest with zero
		if (format.byteSize > 0) {
			for (unsigned int i = bytes.size(); i < format.byteSize; i++)
				out << (quint8)0;
		}

	} break;
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

    	 if(f.type == rowFormat::INT || f.type == rowFormat::UINT){
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
	if(format.type == rowFormat::INT){
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
	}else if(format.type == rowFormat::UINT){
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
	}else if(format.type == rowFormat::SHIFT_JIS){
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
