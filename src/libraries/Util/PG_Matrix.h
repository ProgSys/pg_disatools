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
#ifndef INCLUDE_PG_UTIL_PG_MATRIX_H_
#define INCLUDE_PG_UTIL_PG_MATRIX_H_

#include <ostream>
#include <iomanip>
#include <Util/PG_Vector.h>
#include <Util/PG_Exception.h>

namespace PG {
namespace UTIL {

/*
 * column-major
 *
 * v0 v1 v2  v3
 * -------------
 * m0 m4 m8  m12
 * m1 m5 m9  m13
 * m2 m6 m10 m14
 * m3 m7 m11 m15
 */

template<typename T>
struct tMatrix4x4 {
public:
	tVector4<T> mat[4];

	tMatrix4x4();
	tMatrix4x4(const T& setVal);
	tMatrix4x4(const tMatrix4x4<T>& matrix);
	tMatrix4x4(
			T m0,      T m1,      T m2  = 0, T m3  = 0,
			T m4  = 0, T m5  = 0, T m6  = 0, T m7  = 0,
			T m8  = 0, T m9  = 0, T m10 = 0, T m11 = 0,
			T m12 = 0, T m13 = 0, T m14 = 0, T m15 = 0
			);

	unsigned int length() const;
	unsigned int width() const;
	unsigned int height() const;
	tVector4<T>& operator[](unsigned int index);
	tVector4<T> const& operator[](unsigned int index) const;

	void operator=(const tMatrix4x4<T>& tmat);

	tMatrix4x4<T> operator*(const tMatrix4x4<T>& tmat) const;
	tVector4<T> operator*(const tVector4<T>& tvec) const;

	void dump(std::ostream& o) const;

	friend std::ostream& operator<<(std::ostream& o,const tMatrix4x4<T>& tmat){
		tmat.dump(o);
		return o;
	}

