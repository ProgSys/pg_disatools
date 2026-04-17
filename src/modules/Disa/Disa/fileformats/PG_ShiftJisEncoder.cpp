#include "PG_ShiftJisEncoder.h"
#include "PG_ShiftJisToUtf8LookUpTable.h"
#include "PG_Utf8ToShiftJisLookUpTable.h"
#include <map>

static std::map<char, unsigned short> g_toFullWidth = {
	{' ', 0x8140},
	{'!', 0x8149},
	{'"', 0x8156},
	{'#', 0x8194},
	{'$', 0x8190},
	{'%', 0x8193},
	{'(', 0x8169},
	{')', 0x816A},
	{'*', 0x8196},
	{'+', 0x817B},
	{',', 0x8143},
	{'-', 0x817C},
	{'.', 0x8144},
	{'/', 0x8196},
	{'0', 0x824F},
	{'1', 0x8250},
	{'2', 0x8251},
	{'3', 0x8252},
	{'4', 0x8253},
	{'5', 0x8254},
	{'6', 0x8255},
	{'7', 0x8256},
	{'8', 0x8257},
	{'9', 0x8258},

	{':', 0x8146},
	{';', 0x8147},
	{'<', 0x8171},
	{'=', 0x8181},
	{'>', 0x8172},
	{'?', 0x8148},

	{'@', 0x8197},

	{'A', 0x8260},
	{'B', 0x8261},
	{'C', 0x8262},
	{'D', 0x8263},
	{'E', 0x8264},
	{'F', 0x8265},
	{'G', 0x8266},
	{'H', 0x8267},
	{'I', 0x8268},
	{'J', 0x8269},
	{'K', 0x826A},
	{'L', 0x826B},
	{'M', 0x826C},
	{'N', 0x826D},
	{'O', 0x826E},
	{'P', 0x826F},
	{'Q', 0x8270},
	{'R', 0x8271},
	{'S', 0x8272},
	{'T', 0x8273},
	{'U', 0x8274},
	{'V', 0x8275},
	{'W', 0x8276},
	{'X', 0x8277},
	{'Y', 0x8278},
	{'Z', 0x8279},

	{'[', 0x826D},
	{'\\', 0x825F},
	{']', 0x826E},
	{'^', 0x824F},
	{'_', 0x8251},

	{'`', 0x824D},

	{'a', 0x8281},
	{'b', 0x8282},
	{'c', 0x8283},
	{'d', 0x8284},
	{'e', 0x8285},
	{'f', 0x8286},
	{'g', 0x8287},
	{'h', 0x8288},
	{'i', 0x8289},
	{'j', 0x828A},
	{'k', 0x828B},
	{'l', 0x828C},
	{'m', 0x828D},
	{'n', 0x828E},
	{'o', 0x828F},
	{'p', 0x8290},
	{'q', 0x8291},
	{'r', 0x8292},
	{'s', 0x8293},
	{'t', 0x8294},
	{'u', 0x8295},
	{'v', 0x8296},
	{'w', 0x8297},
	{'x', 0x8298},
	{'y', 0x8299},
	{'z', 0x829A},

	{'{', 0x826F},
	{'|', 0x8262},
	{'}', 0x8270},
	{'~', 0x8260},
};

