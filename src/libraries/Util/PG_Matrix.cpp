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
#include <Util/PG_Matrix.h>

namespace PG {
namespace UTIL {

template<>
void tMatrix4x4<char>::dump(std::ostream& o) const{
	o 		<<"["<<std::setw(4)<< (int)mat[0][0]<<", "<<std::setw(4)<< (int)mat[1][0]<<", "<<std::setw(4)<< (int)mat[2][0]<<", "<<std::setw(4)<< (int)mat[3][0]<<",\n"
			<<" "<<std::setw(4)<< (int)mat[0][1]<<", "<<std::setw(4)<< (int)mat[1][1]<<", "<<std::setw(4)<< (int)mat[2][1]<<", "<<std::setw(4)<< (int)mat[3][1]<<",\n"
			<<" "<<std::setw(4)<< (int)mat[0][2]<<", "<<std::setw(4)<< (int)mat[1][2]<<", "<<std::setw(4)<< (int)mat[2][2]<<", "<<std::setw(4)<< (int)mat[3][2]<<",\n"
			<<" "<<std::setw(4)<< (int)mat[0][3]<<", "<<std::setw(4)<< (int)mat[1][3]<<", "<<std::setw(4)<< (int)mat[2][3]<<", "<<std::setw(4)<< (int)mat[3][3]<<"  ]";
}

template<>
void tMatrix4x4<unsigned char>::dump(std::ostream& o) const{
	o 		<<"["<<std::setw(4)<< (int)mat[0][0]<<", "<<std::setw(4)<< (int)mat[1][0]<<", "<<std::setw(4)<< (int)mat[2][0]<<", "<<std::setw(4)<< (int)mat[3][0]<<",\n"
			<<" "<<std::setw(4)<< (int)mat[0][1]<<", "<<std::setw(4)<< (int)mat[1][1]<<", "<<std::setw(4)<< (int)mat[2][1]<<", "<<std::setw(4)<< (int)mat[3][1]<<",\n"
			<<" "<<std::setw(4)<< (int)mat[0][2]<<", "<<std::setw(4)<< (int)mat[1][2]<<", "<<std::setw(4)<< (int)mat[2][2]<<", "<<std::setw(4)<< (int)mat[3][2]<<",\n"
			<<" "<<std::setw(4)<< (int)mat[0][3]<<", "<<std::setw(4)<< (int)mat[1][3]<<", "<<std::setw(4)<< (int)mat[2][3]<<", "<<std::setw(4)<< (int)mat[3][3]<<"  ]";
}

} /* namespace UTIL */
} /* namespace PG */
