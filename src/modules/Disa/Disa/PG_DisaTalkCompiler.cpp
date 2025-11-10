#include "PG_DisaTalkCompiler.h"
#include <Script/CScript/PG_CScriptTokenizer.h>
#include <Stream/PG_StreamInByteArray.h>
#include <Util/PG_StringUtil.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <glm/glm.hpp>
#include <Base/PG_Variety.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>

using namespace PG_DisaTalkScript;


std::vector<unsigned char> PG_DisaTalkScript::compile(const std::string& code) {
	if (code.empty()) return {};
	PG::STREAM::InByteArray stream(&code.front(), code.size());
	return compile(&stream);
}

static inline void throwError(int line, std::string&& msg) {
	throw PG_CompileException(line, msg);
}

static inline void goNext(PG_CScriptTokenizer::Tokens::iterator& it, const PG_CScriptTokenizer::Tokens::iterator& end, int& line) {
	
	do{
		++it;
		if (it == end) {
			throwError(line, "Unexpected script ending! Like in the middle of a function definition!");
		}
		if (it->type == PG_Enum::CScript::EndOfLine) {
			++line;
			continue;
		}
	} while (false);
}

static inline void goNextExpectEnd(PG_CScriptTokenizer::Tokens::iterator& it, const PG_CScriptTokenizer::Tokens::iterator& end, int& line) {

	do {
		++it;
		if (it == end) {
			return;
		}
		if (it->type == PG_Enum::CScript::EndOfLine) {
			++line;
			continue;
		}
	} while (false);
}

const std::unordered_map<std::string, std::pair<unsigned char, glm::ivec2>> gFunctionToOpCode = {
	{"loaddefaultvariablesv1", {0x00, glm::ivec2(0) }},
	{"dialog", {0x01, glm::ivec2(1) }},
	{"confirm", {0x02, glm::ivec2(0) }},
	{"confirmclose", {0x03, glm::ivec2(0) }},
	{"charleft", {0x04, glm::ivec2(4) }},
	{"portrait", {0x05, glm::ivec2(1) }},
	{"voice", {0x06, glm::ivec2(1) }},
	{"speechbubble", {0x07, glm::ivec2(2) }},
	{"namelabel", {0x08, glm::ivec2(2) }},
	{"name", {0x08, glm::ivec2(2) }},
	{"set", {0x09, glm::ivec2(2) }},
	{"clear", {0x0A, glm::ivec2(2) }},
	{"script", {0x0B, glm::ivec2(1) }},
	{"call", {0x0C, glm::ivec2(1) }},
	{"menustart", {0x14, glm::ivec2(2) }},
	{"menuentry", {0x14, glm::ivec2(2) }},
	{"menuend", {0xFD, glm::ivec2(0) }},
	{"scriptcustom", {0x0D, glm::ivec2(1) }},
	{"callcustom", {0x0F, glm::ivec2(1) }},
	{"charmid", {0x0E, glm::ivec2(4) }},
	{"charright", {0x18, glm::ivec2(4) }},
	{"loadimg", {0x64, glm::ivec2(1,2) }},
	{"showbox", {0xC9, glm::ivec2(1) }},
	{"confirmauto", {0x65, glm::ivec2(3) }},
	{"return", {0xFF, glm::ivec2(0) }},
	{"restart", {0xFE, glm::ivec2(0) }},
};