static std::map<unsigned short, char> g_toHalfWidth = {
	{0x8140, ' '},
	{0x8149, '!'},
	{0x8156, '"'},
	{0x8194, '#'},
	{0x8190, '$'},
	{0x8193, '%'},
	{0x8169, '('},
	{0x816A, ')'},
	{0x8196, '*'},
	{0x817B, '+'},
	{0x8143, ','},
	{0x817C, '-'},
	{0x8144, '.'},
	{0x8196, '/'},
	{0x824F, '0'},
	{0x8250, '1'},
	{0x8251, '2'},
	{0x8252, '3'},
	{0x8253, '4'},
	{0x8254, '5'},
	{0x8255, '6'},
	{0x8256, '7'},
	{0x8257, '8'},
	{0x8258, '9'},

	{0x8146, ':'},
	{0x8147, ';'},
	{0x8171, '<'},
	{0x8181, '='},
	{0x8172, '>'},
	{0x8148, '?'},

	{0x8197, '@'},

	{0x8260, 'A'},
	{0x8261, 'B'},
	{0x8262, 'C'},
	{0x8263, 'D'},
	{0x8264, 'E'},
	{0x8265, 'F'},
	{0x8266, 'G'},
	{0x8267, 'H'},
	{0x8268, 'I'},
	{0x8269, 'J'},
	{0x826A, 'K'},
	{0x826B, 'L'},
	{0x826C, 'M'},
	{0x826D, 'N'},
	{0x826E, 'O'},
	{0x826F, 'P'},
	{0x8270, 'Q'},
	{0x8271, 'R'},
	{0x8272, 'S'},
	{0x8273, 'T'},
	{0x8274, 'U'},
	{0x8275, 'V'},
	{0x8276, 'W'},
	{0x8277, 'X'},
	{0x8278, 'Y'},
	{0x8279, 'Z'},

	{0x826D, '['},
	{0x825F, '\\'},
	{0x826E, ']'},
	{0x824F, '^'},
	{0x8251, '_'},

	{0x824D, '`'},

	{0x8281, 'a'},
	{0x8282, 'b'},
	{0x8283, 'c'},
	{0x8284, 'd'},
	{0x8285, 'e'},
	{0x8286, 'f'},
	{0x8287, 'g'},
	{0x8288, 'h'},
	{0x8289, 'i'},
	{0x828A, 'j'},
	{0x828B, 'k'},
	{0x828C, 'l'},
	{0x828D, 'm'},
	{0x828E, 'n'},
	{0x828F, 'o'},
	{0x8290, 'p'},
	{0x8291, 'q'},
	{0x8292, 'r'},
	{0x8293, 's'},
	{0x8294, 't'},
	{0x8295, 'u'},
	{0x8296, 'v'},
	{0x8297, 'w'},
	{0x8298, 'x'},
	{0x8299, 'y'},
	{0x829A, 'z'},

	{0x826F, '{'},
	{0x8262, '|'},
	{0x8270, '}'},
	{0x8260, '~'},
};


std::string shiftJisToUtf8(const std::string& shiftJs, const std::function<uint32_t(unsigned short)>& remap) {
	std::string output;
	std::string part;

	for (auto it = shiftJs.begin(); it != shiftJs.end(); ++it) {
		unsigned char byte = static_cast<unsigned char>(*it);
		// Single-byte character
		if (byte <= 0x7F) { 
			switch (byte) { //TODO except for a yen sign 0x5C and overline 0x7E in place
			case 0x5C: // yen sign
				output.push_back(byte); //TODO
				break;
			case 0x7E: // overline
				output.push_back(byte); //TODO
				break;
			default:
				output.push_back(byte); // matches the ASCII encoding
				break;
			}
			continue;
		}

		++it;
        if (it == shiftJs.end()) {
            //invalid encoding
            output.push_back('?');
            break;
        }
		
		part.clear();
		const unsigned short doubleByte = (byte << 8) | static_cast<unsigned char>(*it);
		uint32_t value = 0;
		if (remap) {
			value = remap(doubleByte);
			if (value) {
				for (int i = 0; i < sizeof(uint32_t) / sizeof(char); ++i) {
					char v = static_cast<char>(value);
					if (v == 0) break;
					output.push_back(v);
					value = value >> 8;
				}
				continue;
			}
		}

		value = utf8FullwidthToHalfwidth(shiftJisToUtf8(doubleByte));
		for (int i = 0; i < sizeof(uint32_t) / sizeof(char); ++i) {
            char v  = static_cast<char>(value);
			if (v == 0) break;
			part.push_back(v);
			value = value >> 8;
        }
        output.insert(output.end(), part.rbegin(), part.rend());

	}

	return output;
}



