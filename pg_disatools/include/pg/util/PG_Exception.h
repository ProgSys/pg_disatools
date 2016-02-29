/*
 * A custom exception and assert.
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


#ifndef INCLUDE_PG_ENGINE_UTIL_PG_EXCEPTION_H_
#define INCLUDE_PG_ENGINE_UTIL_PG_EXCEPTION_H_

#include <iostream>
#include <exception>
#include <sstream>
#include <pg/util/PG_Base.h>


using namespace std;

namespace PG {
namespace UTIL {
class Exception: public exception {
public:
	Exception(string msg):m_msg(msg){

	}

	Exception(string msg, const char *file, int line){
		 std::stringstream o;
		 o<< "["<< file << ":" << line<<"][EXCEPTION]: " << msg;
		 m_msg = o.str();
	}

	const char * what () const throw (){
	    return m_msg.c_str();
	}
private:
	string m_msg;
};

#define throw_Exception(arg) throw PG::UTIL::Exception(arg, FUNCTION_NAME, __LINE__)

inline void assertTest(std::string text, bool condition,const char *file, int line){
	if (condition) {
		std::cerr << "["<<file <<":"<<line<<"][FAIL]: "<<text<<std::endl;
		throw PG::UTIL::Exception(text, file, line);
	}
}

#define assert_Test(text, condition) PG::UTIL::assertTest(text,condition,FUNCTION_NAME,__LINE__)

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_ENGINE_UTIL_PG_EXCEPTION_H_ */
