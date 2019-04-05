/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2016 ProgSys
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#ifndef INCLUDE_PG_ENGINE_UTIL_PG_EXCEPTION_H_
#define INCLUDE_PG_ENGINE_UTIL_PG_EXCEPTION_H_

#include <iostream>
#include <exception>
#include <sstream>
#include <Util/PG_Base.h>


using namespace std;

namespace PG {
namespace UTIL {
class Exception: public exception {
public:
	Exception(const string& msg):m_msg(msg){

	}

	Exception(const string& msg, const char *file, int line){
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

class AssertException: public Exception{
public:
	AssertException(const string& msg):Exception(msg){}

	AssertException(const string& msg, const char *file, int line):
		Exception(msg,file,line){}
};

#define throw_Exception(arg) throw PG::UTIL::Exception(arg, FUNCTION_NAME, __LINE__)

inline void assertTest(std::string text, bool condition,const char *file, int line){
	if (condition) {
		std::cerr << "["<<file <<":"<<line<<"][FAIL]: "<<text<<std::endl;
		throw PG::UTIL::AssertException(text, file, line);
	}
}

#ifdef DEBUG
#define assert_Test(text, condition) PG::UTIL::assertTest(text,condition,FUNCTION_NAME,__LINE__)
#else
#define assert_Test(text, condition)
#endif

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_ENGINE_UTIL_PG_EXCEPTION_H_ */
