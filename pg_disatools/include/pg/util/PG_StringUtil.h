/*
 * PG_StringUtil.h
 *
 *  Created on: 01.03.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_UTIL_PG_STRINGUTIL_H_
#define INCLUDE_PG_UTIL_PG_STRINGUTIL_H_

#include <string>
#include <algorithm>

namespace PG {
namespace UTIL {


// trim from start
inline std::string& ltrim(std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
inline std::string& rtrim(std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
inline std::string& trim(std::string &s) {
        return ltrim(rtrim(s));
}

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_STRINGUTIL_H_ */
