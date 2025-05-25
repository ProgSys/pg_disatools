#include "PG_Def.h"

#include <Util/PG_StringUtil.h>
#include <Util/PG_Exception.h>

using namespace PG_DefDefinition;

class PG_DefParseException : public PG_Exception{
	using PG_Exception::PG_Exception;
};

inline bool errorAt(const std::string& info = "") {
	throw PG_DefParseException(info);
	return false;
}


RowFormat RowFormat::create(const std::string& name, const std::vector<std::string>& args) {
	assert(!name.empty());

	auto format = RowFormat{ RowFormat::INVALID };
	if (PG_StringUtil::isEqualCaseless(name, "uint")) {
		format.type = RowFormat::UINT;
	}
	else if (PG_StringUtil::isEqualCaseless(name, "int")) {
		format.type = RowFormat::INT;
	}
	else if (PG_StringUtil::isEqualCaseless(name, "shiftjis")) {
		format.type = RowFormat::SHIFT_JIS;
	}
	else if (PG_StringUtil::isEqualCaseless(name, "unicode")) {
		format.type = RowFormat::UNICODE;
	}
	else if (PG_StringUtil::isEqualCaseless(name, "zero")) {
		format.type = RowFormat::ZERO;
	}
	else if (PG_StringUtil::isEqualCaseless(name, "hex")) {
		format.type = RowFormat::HEX;
	}
	else {
		throw PG_DefParseException("Unknown row format " + name + "!");
	}

	const auto paseInt = [&](int index, unsigned short& target) {
		if (args.size() > index) {
			target = PG_StringUtil::toInt(args[index]);
		}
		};

	switch (format.type)
	{
	case RowFormat::ZERO:
	case RowFormat::UINT:
	case RowFormat::INT: {
		paseInt(0, format.byteSize);
	}break;

	case RowFormat::SHIFT_JIS:
	case RowFormat::UNICODE:
	case RowFormat::HEX: {
		if (args.size() > 0 && PG_StringUtil::isEqualCaseless(args[0], "dynamic")) {
			if (args.size() < 1) {
				throw PG_DefParseException("The byte size for the lenght of a dynamic " + name + " must be provided!");
			}
			paseInt(1, format.dynamicLength);
		}
		else {
			paseInt(0, format.byteSize);
		}
	}break;
	default: break;
	}

	if (format.type == RowFormat::INVALID) {
		throw PG_DefParseException("Failed to parse rowFormat " + name + "!");
	}

	return format;
}

struct defParser {
	PG::STREAM::In* in;
	char currentChar = ' ';

	defParser(PG::STREAM::In* reader) : in(reader) {

	}



	char getNextChar() {
		if (in->eof()) return '!';
		currentChar = in->readChar();
		return currentChar;
	}

	void comment() {
		while (!in->eof()) {
			if (currentChar == '\n') break;
			getNextChar();
		}
		getNextChar();
		skip();
	}

	void skip(const std::string& skipChars = " \n\t\r") {
		if (in->eof()) return;

		if (currentChar == '#') {
			comment();
			return;
		}
		for (const char& v : skipChars)
			if (currentChar == v) {
				getNextChar();
				skip(skipChars);
			}
	}

	bool compare(const std::string& str) {
		skip();
		int i = 0;
		while (i < str.size() && !in->eof()) {
			if (std::tolower(currentChar) != std::tolower(str[i])) return errorAt("Could not find '" + str + "'!");
			getNextChar();
			i++;
		}
		return true;
	}

	bool read(std::string& out, const std::string& endvalues = " \n\t\r,();{}") {
		skip();
		bool isString = false;
		while (!in->eof()) {
			if (currentChar == '\"') {
				isString = !isString;
				getNextChar();
				continue;
			}
			if (!isString)
				for (const char& v : endvalues)
					if (currentChar == v) return true;
			out.push_back(currentChar);
			getNextChar();
		}
		if (isString)
			return errorAt("A string needs to end with a '\"'!");
		return errorAt("Failed to read value!");
	}

	bool readParamters(std::vector<std::string>& paramter) {
		skip();
		if (currentChar != '(') return errorAt("(multiple) Parameters need to start with '('!");
		getNextChar();
		while (!in->eof()) {
			skip();
			std::string para;
			if (!read(para)) return false;
			paramter.push_back(para);
			skip();
			if (currentChar == ')') {
				getNextChar();
				return true;
			}
			if (currentChar != ',') return errorAt("Parameters should be separated with a ',' or end with ')'!");
			getNextChar();
		}


		if (currentChar != ')') return errorAt("(multiple) Parameters need to end with ')'!");
		getNextChar();
		return true;
	}

