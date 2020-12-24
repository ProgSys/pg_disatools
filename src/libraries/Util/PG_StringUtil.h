/*
 * PG_StringUtil.h
 *
 *  Created on: 01.03.2016
 *      Author: ProgSys
 */

#pragma once

#include <string>
#include <algorithm>
#ifdef _MSC_VER
#include <functional>
#include <cctype>
#endif

namespace PG {
namespace UTIL {


// trim from start
inline std::string& ltrim(std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c); }));
        return s;
}

// trim from end
inline std::string& rtrim(std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c); }).base(), s.end());
        return s;
}

// trim from both ends
inline std::string& trim(std::string &s) {
        return ltrim(rtrim(s));
}

} /* namespace UTIL */
} /* namespace PG */