const std::unordered_map<unsigned char, std::pair<std::string, glm::ivec2>> gOpCodeToFunction = {
	{0x01, { "dialog", glm::ivec2(1) }},
	{0x02, { "confirm",glm::ivec2(0) }},
	{0x03, { "confirmClose",glm::ivec2(0) }},
	{0x04, { "charLeft",glm::ivec2(4) }},
	{0x05, { "portrait",glm::ivec2(1) }},
	{0x06, { "voice", glm::ivec2(1) }},
	{0x07, { "speechBubble", glm::ivec2(2) }},
	{0x08, { "nameLabel", glm::ivec2(2) }},
	{0x09, { "set", glm::ivec2(2) }},
	{0x0A, { "clear", glm::ivec2(2) }},
	{0x0B, { "script", glm::ivec2(1) }},
	{0x0C, { "call", glm::ivec2(1) }},
	{0x14, { "menuEntry", glm::ivec2(1) }},
	{0xFD, { "menuEnd", glm::ivec2(0) }},
	{0x0D, { "scriptCustom", glm::ivec2(1) }},
	{0x0F, { "callCustom", glm::ivec2(1) }},
	{0x0E, { "charMid", glm::ivec2(4) }},
	{0x18, { "charRight", glm::ivec2(4) }},
	{0x64, { "loadImg", glm::ivec2(1,2) }},
	{0x65, { "ConfirmAuto", glm::ivec2(3) }},
	{0xC9, { "showBox",glm::ivec2(1) }},
	{0xFF, { "return",glm::ivec2(0) }},
	{0xFE, { "restart",glm::ivec2(0) }},
};

struct DisaGameStateFilterExport {
	unsigned short bitIndex : 3; //the bit that should be tested
	unsigned short index : 13; //the index of a gamestate at 0x00619330
};

static inline int getInt(const PG_CScriptTokenizer::Token& n, const std::map<std::string, PG_Variety>& vars) {
	if (n.type == PG_Enum::CScript::Int || n.type == PG_Enum::CScript::Float) {
		return PG_StringUtil::toInt(n.str);
	}
	else if (n.type == PG_Enum::CScript::Undefined) {
		auto findIt = vars.find(n.str);
		if (findIt != vars.end()) {
			return findIt->second.get<int>();
		}
		else {
			throwError(0, "Variable \"" + n.str + "\" not found!");
		}
	}
	throwError(0, "Invalid type expected int!");
	return 0;
}

static inline int getBool(const PG_CScriptTokenizer::Token& n, const std::map<std::string, PG_Variety>& vars) {
	if (n.type == PG_Enum::CScript::True || n.type == PG_Enum::CScript::False) {
		return n.type == PG_Enum::CScript::True;
	}
	else if (n.type == PG_Enum::CScript::Undefined) {
		auto findIt = vars.find(n.str);
		if (findIt != vars.end()) {
			return findIt->second.get<bool>();
		}
		else {
			throwError(0, "Variable \"" + n.str + "\" not found!");
		}
	}
	throwError(0, "Invalid type expected boolean!");
	return 0;
}


