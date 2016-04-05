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


#include <pg/util/PG_Vector.h>

namespace PG {
namespace UTIL {

template<>
void tVector2<char>::dump(std::ostream& o) const{
	o<<"("<<(int)x<<", "<<(int)y<<")";
}

template<>
void tVector2<unsigned char>::dump(std::ostream& o) const{
	o<<"("<<(int)x<<", "<<(int)y<<")";
}

template<>
void tVector2<float>::dump(std::ostream& o) const{
	o<<"("<<x<<", "<<y<<")";
}

template<>
void tVector2<double>::dump(std::ostream& o) const{
	o<<"("<<x<<", "<<y<<")";
}

/////// tVector3 ///////

template<typename T>
void tVector3<T>::dump(std::ostream& o) const{
	o<<"("<<x<<", "<<y<<", "<<z<<")";
}

template<>
void tVector3<char>::dump(std::ostream& o) const{
	o<<"("<<(int)x<<", "<<(int)y<<", "<<(int)z<<")";
}

template<>
void tVector3<unsigned char>::dump(std::ostream& o) const{
	o<<"("<<(int)x<<", "<<(int)y<<", "<<(int)z<<")";
}

template<>
void tVector3<float>::dump(std::ostream& o) const{
	o<<"("<<x<<", "<<y<<", "<<z<<")";
}

template<>
void tVector3<double>::dump(std::ostream& o) const{
	o<<"("<<x<<", "<<y<<", "<<z<<")";
}

/////// tVector4 ///////

template<>
void tVector4<int>::dump(std::ostream& o) const{
	o<<"("<<x<<", "<<y<<", "<<z<<", "<<w<<")";
}

template<>
void tVector4<char>::dump(std::ostream& o) const{
	o<<"("<<(int)x<<", "<<(int)y<<", "<<(int)z<<", "<<(int)w<<")";
}


template<typename T>
void tVector4<T>::dump(std::ostream& o) const{
	o<<"("<<x<<", "<<y<<", "<<z<<", "<<w<<")";
}

template<>
void tVector4<unsigned char>::dump(std::ostream& o) const{
	o<<"("<<(int)x<<", "<<(int)y<<", "<<(int)z<<", "<<(int)w<<")";
}

template<>
void tVector4<float>::dump(std::ostream& o) const{
	o<<"("<<x<<", "<<y<<", "<<z<<", "<<w<<")";
}

template<>
void tVector4<double>::dump(std::ostream& o) const{
	o<<"("<<x<<", "<<y<<", "<<z<<", "<<w<<")";
}

} /* namespace UTIL */
} /* namespace PG */