	//virtual ~tMatrix4x4();

};

template<typename T>
tMatrix4x4<T>::tMatrix4x4(){
	for(unsigned int i = 1; i < 4*4; i++)
	 (&mat[0].x)[i] = 0;

	mat[0][0] = 1;
	mat[1][1] = 1;
	mat[2][2] = 1;
	mat[3][3] = 1;
}

template<typename T>
tMatrix4x4<T>::tMatrix4x4(const T& setVal){
	for(unsigned int i = 0; i < 4*4; i++)
	 (&mat[0].x)[i] = setVal;
}

template<typename T>
tMatrix4x4<T>::tMatrix4x4(const tMatrix4x4<T>& matrix){
	mat[0] = matrix.mat[0];
	mat[1] = matrix.mat[1];
	mat[2] = matrix.mat[2];
	mat[3] = matrix.mat[3];
}

template<typename T>
tMatrix4x4<T>::tMatrix4x4(
		T m0,  T m1,  T m2,  T m3,
		T m4,  T m5,  T m6,  T m7,
		T m8,  T m9,  T m10, T m11,
		T m12, T m13, T m14, T m15
		){

	T* pttr = (&mat[0].x);
	 pttr[0] = m0;
	 pttr[1] = m1;
	 pttr[2] = m2;
	 pttr[3] = m3;

	 pttr[4] = m4;
	 pttr[5] = m5;
	 pttr[6] = m6;
	 pttr[7] = m7;

	 pttr[8] = m8;
	 pttr[9] = m9;
	 pttr[10] = m10;
	 pttr[11] = m11;

	 pttr[12] = m12;
	 pttr[13] = m13;
	 pttr[14] = m14;
	 pttr[15] = m15;
}

template<typename T>
unsigned int tMatrix4x4<T>::length() const{
	return 4*4;
}

template<typename T>
unsigned int tMatrix4x4<T>::width() const{
	return 4;
}

template<typename T>
unsigned int tMatrix4x4<T>::height() const{
	return 4;
}

template<typename T>
tVector4<T>& tMatrix4x4<T>::operator[](unsigned int index){
	assert_Test("Out of bounds!", (index >= width()) );
	return mat[index];
}

template<typename T>
tVector4<T> const& tMatrix4x4<T>::operator[](unsigned int index) const{
	assert_Test("Out of bounds!", (index >= width()) );
	return mat[index];
}

template<typename T>
void tMatrix4x4<T>::operator=(const tMatrix4x4<T>& tmat){
	mat[0] = tmat.mat[0];
	mat[1] = tmat.mat[1];
	mat[2] = tmat.mat[2];
	mat[3] = tmat.mat[3];
}

template<typename T>
tMatrix4x4<T> tMatrix4x4<T>::operator*(const tMatrix4x4<T>& b) const{
	return tMatrix4x4<T>(
			mat[0][0]*b[0][0] + mat[1][0]*b[0][1] + mat[2][0]*b[0][2] +mat[3][0]*b[0][3],
			mat[0][1]*b[0][0] + mat[1][1]*b[0][1] + mat[2][1]*b[0][2] +mat[3][1]*b[0][3],
			mat[0][2]*b[0][0] + mat[1][2]*b[0][1] + mat[2][2]*b[0][2] +mat[3][2]*b[0][3],
			mat[0][3]*b[0][0] + mat[1][3]*b[0][1] + mat[2][3]*b[0][2] +mat[3][3]*b[0][3],

			mat[0][0]*b[1][0] + mat[1][0]*b[1][1] + mat[2][0]*b[1][2] +mat[3][0]*b[1][3],
			mat[0][1]*b[1][0] + mat[1][1]*b[1][1] + mat[2][1]*b[1][2] +mat[3][1]*b[1][3],
			mat[0][2]*b[1][0] + mat[1][2]*b[1][1] + mat[2][2]*b[1][2] +mat[3][2]*b[1][3],
			mat[0][3]*b[1][0] + mat[1][3]*b[1][1] + mat[2][3]*b[1][2] +mat[3][3]*b[1][3],

			mat[0][0]*b[2][0] + mat[1][0]*b[2][1] + mat[2][0]*b[2][2] +mat[3][0]*b[2][3],
			mat[0][1]*b[2][0] + mat[1][1]*b[2][1] + mat[2][1]*b[2][2] +mat[3][1]*b[2][3],
			mat[0][2]*b[2][0] + mat[1][2]*b[2][1] + mat[2][2]*b[2][2] +mat[3][2]*b[2][3],
			mat[0][3]*b[2][0] + mat[1][3]*b[2][1] + mat[2][3]*b[2][2] +mat[3][3]*b[2][3],

			mat[0][0]*b[3][0] + mat[1][0]*b[3][1] + mat[2][0]*b[3][2] +mat[3][0]*b[3][3],
			mat[0][1]*b[3][0] + mat[1][1]*b[3][1] + mat[2][1]*b[3][2] +mat[3][1]*b[3][3],
			mat[0][2]*b[3][0] + mat[1][2]*b[3][1] + mat[2][2]*b[3][2] +mat[3][2]*b[3][3],
			mat[0][3]*b[3][0] + mat[1][3]*b[3][1] + mat[2][3]*b[3][2] +mat[3][3]*b[3][3]
	);
}

template<typename T>
tVector4<T> tMatrix4x4<T>::operator*(const tVector4<T>& tvec) const{
	return tVector4<T>(
			mat[0][0]*tvec[0] + mat[1][0]*tvec[1] + mat[2][0]*tvec[2] +mat[3][0]*tvec[3],
			mat[0][1]*tvec[0] + mat[1][1]*tvec[1] + mat[2][1]*tvec[2] +mat[3][1]*tvec[3],
			mat[0][2]*tvec[0] + mat[1][2]*tvec[1] + mat[2][2]*tvec[2] +mat[3][2]*tvec[3],
			mat[0][3]*tvec[0] + mat[1][3]*tvec[1] + mat[2][3]*tvec[2] +mat[3][3]*tvec[3]
	);
}

template<typename T>
void tMatrix4x4<T>::dump(std::ostream& o) const{
	o 		<<"["<<std::setw(4)<< mat[0][0]<<", "<<std::setw(4)<< mat[1][0]<<", "<<std::setw(4)<< mat[2][0]<<", "<<std::setw(4)<< mat[3][0]<<",\n"
			<<" "<<std::setw(4)<< mat[0][1]<<", "<<std::setw(4)<< mat[1][1]<<", "<<std::setw(4)<< mat[2][1]<<", "<<std::setw(4)<< mat[3][1]<<",\n"
			<<" "<<std::setw(4)<< mat[0][2]<<", "<<std::setw(4)<< mat[1][2]<<", "<<std::setw(4)<< mat[2][2]<<", "<<std::setw(4)<< mat[3][2]<<",\n"
			<<" "<<std::setw(4)<< mat[0][3]<<", "<<std::setw(4)<< mat[1][3]<<", "<<std::setw(4)<< mat[2][3]<<", "<<std::setw(4)<< mat[3][3]<<"  ]";
}

typedef tMatrix4x4<float> mat4;

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_MATRIX_H_ */