std::vector<unsigned char> PG_DisaTalkScript::compile(PG::STREAM::In* codeStream) {
	std::vector<unsigned char> out;
	PG_CScriptTokenizer tokenizer;
	auto tokens = tokenizer.tokenize(codeStream);
	if (tokens.size() < 3) return out;

	std::erase_if(tokens, [](const PG_CScriptTokenizer::Token& n) { return n.type == PG_Enum::CScript::EmptySpace; });

	std::map<std::string, PG_Variety> vars;

	int line = 0;
	auto it = tokens.begin();
	auto endIt = tokens.end();
	while (it != endIt){
		if (it->type == PG_Enum::CScript::EndOfLine) {
			++it;
			++line;
			continue;
		}else
		if (it->type == PG_Enum::CScript::CommentLine) {
			while (it != endIt && it->type != PG_Enum::CScript::EndOfLine) {
				++it;
			}
			continue;
		}else if (it->type == PG_Enum::CScript::CommentOpen) {
			int depth = 0;
			while (it != endIt) {
				++it;
				if (it->type == PG_Enum::CScript::CommentOpen) ++depth;
				else if (it->type == PG_Enum::CScript::CommentClose && depth > 0) --depth;
				else if (it->type == PG_Enum::CScript::CommentClose) break;
				else if (it->type == PG_Enum::CScript::EndOfLine) ++line;
			}
			continue;
		}else if (it->type == PG_Enum::CScript::Integer) {
			goNext(it, endIt, line);
			if (it->type != PG_Enum::CScript::Undefined) {
				throwError(line, "Invalid variable name!");
			}
			const auto& varName = it->str;
			goNext(it, endIt, line);
			if (it->type != PG_Enum::CScript::Set) {
				throwError(line, "Equal expected for variable!");
			}
			goNext(it, endIt, line);
			if (it->type != PG_Enum::CScript::Int && it->type != PG_Enum::CScript::Float) {
				throwError(line, "Expected a number!");
			}
			vars[varName] = PG_StringUtil::toInt(it->str);

			goNext(it, endIt, line); //;
			if (it->type != PG_Enum::CScript::Semikolon && it->type != PG_Enum::CScript::EndOfLine) {
				throwError(line, "Each variable must end with a Semikolon!");
			}
			goNextExpectEnd(it, endIt, line);

			continue;
		}
		else if (it->type == PG_Enum::CScript::Boolean) {
			goNext(it, endIt, line);
			if (it->type != PG_Enum::CScript::Undefined) {
				throwError(line, "Invalid variable name!");
			}
			const auto& varName = it->str;
			goNext(it, endIt, line);
			if (it->type != PG_Enum::CScript::Set) {
				throwError(line, "Equal expected for variable!");
			}
			goNext(it, endIt, line);
			if (it->type != PG_Enum::CScript::False && it->type != PG_Enum::CScript::True) {
				throwError(line, "Expected a bool (true or false)!");
			}
			vars[varName] = bool(it->type == PG_Enum::CScript::True);

			goNext(it, endIt, line); //;
			if (it->type != PG_Enum::CScript::Semikolon && it->type != PG_Enum::CScript::EndOfLine) {
				throwError(line, "Each variable must end with a Semikolon!");
			}
			goNextExpectEnd(it, endIt, line);

			continue;
		}

		if (it->type != PG_Enum::CScript::Undefined && it->type != PG_Enum::CScript::Return) {
			throwError(line, "Line didn't start with a function!");
		}
		auto functionName = PG_StringUtil::toLowerCopy(it->str);
		auto findIt = gFunctionToOpCode.find(functionName);
		
		if (findIt == gFunctionToOpCode.end()) {
			throwError(line, "Unknown function: " + it->str + "!");
		}

		std::vector<PG_CScriptTokenizer::Tokens::iterator> args;
		if(findIt->second.first != 0xFF){ //no need to check for brackets for return
			goNext(it, endIt, line);
			if (it->type != PG_Enum::CScript::BracketOpen) {
				throwError(line, "Open bracket is missing! Function arguments block must start and close with brackets! Syntax: FunctionName(args...);");
			}
		
			
			if (findIt->second.second.y > 0) {
				while (true) {
					goNext(it, endIt, line);
					args.push_back(it);
					if (args.size() > 4) {
						throwError(line, "Function has too many arguments! Expected: " + std::to_string(findIt->second.second.y));
					}
					if (it->type == PG_Enum::CScript::Minus || it->type == PG_Enum::CScript::Plus) {
						goNext(it, endIt, line);
						args.back()->type = it->type;
						args.back()->str += it->str;
					}

					goNext(it, endIt, line); //comma
					if (it->type == PG_Enum::CScript::BracketClose) {
						break;
					}else if (it->type != PG_Enum::CScript::Comma) {
						throwError(line, "Comma is missing! Arguments in a function must be seperated via a comma! Syntax: FunctionName(arg1, arg2, arg3);");
					}
				}
			}
			else {
				goNext(it, endIt, line); //comma
				if (it->type != PG_Enum::CScript::BracketClose) {
					throwError(line, "Close bracket is missing! Function arguments block must close and start with brackets! Syntax: FunctionName(args...);");
				}
			}
		}

		goNext(it, endIt, line); //;
		if (it->type != PG_Enum::CScript::Semikolon && it->type != PG_Enum::CScript::EndOfLine) {
			throwError(line, "Each function must end with a Semikolon!");
		}
		goNextExpectEnd(it, endIt, line);

		if (args.size() < findIt->second.second.x || args.size() > findIt->second.second.y) {
			if (findIt->second.second.x != findIt->second.second.y) {
				throwError(line, "Invalid number of arguments! The function " + it->str + " expects between " 
					+ std::to_string(findIt->second.second.x) + " - " + std::to_string(findIt->second.second.y) + " arguments!");
			}
			else {
				throwError(line, "Invalid number of arguments! The function "+ it->str + " expects " + std::to_string(findIt->second.second.x)+ " arguments!");
			}
		}
		//parse function
		//example 1245
		/*
		Dialog(short); 01 -> untill 00 
		Confirm(); 03 
		CharLeft(short); 04 2bytes 3bytes
		Portrait(short); 05 2bytes
		Voice(?); 06 2bytes 
		PopUp(?); 07 2bytes 
		Name(short); 08 2bytes 1byte
		set(index, byte); 09 2bytes
		clear(index, byte); 0A 2bytes
		script(id); 0B 3bytes
		call(id); 0C 3bytes
		CharMid(short); 0E 2bytes 3bytes
		CharRight(short); 18 2bytes 3bytes
		ShowBox; C9 1bytes
		return; FF 0bytes
		64?; 64 3bytes
		*/
		out.push_back(findIt->second.first);
		switch (findIt->second.first)
		{
		case 0x00: {
			if (functionName == "loaddefaultvariablesv1") {
				vars["INSTANT"] = 0;
				vars["SLIDE_FROM_LEFT"] = 1;
				vars["SLIDE_FROM_RIGHT"] = 2;
				vars["SLIDE_FROM_LEFT_TO_CENTER"] = 3;
				vars["SLIDE_FROM_RIGHT_TO_CENTER"] = 4;
				vars["FADE_IN"] = 5;
				vars["SLIDE_FROM_BOTTOM"] = 6;
				vars["SLIDE_FROM_TOP"] = 7;
				vars["SLIDE_TO_LEFT"] = 8;
				vars["SLIDE_TO_ALMOST_LEFT"] = 9;
			}
		} break;
		case 0x01: {
			const std::string converted = stringToShiftJis(args.front()->str);
			out.insert(out.end(), converted.begin(), converted.end());
			out.push_back(0);
		} break; //"dialog",
		case 0x02: //"confirm2",
		case 0x03: break; //"confirm",
		case 0x05://"portrait",
		case 0x06: {
			const unsigned short id = getInt(*args[0], vars);
			out.push_back(unsigned char(id & 0xFF));
			out.push_back(unsigned char((id >> 8) & 0xFF));
		}break;//"voice",
		case 0x07: //"speechbubble",
		{
			const char boxType = getInt(*args[0], vars);
			const char arrowPosition = getInt(*args[1], vars);
			out.push_back(boxType);
			out.push_back(arrowPosition);
		}break;
		case 0x08: //"name", 
		{
			const short a = getInt(*args[0], vars);
			const unsigned char b = getInt(*args[1], vars);
			out.push_back(unsigned char(a & 0xFF));
			out.push_back(unsigned char((a >> 8) & 0xFF));
			out.push_back(b);
		} break;
		case 0x09: //"set", 
		case 0x0A: {
			const unsigned short index = getInt(*args[0], vars); 
			const unsigned short indexByte =  glm::clamp(getInt(*args[1], vars), 0, 8);
			DisaGameStateFilterExport ex{ indexByte, index };
			unsigned short& data = *(unsigned short*)&ex;
			out.push_back(unsigned char(data & 0xFF));
			out.push_back(unsigned char((data >> 8) & 0xFF));
		}break; //"clear", 
		case 0x64: { //"loadImg", 
			if (args.size() == 2) {
				const unsigned int slot = getInt(*args[0], vars);
				const unsigned int buId = std::min(9999, getInt(*args[1], vars));
				const unsigned int scriptId = slot * 10000 + buId;
				out.push_back(unsigned char(scriptId & 0xFF));
				out.push_back(unsigned char((scriptId >> 8) & 0xFF));
				out.push_back(unsigned char((scriptId >> 16) & 0xFF));
				
			}
		} break;
		case 0x0C: //"call",
		case 0x0B: { //"script"
			const unsigned int scriptId = getInt(*args[0], vars);
			out.push_back(unsigned char(scriptId & 0xFF));
			out.push_back(unsigned char((scriptId >> 8) & 0xFF));
			out.push_back(unsigned char((scriptId >> 16) & 0xFF));
		} break;
		case 0x0F:
		case 0x0D: { //"scriptcustom"
			PG_SimpleWordNamespaceValidator()(args.front()->str);
			out.insert(out.end(), args.front()->str.begin(), args.front()->str.end());
			out.push_back(0);
		} break;
		case 0x04: //"charleft",
		case 0x0E:  //"charmid",
		case 0x18: {
			const short slot = getInt(*args[0], vars);
			const unsigned int speed = getInt(*args[1], vars);
			const unsigned int animationType = glm::clamp(getInt(*args[2], vars), 0, 99);
			const unsigned int modefier = glm::clamp(getInt(*args[3], vars), 0, 255);

			const unsigned int combined = speed * 100 + animationType;

			out.push_back(unsigned char(slot & 0xFF));
			out.push_back(unsigned char((slot >> 8) & 0xFF));

			out.push_back(unsigned char(combined & 0xFF));
			out.push_back(unsigned char((combined >> 8) & 0xFF));
			out.push_back(unsigned char(modefier & 0xFF));

		} break; //"charright",
		case 0xC9: {
			out.push_back(getBool(*args[0], vars)? 0x01 : 0x00);
		}break; // "showbox", 
		case 0x65: {
			const short param1 = getInt(*args[0], vars);
			const short param2 = getInt(*args[1], vars);
			const signed char param3 = getInt(*args[2], vars);

			out.push_back(unsigned char(param1 & 0xFF));
			out.push_back(unsigned char((param1 >> 8) & 0xFF));

			out.push_back(unsigned char(param2 & 0xFF));
			out.push_back(unsigned char((param2 >> 8) & 0xFF));

			out.push_back(param3);

		}break;
		case 0x14: {
			const unsigned char param1 = getInt(*args[0], vars);
			const std::string converted = stringToShiftJis(args[1]->str);
			out.push_back(param1);
			out.insert(out.end(), converted.begin(), converted.end());
			out.push_back(0);
		} break;
		case 0xFD: break; // "menuend",
		case 0xFF: break; // "return",
		case 0xFE: break; // "Restart",
		default:
			break;
		}
	}

	//end marker
	out.push_back(0x0B);
	out.push_back(0x00);
	out.push_back(0x00);
	out.push_back(0x00);

	return out;
}

