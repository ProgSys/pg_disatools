#include "PG_ShiftJisEncoder.h"
#include "PG_ShiftJisToUtf8LookUpTable.h"

std::string shiftJisToUtf8(const std::string& shiftJs) {
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
        unsigned long long value = shiftJisToUtf8(doubleByte);
		for (int i = 0; i < sizeof(unsigned long long) / sizeof(char); ++i) {
            char v  = static_cast<char>(value);
			if (v == 0) break;
			part.push_back(v);
			value = value >> 8;
        }
        output.insert(output.end(), part.rbegin(), part.rend());

	}

	return output;
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