std::string utf8ToShiftJis(const std::string& utf8, bool fullSize, const std::function<unsigned short(uint32_t)>& unknownMapping) {
	std::string out;
	if (utf8.empty()) return out;

	uint32_t utf8Char;
	unsigned short shiftJisChar;
	auto it = utf8.begin();
	while (it != utf8.end()) {
		unsigned char c = *it;
		if ((c & 0x80) == 0) { // Single-byte character
			utf8Char = c;
			++it;
		}
		else if ((c & 0xE0) == 0xC0) { // Two-byte character
			if (std::distance(it, utf8.end()) < 2) break; //invalid size
			utf8Char = c;
			++it;
			utf8Char |= (static_cast<unsigned long long>(*it) << 8) & 0xFF00;
			++it;
		}
		else if ((c & 0xF0) == 0xE0) { // Three-byte character
			if (std::distance(it, utf8.end()) < 3) break; //invalid size
			utf8Char = c;
			++it;
			utf8Char |= (static_cast<unsigned long long>(*it) << 8) & 0xFF00;
			++it;
			utf8Char |= (static_cast<unsigned long long>(*it) << 16) & 0xFF0000;
			++it;
		}
		else if ((c & 0xF0) == 0xF0) { // Four-byte character
			if (std::distance(it, utf8.end()) < 4) break; //invalid size
			utf8Char = c;
			++it;
			utf8Char |= (static_cast<unsigned long long>(*it) << 8) & 0xFF00;
			++it;
			utf8Char |= (static_cast<unsigned long long>(*it) << 16) & 0xFF0000;
			++it;
			utf8Char |= (static_cast<unsigned long long>(*it) << 24) & 0xFF000000;
			++it;
		}
		else {
			return {};
		}

		shiftJisChar = utf8ToShiftJis(utf8Char);
		if (fullSize) {
			auto findIt = g_toFullWidth.find(char(shiftJisChar));
			if (findIt != g_toFullWidth.end()) shiftJisChar = findIt->second;
		}
		if (shiftJisChar == 0 && unknownMapping) {
			shiftJisChar = unknownMapping(utf8Char);
		}

		if (shiftJisChar == 0) {
			//aproximate todo maybe do it better
			shiftJisChar = 0x8260;
		}
		if (unsigned char c = shiftJisChar >> 8) {
			out.push_back(c);
		}
		out.push_back(shiftJisChar & 0x00FF);
	}

	return out;
}


void shiftJisToHalf(unsigned char* shiftJs) {
	unsigned char* write = shiftJs;
	while (*shiftJs != 0) {
		unsigned char byte = static_cast<unsigned char>(*shiftJs);
		if (byte <= 0x7F || (byte >= 0xA0 && byte <= 0xDF)) {
			*write = *shiftJs;
			++shiftJs;
			++write;
		}
		else {
			++shiftJs;
			unsigned short dChar = (byte << 8) | (*shiftJs & 0xFF);
			auto findIt = g_toHalfWidth.find(dChar);
			if (findIt != g_toHalfWidth.end()) {
				*write = findIt->second;
				++write;
				++shiftJs;
			}
			else {
				*write = byte;
				++write;
				*write = *shiftJs;
				++write;
				++shiftJs;
			}
		}
	}
	*write = 0;
}

std::string shiftJisToString(const std::string& shiftJs) {
	std::string output;

	for (auto it = shiftJs.begin(); it != shiftJs.end(); ++it) {
		unsigned char byte = static_cast<unsigned char>(*it);
		// Single-byte character
		if (byte <= 0x7F) {
			switch (byte) { //TODO except for a yen sign 0x5C and overline 0x7E in place
			case 0x5C: // yen sign
				output.push_back(byte); //TODO
				break;
			case 0x7E: // overline
				output.push_back(byte); //TODO
				break;
			default:
				output.push_back(byte); // matches the ASCII encoding
				break;
			}
			continue;
		}

		++it;
		if (it == shiftJs.end()) {
			//invalid encoding
			output.push_back('?');
			break;
		}

		const unsigned short doubleByte = (byte << 8) | static_cast<unsigned char>(*it);

		if (0x824F <= doubleByte && doubleByte <= 0x8258) { //Number
			output.push_back('0' + doubleByte - 0x824F); 
		}else if (0x8260 <= doubleByte && doubleByte <= 0x8279) { //Letter big
			output.push_back('A' + doubleByte - 0x8260);
		}else if (0x8281 <= doubleByte && doubleByte <= 0x829A) { //Letter small
			output.push_back('a' + doubleByte - 0x8281);
		}
		else if ((0x873F + 1) <= doubleByte && doubleByte <= (0x889E - 4)) { //Special
			output.push_back('<');
			output.append(std::to_string(doubleByte - 0x873F - 1));
			output.push_back('>');
		}
		else { //special
			switch (doubleByte)
			{
			case 0x8140: output.push_back(' '); break;
			case 0x8240: output.push_back(' '); break;
			case 0x8146: output.push_back(':'); break;
			case 0x8147: output.push_back(';'); break;
			case 0x8148: output.push_back('?'); break;
			case 0x8149: output.push_back('!'); break;
			case 0x8151: output.push_back('_'); break;
			case 0x8165: output.push_back('‘'); break;
			case 0x8166: output.push_back('’'); break;
			case 0x8167: output.push_back('“'); break;
			case 0x8168: output.push_back('”'); break;
			case 0x8169: output.push_back('('); break;
			case 0x816A: output.push_back(')'); break;
			case 0x816D: output.push_back('['); break;
			case 0x816E: output.push_back(']'); break;
			case 0x8197: output.push_back('@'); break;
			case 0x8193: output.push_back('%'); break;
			case 0x8196: output.push_back('*'); break;
			case 0x818F: output.push_back('¥'); break;
			case 0x8143: output.push_back(','); break;
			case 0x8144: output.push_back('.'); break;
			case 0x815C: output.push_back(196); break;
			case 0x817C: output.push_back('-'); break;
			case 0x815E: output.push_back('/'); break;
			case 0x815F: output.push_back('\\'); break;
			case 0x8183: output.push_back('<'); break;
			case 0x8184: output.push_back('>'); break;
			case 0x81E4: output.push_back('~'); break;
			case 0x814F: output.push_back('^'); break;
			default:
				//use half characters table
				output.push_back('?');
				break;
			}
		}

	}

	return output;
}