inline int toInt(char c1, char c2, char c3, char c4) {
	// Shift the characters to their respective positions and OR them together
	return (static_cast<int>(c1) << 24) & 0xFF00000 |
		(static_cast<int>(c2) << 16) & 0x00FF0000 |
		(static_cast<int>(c3) << 8) & 0x0000FF00 |
		static_cast<int>(c4) & 0x000000FF;
}
inline unsigned int toUInt(char c1, char c2, char c3, char c4) {
	// Shift the characters to their respective positions and OR them together
	return (static_cast<unsigned int>(c1) << 24) & 0xFF000000 |
		(static_cast<unsigned int>(c2) << 16) & 0x00FF0000 |
		(static_cast<unsigned int>(c3) << 8) & 0x0000FF00 |
		static_cast<unsigned int>(c4) & 0x000000FF;
}

inline short toShort(char c1, char c2) {
	// Shift the characters to their respective positions and OR them together
	return (static_cast<short>(c1) << 8) & 0xFF00 |
		static_cast<short>(c2) & 0x00FF;
}
inline unsigned short toUShort(char c1, char c2) {
	// Shift the characters to their respective positions and OR them together
	return (static_cast<unsigned short>(c1) << 8) & 0xFF00 |
		static_cast<unsigned short>(c2) & 0x00FF;
}