	inline std::vector<int> getColums(const std::string& parameter, const PG_Def& data) {
		std::vector<int> list;
		if (parameter[0] == '$') {

			std::vector<std::string> vals = PG_StringUtil::split(PG_StringUtil::mid(parameter, 1), '-');
			if (vals.empty()) return list;
			if (vals.size() == 1) {
				int num = PG_StringUtil::toInt(vals[0]);
				if (num > 0 || num < data.header.size())
					list.push_back(num);
				return list;
			}
			if (vals.size() != 2) return list;
			int start = PG_StringUtil::toInt(vals[0]);
			int end = PG_StringUtil::toInt(vals[1]);
			for (unsigned int i = start; i < data.header.size() && i <= end; i++)
				list.push_back(i);
		}
		else {
			for (int i = 0; i < data.header.size(); i++)
				if (data.header[i] == parameter) list.push_back(i);
		}
		return list;
	}

	bool readSetColumnFlag(PG_Def& data) {
		skip();

		std::vector<std::string> paras;
		if (!readParamters(paras)) return false;
		if (paras.size() != 2) errorAt("'SetColumnFlag(column, FLAG)': Has 2 parameters!");
		std::vector<int> columns = getColums(paras[0], data);

		const std::string& flag = PG_StringUtil::toLower(paras[1]);
		for (int i : columns) {
			ColumnFormat& col = data.header[i];
			if (flag == "none") {
				col.flag = ColumnFormat::ColumnFLAGS::NONE;
			}
			else if (flag == "noedit") {
				col.flag = ColumnFormat::ColumnFLAGS::NOEDIT;
			}
			else {
				return errorAt("'SetColumnFlag(column, FLAG)': Unknown FLAG '" + flag + "' is used!");
			}
		}

		return true;
	}

	bool readSetColumnType(PG_Def& data) {
		skip();

		std::vector<std::string> paras;
		if (!readParamters(paras)) return false;
		if (paras.size() != 2) errorAt("'SetColumnType(column, TYPE)': Has 2 parameters!");
		std::vector<int> columns = getColums(paras[0], data);


		const std::string& type = PG_StringUtil::toLower(paras[1]);
		for (int i : columns) {
			ColumnFormat& col = data.header[i];
			if (type == "default") {
				col.type = ColumnFormat::ColumnType::DEFAULT;
			}
			else if (type == "index") {
				col.type = ColumnFormat::ColumnType::INDEX;
			}
			else {
				return errorAt("'SetColumnType(column, TYPE)': Unknown TYPE '" + type + "' is used!");
			}
		}

		return true;
	}

	bool readSetColumnColor(PG_Def& data) {
		skip();

		std::vector<std::string> paras;
		if (!readParamters(paras)) return false;
		if (paras.empty()) errorAt("SetColumnColor(column, r,g,b, [r2,g2,b2, [min, max]]): Can have 4, 7 or 9 parameters, but you have zero!");

		std::vector<int> colums = getColums(paras[0], data);

		if (paras.size() == 4) {

			for (int i : colums) {
				ColumnFormat& col = data.header[i];
				col.hasColor = true;
				col.colorMinBackground.x = PG_StringUtil::toInt(paras[1]);
				col.colorMinBackground.y = PG_StringUtil::toInt(paras[2]);
				col.colorMinBackground.z = PG_StringUtil::toInt(paras[3]);
				col.colorMaxBackground = col.colorMinBackground;
			}
		}
		else if (paras.size() == 7) {

			for (int i : colums) {
				ColumnFormat& col = data.header[i];
				col.hasColor = true;
				col.colorMinBackground.x = PG_StringUtil::toInt(paras[1]);
				col.colorMinBackground.y = PG_StringUtil::toInt(paras[2]);
				col.colorMinBackground.z = PG_StringUtil::toInt(paras[3]);

				col.colorMaxBackground.x = PG_StringUtil::toInt(paras[4]);
				col.colorMaxBackground.y = PG_StringUtil::toInt(paras[5]);
				col.colorMaxBackground.z = PG_StringUtil::toInt(paras[6]);
			}
		}
		else if (paras.size() == 9) {

			for (int i : colums) {
				ColumnFormat& col = data.header[i];
				col.hasColor = true;

				col.colorMinBackground.x = PG_StringUtil::toInt(paras[1]);
				col.colorMinBackground.y = PG_StringUtil::toInt(paras[2]);
				col.colorMinBackground.z = PG_StringUtil::toInt(paras[3]);

				col.colorMaxBackground.x = PG_StringUtil::toInt(paras[4]);
				col.colorMaxBackground.y = PG_StringUtil::toInt(paras[5]);
				col.colorMaxBackground.z = PG_StringUtil::toInt(paras[6]);

				col.colorMin = PG_StringUtil::toInt(paras[7]);
				col.colorMax = PG_StringUtil::toInt(paras[8]);
			}
		}
		else {
			return errorAt("SetColumnColor(column, r,g,b, [r2,g2,b2, [min, max]]): Can have 4, 7 or 9 parameters!");
		}
		return true;
	}

