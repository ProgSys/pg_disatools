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
/**
* @brief Will convert a Shift Jis string to an utf-8 string
*/
std::string shiftJisToUtf8(const std::string& shiftJs);
/**
* @brief Will convert a Shift Jis string to an std::string
* Some chars may get lost
*/
std::string shiftJisToString(const std::string& shiftJs);
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