std::string PG_DisaTalkScript::decompile(const std::vector<unsigned char>& code) {
	if (code.empty()) return {};
	PG::STREAM::InByteArray stream((char*)code.data(), code.size());
	return decompile(&stream);
}

std::string PG_DisaTalkScript::decompile(PG::STREAM::In* codeStream) {
	std::stringstream ss;
	char opCode;

	while (*(*codeStream >> opCode)){
		auto findIt = gOpCodeToFunction.find(opCode);
		if (findIt == gOpCodeToFunction.end()) {
			throwError(0, "Unknown op code " + std::to_string(opCode) + "!");
			return {};
		}

		switch (opCode)
		{
		case 0x01: {

			std::string shiftJis;
			while (char c = codeStream->readChar()) {
				shiftJis.push_back(c);
			}

			const std::string converted = shiftJisToString(shiftJis);

			ss << findIt->second.first << "(\"" << converted << "\");\n";
		} break; //"dialog",
		case 0x02: //"confirm2",
		case 0x03:
		{
			ss << findIt->second.first << "(" << ");\n\n";
		}break; //"confirm",
		case 0x05: {
			const short id = toShort(codeStream->readChar(), codeStream->readChar());
			ss << findIt->second.first << "(" << id << ");\n";
		}break;//"portrait",
		case 0x06: {
			const unsigned short id = toShort(codeStream->readChar(), codeStream->readChar());
			ss << findIt->second.first << "(" << id << ");\n";
		}break;//"voice",
		case 0x07: //"speechbubble",
		{
			const char boxType = codeStream->readChar();
			const char arrowPosition = codeStream->readChar();
			ss << findIt->second.first << "(" << (int)boxType<< ", "<< (int)arrowPosition << ");\n";
		}break;
		case 0x08: //"name", 
		{
			short id = toShort(codeStream->readChar(), codeStream->readChar());
			const char position = codeStream->readChar();

			ss << findIt->second.first << "(" << id << ", " << (int)position << ");\n";
		} break;
		case 0x09: //"set", 
		case 0x0A: {
			DisaGameStateFilterExport ex{ 0, 0 };
			unsigned short& data = *(unsigned short*)&ex;
			data = toUShort(codeStream->readChar(), codeStream->readChar());

			ss << findIt->second.first << "(" << ex.index << ", " << ex.bitIndex << ");\n";
		}break; //"clear", 
		case 0x64: { //"loadImg", 
			const unsigned int id = toUInt(0, codeStream->readChar(), codeStream->readChar(), codeStream->readChar());
			ss << findIt->second.first << "(" << (id / 10000) << ", " << (id % 10000) << ");\n";
		} break;
		case 0x0C: //"call",
		case 0x0B: {
			const unsigned int id = toUInt(0, codeStream->readChar(), codeStream->readChar(), codeStream->readChar());
			ss << findIt->second.first << "(" << id << ");\n";
		}break; //"script",
		case 0x14: {
			const unsigned char entries = codeStream->readChar();
			std::string shiftJis;
			while (char c = codeStream->readChar()) {
				shiftJis.push_back(c);
			}
			const std::string converted = shiftJisToString(shiftJis);
			ss << (entries == 1? findIt->second.first: "menuStart");
			ss << "(" << (int)entries <<", \"" << converted << "\");\n";
		}break; //"menuEntry",

		case 0xFD: {
			ss << findIt->second.first << "(" << ");\n\n";
		} break;//"menuEND",
		case 0x04: //"charleft",
		case 0x0E:  //"charmid",
		case 0x18: {
			const short slot = toShort(codeStream->readChar(), codeStream->readChar());
			const unsigned short combined = toShort(codeStream->readChar(), codeStream->readChar());
			const unsigned int modefier = codeStream->readChar();

			ss << findIt->second.first << "(" << slot << ", " << (combined / 100)<<", "<< (combined % 100) << ", "<< modefier << ");\n";
		} break; //"charright",
		case 0xC9: {
			const bool b = codeStream->readChar() > 0;

			ss << findIt->second.first << "(";
			ss << (b ? "true" : "false");
			ss << ");\n";

		}break; // "showbox", 
		case 0xFF: ss << "return;\n"; break; // "return",
		case 0xFE: ss << "ffe;\n"; break; // "return",
		default:
			break;
		};
	}
	

	return ss.str();
}

