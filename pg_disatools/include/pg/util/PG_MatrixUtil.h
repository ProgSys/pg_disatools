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
#ifndef INCLUDE_PG_UTIL_PG_MATRIXUTIL_H_
#define INCLUDE_PG_UTIL_PG_MATRIXUTIL_H_

#include <cmath>
#include <limits.h>
#include <pg/util/PG_Exception.h>
#include <pg/util/PG_Matrix.h>
#include <pg/util/PG_Vector.h>
#include <pg/util/PG_VectorUtil.h>

namespace PG {
namespace UTIL {

template<typename T>
T const * value_ptr(const tMatrix4x4<T>& mat){
	return &(mat[0].x);
}

template<typename T>
tMatrix4x4<T> lookAt( tVector3<T> eyePos, tVector3<T> lookAtPoint, tVector3<T> upVector){
	//build a normal base
	const tVector3<T> f(normalize(lookAtPoint - eyePos));
	const tVector3<T> s(normalize(cross(f, upVector)));
	const tVector3<T> u(cross(s, f));

	tMatrix4x4<T> viewMat;
	viewMat[0][0] = s.x;
	viewMat[1][0] = s.y;
	viewMat[2][0] = s.z;
	viewMat[0][1] = u.x;
	viewMat[1][1] = u.y;
	viewMat[2][1] = u.z;
	viewMat[0][2] =-f.x;
	viewMat[1][2] =-f.y;
	viewMat[2][2] =-f.z;
	viewMat[3][0] =-dot(s, eyePos);
	viewMat[3][1] =-dot(u, eyePos);
	viewMat[3][2] = dot(f, eyePos);
	return viewMat;
}


template<typename T>
tMatrix4x4<T> perspective(T fovy, T aspectRatio, T nearZ, T farZ){
	assert_Test("Aspect ratio is 0!", abs(aspectRatio - std::numeric_limits<T>::epsilon()) <= static_cast<T>(0));

	const T tanHalfFovy = tan(fovy / static_cast<T>(2));

	tMatrix4x4<T> perspectiveMat(static_cast<T>(0));
	perspectiveMat[0][0] = static_cast<T>(1) / (aspectRatio * tanHalfFovy);
	perspectiveMat[1][1] = static_cast<T>(1) / (tanHalfFovy);
	perspectiveMat[2][2] = - (farZ + nearZ) / (farZ - nearZ);
	perspectiveMat[2][3] = - static_cast<T>(1);
	perspectiveMat[3][2] = - (static_cast<T>(2) * farZ * nearZ) / (farZ);
	return perspectiveMat;
}

template<typename T>
tMatrix4x4<T> perspective(T fovy, int width, int height, T nearZ, T farZ){
	return perspective(fovy, width/static_cast<T>(height), nearZ, farZ);
}


} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_MATRIXUTIL_H_ */
