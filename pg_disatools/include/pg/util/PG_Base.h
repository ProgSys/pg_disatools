/*
 * Some definitions.
 *
 *  Copyright (C) 2016  ProgSys
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_PG_UTIL_PG_BASE_H_
#define INCLUDE_PG_UTIL_PG_BASE_H_

#include <iostream>
/// Colors
#define FONT_RESET "\033[0m"
#define FONT_RED "\033[30m"
#define FONT_GREEN "\033[32m"
#define FONT_YELLOW "\033[33m"
#define FONT_BLUE "\033[34m"

#define FONT_BOLDRED "\033[1m\033[31m"

//Set true false
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#if ( defined(_MSC_VER) && !defined(__INTEL_COMPILER) )
	#define FUNCTION_NAME __FUNCTION__
#elif __MINGW32__
	#define FUNCTION_NAME __PRETTY_FUNCTION__
#elif __linux__
	#define FUNCTION_NAME __PRETTY_FUNCTION__
#else
	#define FUNCTION_NAME "noName"
#endif//
/// Used to print, so you can easily find it in code.
#define INFO_HEADER "["<<FUNCTION_NAME<<":"<<__LINE__<<"]: "
#define INFO_HEADER_INFO "["<<FUNCTION_NAME <<":"<<__LINE__<<"][INFO]: "
#define INFO_HEADER_EXEPT "["<<FUNCTION_NAME <<":"<<__LINE__<<"][EXEPT]: "
#define INFO_HEADER_DEBUG "["<<FUNCTION_NAME <<":"<<__LINE__<<"][DEBUG]: "
#define INFO_HEADER_SUCCESS "["<<FUNCTION_NAME <<":"<<__LINE__<<"][SUCCESS]: "
#define INFO_HEADER_ERROR "["<<FUNCTION_NAME <<":"<<__LINE__<<"][ERROR]: "
#define INFO_HEADER_FATAL_ERROR "["<<FUNCTION_NAME <<":"<<__LINE__<<"][FATAL ERROR]: "
#define INFO_HEADER_EXEPT_ERROR "["<<FUNCTION_NAME <<":"<<__LINE__<<"][EXCEPTION]: "
#define INFO_HEADER_WARN "["<<FUNCTION_NAME <<":"<<__LINE__<<"][WARN]: "

//LINIX
#if FALSE
#define PG_INFO_STREAM(x) std::cout<<INFO_HEADER_INFO<<x<<std::endl;
#define PG_DEBUG_STREAM(x) std::cout<<FONT_BLUE<<INFO_HEADER_DEBUG<<x<<FONT_RESET<<std::endl;
#define PG_SUCCESS_STREAM(x) std::cout<<FONT_GREEN<<INFO_HEADER_SUCCESS<<x<<FONT_RESET<<std::endl;
#define PG_ERROR_STREAM(x) std::cout<<FONT_RED<<INFO_HEADER_ERROR<<x<<FONT_RESET<<std::endl;
#define PG_FATAL_ERROR_STREAM(x) std::cout<<FONT_BOLDRED<<INFO_HEADER_FATAL_ERROR<<x<<FONT_RESET<<std::endl;
#define PG_WARN_STREAM(x) std::cout<<FONT_YELLOW<<INFO_HEADER_WARN<<x<<FONT_RESET<<std::endl;
#else
#define PG_INFO_STREAM(x) std::cout<<INFO_HEADER_INFO<<x<<std::endl;
#define PG_DEBUG_STREAM(x)
//#define PG_DEBUG_STREAM(x) std::cout<<INFO_HEADER_DEBUG<<x<<std::endl;
#define PG_SUCCESS_STREAM(x)
//#define PG_SUCCESS_STREAM(x) std::cout<<INFO_HEADER_SUCCESS<<x<<std::endl;
#define PG_ERROR_STREAM(x) std::cerr<<INFO_HEADER_ERROR<<x<<std::endl;
#define PG_FATAL_ERROR_STREAM(x) std::cerr<<INFO_HEADER_FATAL_ERROR<<x<<std::endl;
#define PG_WARN_STREAM(x) std::cout<<INFO_HEADER_WARN<<x<<std::endl;
#endif

#define PG_OUTVEC2(s) "("<<s.x<<", "<<s.y<<")"
#define PG_OUTVEC2v(x,y) "("<<x<<", "<<y<<")"
#define PG_OUTVEC3(s) "("<<s.x<<", "<<s.y<<", "<<s.z<<")"
#define PG_OUTVEC3v(x,y,z) "("<<x<<", "<<y<<", "<<z<<")"
#define PG_OUTVEC4(s) "("<<s.x<<", "<<s.y<<", "<<s.z<<", "<<s.w<<")"
#define PG_OUTVEC4v(x,y,z,w) "("<<x<<", "<<y<<", "<<z<<", "<<w<<")"
#define PG_OUTMAT2(mat) "[("<<mat[0][0]<<", "<<mat[0][1]<<")"<<", "<<"("<<mat[1][0]<<", "<<mat[1][1]<<")"<<", "<<"("<<mat[2][0]<<", "<<mat[2][1]<<")"<<", "<<"("<<mat[3][0]<<", "<<mat[3][1]<<")]"
#define PG_OUTMAT3(mat) "[("<<mat[0][0]<<", "<<mat[0][1]<<", "<<mat[0][2]<<", "<<")"<<", "<<"("<<mat[1][0]<<", "<<mat[1][1]<<", "<<mat[1][2]<<", "<<")"<<", "<<"("<<mat[2][0]<<", "<<mat[2][1]<<", "<<mat[2][2]<<", "<<")"<<", "<<"("<<mat[3][0]<<", "<<mat[3][1]<<", "<<mat[3][2]<<", "<<")]"
#define PG_OUTMAT4(mat) "[("<<mat[0][0]<<", "<<mat[0][1]<<", "<<mat[0][2]<<", "<<mat[0][3]<<")"<<", "<<"("<<mat[1][0]<<", "<<mat[1][1]<<", "<<mat[1][2]<<", "<<mat[1][3]<<")"<<", "<<"("<<mat[2][0]<<", "<<mat[2][1]<<", "<<mat[2][2]<<", "<<mat[2][3]<<")"<<", "<<"("<<mat[3][0]<<", "<<mat[3][1]<<", "<<mat[3][2]<<", "<<mat[3][3]<<")]"

#define PG_MARK PG_INFO_STREAM("___HERE!___")


#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

namespace PG {
namespace UTIL {


} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_BASE_H_ */