	template<typename T>
	void repeatLastComamnd(std::vector<T>& from) {
		skip();
		if (currentChar == ':') {
			getNextChar();
			skip();
			std::string str;
			read(str);

			int count = PG_StringUtil::toInt(str);
			if (count <= 1) throw PG_DefParseException("Repeat command extension is invalid! Must be a number and bigger then 1!");
			const auto lastFormat = from.back();
			--count;
			for (int i = 0; i < count; ++i) {
				from.push_back(lastFormat);
			}
		}
	}

	bool readRowFormat(PG_Def& data) {
		skip();
		if (currentChar != '{') return errorAt("A RowFormat black need to start with '{'!");
		getNextChar();
		while (!in->eof()) {
			skip();
			if (currentChar == ';') {
				getNextChar();
				continue;
			}
			else if (currentChar == '}') {
				getNextChar();
				return true;
			}
			else if (currentChar == ')' || currentChar == ',' || currentChar == '"') {
				return errorAt(std::string("Unexpected char '") + currentChar + "'!");
			}

			std::string command;
			if (!read(command)) return false;
			if (command.empty()) return errorAt("Failed to parse command!");

			std::vector<std::string> commandArgs;
			if (!readParamters(commandArgs)) return false;
			if (commandArgs.empty()) return errorAt("RowFormat has zero parameters!");

			data.formats.push_back(RowFormat::create(command, commandArgs));

			//do we repeat the command?
			repeatLastComamnd(data.formats);

		}
		return errorAt("A RowFormat block need to end with '}'!");
	}

	bool readHeaderFormat(PG_Def& data) {
		skip();
		if (currentChar != '{') return errorAt("A HeaderFormat black need to start with '{'!");
		getNextChar();
		while (!in->eof()) {
			skip();
			if (currentChar == ';') {
				getNextChar();
				continue;
			}
			else if (currentChar == '}') {
				getNextChar();
				return true;
			}

			std::string command;
			if (!read(command)) return false;
			if (command.empty()) return errorAt("Given command is empty!");

			std::vector<std::string> commandArgs;
			if (!readParamters(commandArgs)) return false;
			if (commandArgs.empty()) return errorAt("HeaderFormat has zero parameters!");

			auto fromat = RowFormat::create(command, commandArgs);

			HeaderFormat::Type headerType = HeaderFormat::KEEP;
			if (commandArgs.size() > 1 && PG_StringUtil::isEqualCaseless(commandArgs.back(), "row_size")) {
				headerType = HeaderFormat::ROW_SIZE;
			}

			data.headerFormats.push_back(HeaderFormat{ std::move(fromat), headerType });

			//do we repeat the command?
			repeatLastComamnd(data.headerFormats);
		}
		return errorAt("A HeaderFormat block need to end with '}'!");
	}

	bool readTableBlock(PG_Def& data) {
		skip();
		if (currentChar != '{') return errorAt("A table block need to start with '{'!");
		getNextChar();
		while (!in->eof()) {
			skip();
			if (currentChar == ';') {
				getNextChar();
				continue;
			}
			else if (currentChar == '}') {
				return true;
			}
			std::string command;
			if (!read(command)) return false;
			PG_StringUtil::toLower(command);

			if (command == "setcolumnflag") {
				if (!readSetColumnFlag(data)) return false;
			}
			else if (command == "setcolumntype") {
				if (!readSetColumnType(data)) return false;
			}
			else if (command == "setcolumncolor") {
				if (!readSetColumnColor(data)) return false;
			}
			else if (command == "rowformat") {
				if (!readRowFormat(data)) return false;
			}
			else if (command == "headerformat") {
				if (!readHeaderFormat(data)) return false;
			}
			else {
				return errorAt("Command '" + command + "' is unknown!");
			}

		}
		return errorAt("A table block need to end with '}'!");
	}

	bool parseDef(PG_Def& data) {

		if (!compare("TABLE")) return false;

		std::vector<std::string> headers;
		if (!readParamters(headers)) return false;
		for (std::string& header : headers) {
			if (header[0] == '$')
				header[0] = '€';
			data.header.push_back(ColumnFormat(header));
		}

		if (!readTableBlock(data)) return false;

		return true;
	}
};

PG_Def PG_Def::parse(PG::STREAM::In* reader) {
	assert(reader);
	PG_Def out;
	defParser parser{ reader };
	parser.parseDef(out);
	return out;
}