std::string stringToShiftJis(const std::string& str) {
	std::string output;
	auto push = [&output](unsigned short shiftJisChar) {
		output.push_back(static_cast<char>(shiftJisChar >> 8));
		output.push_back(static_cast<char>(shiftJisChar));
	};
	//https://www.msx.org/wiki/Shift-JIS_code
	//https://www.w3resource.com/cpp-exercises/for-loop/cpp-for-loop-exercise-61.php
	for (char c : str) {
		if (c >= '0' && c <= '9') {
			push(0x824F + (c - '0'));
		}else if (c >= 'A' && c <= 'Z') {
			push(0x8260 + (c - 'A'));
		}else if (c >= 'a' && c <= 'z') {
			push(0x8281 + (c - 'a'));
		}
		else {
			switch (c)
			{
			case ' ': push(0x8140); break;
			case ':': push(0x8146); break;
			case ';': push(0x8147); break;
			case '?': push(0x8148); break;
			case '!': push(0x8149); break;
			case '_': push(0x8151); break;
			case '‘': push(0x8165); break;
			case '’': push(0x8166); break;
			case '“': push(0x8167); break;
			case '”': push(0x8168); break;
			case '(': push(0x8169); break;
			case ')': push(0x816A); break;
			case '[': push(0x816D); break;
			case ']': push(0x816E); break;
			case '@': push(0x8197); break;
			case '%': push(0x8193); break;
			case '*': push(0x8196); break;
			case '¥': push(0x818F); break;
			case ',': push(0x8143); break;
			case '.': push(0x8144); break;
			case 196: push(0x815C); break;
			case '-': push(0x817C); break;
			case '/': push(0x815E); break;
			case '\\': push(0x815F); break;
			case '<': push(0x8183); break;
			case '>': push(0x8184); break;
			case '~': push(0x81E4); break;
			case '^': push(0x814F); break;
			default:
				//use half characters table
				output.push_back(0x20 + (c - 32));
				break;
			}
		}
	}

	return output;
}

std::string stringToShiftJisHalf(const std::string& str) {
	//https://www.msx.org/wiki/Shift-JIS_code
	std::string output;
	for (char c : str) {
		output.push_back(0x20 + (c - 32));
	}
	return output;
}

std::vector<std::string> splitShiftJisBySpace(const std::string& str) {

	std::vector<std::string> out;
	std::string current;
	auto it = str.begin();
	unsigned short c;
	do {
		c = 0;
		c = *it;
		++it;
		if (c <= 0x7F) {
			//single it
			if (c == 0x20) {
				if(!current.empty()) out.push_back(std::move(current));
				continue;
			}
			else if (c == 0x0a) {
				if (!current.empty()) {
					out.push_back(std::move(current));
					out.push_back(std::string()); //new line
				}
				continue;
			}
			current.push_back(c);
		}
		else {
			if (it == str.end()) {
				break;
			}
			c = (c << 8) | ((*it) & 0xFF);
			++it;

			if ( c == 0x8140 || c == 0x8240) {
				if (!current.empty()) out.push_back(std::move(current));
				continue;
			}
			current.push_back(static_cast<char>(c >> 8));
			current.push_back(static_cast<char>(c));
		}

	} while (it != str.end());
	if (!current.empty()) out.push_back(std::move(current));
	return out;
}