std::vector<std::string> PG_DisaTalkScript::findStrings(PG::STREAM::In* codeStream) throw(PG_CompileException) {
	std::vector<std::string> out;
	char opCode;

	while (*(*codeStream >> opCode)) {
		auto findIt = gOpCodeToFunction.find(opCode);
		if (findIt == gOpCodeToFunction.end()) {
			throwError(0, "Unknown op code " + std::to_string(opCode) + "!");
			return {};
		}

		switch (opCode)
		{
		case (char)0x01: {

			std::string shiftJis;
			while (char c = codeStream->readChar()) {
				shiftJis.push_back(c);
			}
			out.push_back(std::move(shiftJis));
		} break; //"dialog",
		case (char)0x02: //"confirm2",
		case (char)0x03:
		{
		}break; //"confirm",
		case (char)0x05: {
			const short id = toShort(codeStream->readChar(), codeStream->readChar());
		}break;//"portrait",
		case (char)0x06: {
			const unsigned short id = toShort(codeStream->readChar(), codeStream->readChar());
		}break;//"voice",
		case (char)0x07: //"speechbubble",
		{
			const char boxType = codeStream->readChar();
			const char arrowPosition = codeStream->readChar();
		}break;
		case (char)0x08: //"name", 
		{
			short id = toShort(codeStream->readChar(), codeStream->readChar());
			const char position = codeStream->readChar();
		} break;
		case (char)0x09: //"set", 
		case (char)0x0A: {
			DisaGameStateFilterExport ex{ 0, 0 };
			unsigned short& data = *(unsigned short*)&ex;
			data = toUShort(codeStream->readChar(), codeStream->readChar());
		}break; //"clear", 
		case (char)0x64: { //"loadImg", 
			const unsigned int id = toUInt(0, codeStream->readChar(), codeStream->readChar(), codeStream->readChar());
		} break;
		case (char)0x0C: //"call",
		case (char)0x0B: {
			const unsigned int id = toUInt(0, codeStream->readChar(), codeStream->readChar(), codeStream->readChar());
		}break; //"script",
		case (char)0x14: {
			const unsigned char entries = codeStream->readChar();
			std::string shiftJis;
			while (char c = codeStream->readChar()) {
				shiftJis.push_back(c);
			}
			out.push_back(std::move(shiftJis));
		}break; //"menuEntry",
		case (char)0xFD: break; //"menuEnd",
		case (char)0x04: //"charleft",
		case (char)0x0E:  //"charmid",
		case (char)0x18: {
			const short slot = toShort(codeStream->readChar(), codeStream->readChar());
			const unsigned short combined = toShort(codeStream->readChar(), codeStream->readChar());
			const unsigned int modefier = codeStream->readChar();
		} break; //"charright",
		case (char)0xC9: {
			const bool b = codeStream->readChar() > 0;

		}break; // "showbox", 
		case (char)0xFF: // "return",
		case (char)0xFE: return out; // "return",
		default:
			auto i = 0;
			break;
		};
	}


	return out;
}