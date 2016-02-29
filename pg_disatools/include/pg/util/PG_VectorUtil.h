/*
 * Some helper function for vectors.
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

#ifndef INCLUDE_PG_UTIL_PG_VECTORUTIL_H_
#define INCLUDE_PG_UTIL_PG_VECTORUTIL_H_

#include <pg/util/PG_Vector.h>

namespace PG {
namespace UTIL {

template<typename T>
tVector2<T> interpolate(const tVector2<T>& A,const tVector2<T>& B, float v){
	const float a = (1.0f-v);
	return tVector2<T>(A.x*a+B.x*v, A.y*a+B.y*v);
}

template<typename T>
tVector3<T> interpolate(const tVector3<T>& A,const tVector3<T>& B, float v){
	const float a = (1.0f-v);
	return tVector3<T>(A.x*a+B.x*v, A.y*a+B.y*v, A.z*a+B.z*v);
}

template<typename T>
tVector4<T> interpolate(const tVector4<T>& A,const tVector4<T>& B, float v){
	const float a = (1.0f-v);
	return tVector4<T>(A.x*a+B.x*v, A.y*a+B.y*v, A.z*a+B.z*v, A.w*a+B.w*v);
}

template<typename T>
T interpolate(T a, T b, float v){
	return a*(1-v)+b*v;
}

inline void scaleRGB565(tVector3<char>& vec){
	vec.r = ( vec.r << 3 ) | ( vec.r >> 2 );
	vec.g = ( vec.g << 2 ) | ( vec.g >> 4 );
	vec.b = ( vec.b << 3 ) | ( vec.b >> 2 );
}

inline void scaleRGB565(tVector4<char>& vec){
	vec.r = ( vec.r << 3 ) | ( vec.r >> 2 );
	vec.g = ( vec.g << 2 ) | ( vec.g >> 4 );
	vec.b = ( vec.b << 3 ) | ( vec.b >> 2 );
}

inline void scaleRGB565(tVector3<unsigned char>& vec){
	vec.r = ( vec.r << 3 ) | ( vec.r >> 2 );
	vec.g = ( vec.g << 2 ) | ( vec.g >> 4 );
	vec.b = ( vec.b << 3 ) | ( vec.b >> 2 );
}

inline void scaleRGB565(tVector4<unsigned char>& vec){
	vec.r = ( vec.r << 3 ) | ( vec.r >> 2 );
	vec.g = ( vec.g << 2 ) | ( vec.g >> 4 );
	vec.b = ( vec.b << 3 ) | ( vec.b >> 2 );
}


} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_VECTORUTIL_H_ */