static inline std::string::const_iterator moveSteps(std::string::const_iterator it, std::string::const_iterator itEnd, unsigned int steps) {
	if (steps == 0) return it;
	unsigned char c;
	do {
		c = *it;
		if (c <= 0x7F) {
			++it;
		}
		else {
			it += 2;
		}
		--steps;
	} while (steps != 0 && it != itEnd);
	return it;
}

std::string splitShiftJisClampToSize(const std::string& str, unsigned int size) {
	if (str.empty() || size == 0) return {};
	auto it = moveSteps(str.begin(), str.end(), size);
	return { str.begin(), it };
}

unsigned int indexToShiftJisOffset(const std::string& str, unsigned int index, unsigned int startOffset ) {
	if (str.empty() || startOffset >= str.size()) return 0;

	auto it = moveSteps(std::next(str.begin(), startOffset), str.end(), index);

	return std::distance(str.begin(), it) - startOffset;
}

//see https://www.msx.org/wiki/Shift-JIS_code
size_t getShiftJisLength(char* shiftJis) {
	size_t count = 0;
	if (!shiftJis) return count;

	while (*shiftJis && count < 9000) {
		unsigned char c = *shiftJis;
		if (c <= 0x7F || (c >= 0xA0 && c <= 0xDF)) {
			++shiftJis;
		}
		else {
			shiftJis += 2;
		}
		++count;
	}

	return count;
}

size_t getUtf8Length(const std::string& string) {
	size_t count = 0;
	if (string.empty()) return count;

	auto it = string.begin();
	while (it != string.end()) {
		unsigned char c = *it;
		if ((c & 0x80) == 0) { // Single-byte character
			++it;
		}
		else if ((c & 0xE0) == 0xC0) { // Two-byte character
			if (std::distance(it, string.end()) < 2) break;
			it += 2;
		}
		else if ((c & 0xF0) == 0xE0) { // Three-byte character
			if (std::distance(it, string.end()) < 3) break;
			it += 3;
		}
		else if ((c & 0xF0) == 0xF0) { // Four-byte character
			if (std::distance(it, string.end()) < 4) break;
			it += 4;
		}
		else break;
		++count;
	}

	return count;
}

void resizeUtf8(std::string& string, size_t targetSize) {
	if (string.empty()) return;

	size_t count = 0;
	auto it = string.begin();
	while (it != string.end()) {
		unsigned char c = *it;
		if ((c & 0x80) == 0) { // Single-byte character
			++it;
		}
		else if ((c & 0xE0) == 0xC0) { // Two-byte character
			if (std::distance(it, string.end()) < 2) break;
			it += 2;
		}
		else if ((c & 0xF0) == 0xE0) { // Three-byte character
			if (std::distance(it, string.end()) < 3) break;
			it += 3;
		}
		else if ((c & 0xF0) == 0xF0) { // Four-byte character
			if (std::distance(it, string.end()) < 4) break;
			it += 4;
		}
		++count;
		if (count >= targetSize) {
			string.erase(it, string.end());
			return;
		}
	}
}

std::vector<std::string> splitUtf8Simple(const std::string& string, const char splitter) {
	std::vector<std::string> out;
	if (string.empty()) return out;
	std::string curr;

	auto it = string.begin();
	while (it != string.end()) {
		unsigned char c = *it;
		if ((c & 0x80) == 0) { // Single-byte character
			if (c == splitter) {
				out.push_back(std::move(curr));
			}
			else {
				curr.push_back(c);
			}
			++it;
		}
		else if ((c & 0xE0) == 0xC0) { // Two-byte character
			curr.push_back(c);
			if (std::distance(it, string.end()) < 2) break;
			++it;
			curr.push_back(*it);
			++it;
		}
		else if ((c & 0xF0) == 0xE0) { // Three-byte character
			curr.push_back(c);
			if (std::distance(it, string.end()) < 3) break;
			++it;
			curr.push_back(*it);
			++it;
			curr.push_back(*it);
			++it;
		}
		else if ((c & 0xF0) == 0xF0) { // Four-byte character
			curr.push_back(c);
			if (std::distance(it, string.end()) < 4) break;
			++it;
			curr.push_back(*it);
			++it;
			curr.push_back(*it);
			++it;
			curr.push_back(*it);
			++it;
		}

	}

	if(!curr.empty()) out.push_back(std::move(curr));
	return out;
}