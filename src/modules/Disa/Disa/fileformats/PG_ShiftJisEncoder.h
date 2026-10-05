/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2024  ProgSys
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
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <optional>
/**
* @brief Will convert a Shift Jis string to an utf-8 string
*/
std::string shiftJisToUtf8(const std::string& shiftJs, const std::function<uint32_t(unsigned short)>& remap = {});
/**
* @brief Will convert a utf-8 string to Shift Jis string
* @param unknownMapping is called when there is no mapping to shift jis
*/
std::string utf8ToShiftJis(const std::string& utf8, bool fullSize, const std::function<unsigned short(uint32_t)>& unknownMapping = {});
/**
* @brief Will convert a Shift Jis string to an std::string
* Some chars may get lost
*/
std::string shiftJisToString(const std::string& shiftJs);
/**
* @brief Will convert FULL Shift Jis to HALF shift jis
*/
void shiftJisToHalf(unsigned char* shiftJs);
/**
* @brief Will convert a std string to an Shift Jis string 
* This will try to use FULLWIDTH LATIN 
*/
std::string stringToShiftJis(const std::string& str);
/**
* @brief Will convert a std string to an Shift Jis string
* This will use only use "Half characters"
*/
std::string stringToShiftJisHalf(const std::string& str);
/**
* @brief Will split shift js string by empty spaces
* Usefull for displaying the text inside a textbox 
*/
std::vector<std::string> splitShiftJisBySpace(const std::string& str);
/**
* @brief Will reduce the size of given string to given size
*/
std::string splitShiftJisClampToSize(const std::string& str, unsigned int size);
/**
* @brief Will find the offset of the char in string with given index
* A char in ShiftJis is either one byte or two byte
* @param index the index of the char of which to find the byte offset
* @param startOffset starting byte offset form where to start the search from (should be used with care)
*/
unsigned int indexToShiftJisOffset(const std::string& str, unsigned int index, unsigned int startOffset = 0);
/**
* @brief Finds the length of the given zero terminated shift jis string
*/
size_t getShiftJisLength(char* shiftJis);
/**
* @brief Finds the length of the given uft8 string
*/
size_t getUtf8Length(const std::string& string);
/**
* @brief Resize the Utf8 string to given size
* Will only apply if string is bigger then targetSize
*/
void resizeUtf8(std::string& string, size_t targetSize);
/**
* @brief Helper function to copy an array into a string
* Simmilar to strncpy 
*/
template<size_t Size>
std::string toShiftJisString(const std::array<char, Size>& data) {
    std::string str;
    str.reserve(Size);
    for (auto c : data) {
        if (c == 0) break;
        str.push_back(c);
    }
    return str;
}

/**
* @brief Will split given utf8 string by given simple 1 byte character
*/
std::vector<std::string> splitUtf8Simple(const std::string& string, const char splitter);

/**
* @brief Parses a Unicode code point given as "U+2665" and returns it UTF-8 encoded
* Returns std::nullopt if the format or code point is invalid
*/
std::optional<std::string> parseCodePointToUtf8(const std::string& str);