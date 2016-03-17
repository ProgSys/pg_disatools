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
#ifndef INCLUDE_PG_UTIL_PG_VECTORUTIL_H_
#define INCLUDE_PG_UTIL_PG_VECTORUTIL_H_

#include <cmath>
#include <pg/util/PG_Vector.h>

namespace PG {
namespace UTIL {

template<typename T>
T interpolate(T a, T b, double v){
	return a*(1-v)+b*v;
}

template<typename T>
tVector2<T> interpolate(const tVector2<T>& A,const tVector2<T>& B, double v){
	const float a = (1.0f-v);
	return tVector2<T>(A.x*a+B.x*v, A.y*a+B.y*v);
}

template<typename T>
tVector3<T> interpolate(const tVector3<T>& A,const tVector3<T>& B, double v){
	const float a = (1.0f-v);
	return tVector3<T>(A.x*a+B.x*v, A.y*a+B.y*v, A.z*a+B.z*v);
}

template<typename T>
tVector4<T> interpolate(const tVector4<T>& A,const tVector4<T>& B, double v){
	const float a = (1.0f-v);
	return tVector4<T>(A.x*a+B.x*v, A.y*a+B.y*v, A.z*a+B.z*v, A.w*a+B.w*v);
}

template<typename T>
float length(const tVector2<T>& A){
	return  std::sqrt(std::pow(A.x, 2.0)+std::pow(A.y, 2.0));
}

template<typename T>
float length(const tVector3<T>& A){
	return  std::sqrt(std::pow(A.x, 2.0)+std::pow(A.y, 2.0)+std::pow(A.z, 2.0));
}

template<typename T>
float length(const tVector4<T>& A){
	return  std::sqrt(std::pow(A.x, 2.0)+std::pow(A.y, 2.0)+std::pow(A.z, 2.0)+std::pow(A.a, 2.0));
}

template<typename T>
float distance(T a, T b){
	return  std::sqrt(std::pow(a-b, 2.0));
}

template<typename T>
float distance(const tVector2<T>& A,const tVector2<T>& B){
	return  std::sqrt(std::pow(A.x-B.x, 2.0)+std::pow(A.y-B.y, 2.0));
}

template<typename T>
float distance(const tVector3<T>& A,const tVector3<T>& B){
	return  std::sqrt(std::pow(A.x-B.x, 2.0)+std::pow(A.y-B.y, 2.0)+std::pow(A.z-B.z, 2.0));
}

template<typename T>
float distance(const tVector4<T>& A,const tVector4<T>& B){
	return  std::sqrt(std::pow(A.x-B.x, 2.0)+std::pow(A.y-B.y, 2.0)+std::pow(A.z-B.z, 2.0)+std::pow(A.a-B.a, 2.0));
}

template<typename T>
float distanceRGB(const tVector4<T>& A,const tVector4<T>& B){
	return  std::sqrt(std::pow(A.x-B.x, 2.0)+std::pow(A.y-B.y, 2.0)+std::pow(A.z-B.z, 2.0));
}

template<typename T>
float distanceXYZ(const tVector4<T>& A,const tVector4<T>& B){
	return  std::sqrt(std::pow(A.x-B.x, 2.0)+std::pow(A.y-B.y, 2.0)+std::pow(A.z-B.z, 2.0));
}


template<typename T>
T grayscale(const tVector3<T>& A){
	return  A.r*0.2126+A.g*0.7152+A.b*0.0722;
}

template<typename T>
float grayscale(const tVector4<T>& A){
	return  A.r*0.2126+A.g*0.7152+A.b*0.0722;
}




inline void scaleRGB565to888(tVector3<char>& vec){
	/*
	vec.r = ( vec.r << 3 ) | ( vec.r >> 2 );
	vec.g = ( vec.g << 2 ) | ( vec.g >> 4 );
	vec.b = ( vec.b << 3 ) | ( vec.b >> 2 );
	*/

	vec.r = ( vec.r << 3 );
	vec.g = ( vec.g << 2 );
	vec.b = ( vec.b << 3 );
}

inline void scaleRGB565to888(tVector4<char>& vec){
	vec.r = ( vec.r << 3 );
	vec.g = ( vec.g << 2 );
	vec.b = ( vec.b << 3 );
}

inline void scaleRGB888to565(tVector3<char>& vec){
	vec.r = ( vec.r >> 3 );
	vec.g = ( vec.g >> 2 );
	vec.b = ( vec.b >> 3 );
}

inline void scaleRGB888to565(tVector4<char>& vec){
	vec.r = ( vec.r >> 3 );
	vec.g = ( vec.g >> 2 );
	vec.b = ( vec.b >> 3 );
}


inline void scaleRGB565to888(tVector3<unsigned char>& vec){
	vec.r = ( vec.r << 3 );
	vec.g = ( vec.g << 2 );
	vec.b = ( vec.b << 3 );
}

inline void scaleRGB565to888(tVector4<unsigned char>& vec){
	vec.r = ( vec.r << 3 );
	vec.g = ( vec.g << 2 );
	vec.b = ( vec.b << 3 );
}

inline void scaleRGB888to565(tVector3<unsigned char>& vec){
	vec.r = ( vec.r >> 3 );
	vec.g = ( vec.g >> 2 );
	vec.b = ( vec.b >> 3 );
}

inline void scaleRGB888to565(tVector4<unsigned char>& vec){
	vec.r = ( vec.r >> 3 );
	vec.g = ( vec.g >> 2 );
	vec.b = ( vec.b >> 3 );
}



} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_VECTORUTIL_H_ */
