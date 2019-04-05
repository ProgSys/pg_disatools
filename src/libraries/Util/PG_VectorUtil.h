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
#include <Util/PG_Vector.h>

namespace PG {
namespace UTIL {

template<typename T>
tVector3<T> cross(const tVector3<T>& A,const tVector3<T>& B){
	return tVector3<T>(
			A.y*B.z-A.z*B.y,
			A.z*B.x-A.x*B.z,
			A.x*B.y-A.y*B.x
			);
}

template<typename T>
T dot(const tVector4<T>& A,const tVector4<T>& B){
	return A.x*B.x+A.y*B.y+A.z*B.z+A.w*B.w;
}


template<typename T>
T dot(const tVector3<T>& A,const tVector3<T>& B){
	return A.x*B.x+A.y*B.y+A.z*B.z;
}

template<typename T>
T dot(const tVector2<T>& A,const tVector2<T>& B){
	return A.x*B.x+A.y*B.y;
}

/////// normalize ///////
template <typename T>
T invsqrt(T x){
    return 1.0 / std::sqrt(x);
}

template<typename T, template <typename> class Vec>
Vec<T> normalize( const Vec<T>& vector){
	return vector * invsqrt(dot(vector,vector));
}

/////// interpolate ///////
template<typename T>
T interpolate(T a, T b, double v){
	return a*(1-v)+b*v;
}

template<typename T, template <typename> class Vec>
Vec<T> interpolate(const Vec<T>& A,const Vec<T>& B, double v){
	const double a = (1.0f-v);
	return (A*a)+(B*v);
}

/*
template<typename T>
tVector2<T> interpolate(const tVector2<T>& A,const tVector2<T>& B, double v){
	const double a = (1.0f-v);
	return tVector2<T>(A.x*a+B.x*v, A.y*a+B.y*v);
}

template<typename T>
tVector3<T> interpolate(const tVector3<T>& A,const tVector3<T>& B, double v){
	const double a = (1.0f-v);
	return tVector3<T>(A.x*a+B.x*v, A.y*a+B.y*v, A.z*a+B.z*v);
}

template<typename T>
tVector4<T> interpolate(const tVector4<T>& A,const tVector4<T>& B, double v){
	const double a = (1.0f-v);
	return tVector4<T>(A.x*a+B.x*v, A.y*a+B.y*v, A.z*a+B.z*v, A.w*a+B.w*v);
}
*/

/////// length ///////
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

/////// distance ///////
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

/////// abs ///////
template<typename T>
tVector2<T> abs(const tVector2<T>& A){
	return  tVector2<T>(std::abs(A.x),std::abs(A.y));
}

template<typename T>
tVector3<T> abs(const tVector3<T>& A){
	return  tVector3<T>(std::abs(A.x),std::abs(A.y),std::abs(A.z));
}

template<typename T>
tVector4<T> abs(const tVector4<T>& A){
	return  tVector4<T>(std::abs(A.x),std::abs(A.y),std::abs(A.z), std::abs(A.w));
}

/////// grayscale ///////
template<typename T>
T grayscale(const tVector3<T>& A){
	return  A.r*0.2126+A.g*0.7152+A.b*0.0722;
}

template<typename T>
float grayscale(const tVector4<T>& A){
	return  A.r*0.2126+A.g*0.7152+A.b*0.0722;
}

/////// scaleRGB565to888 ///////
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

inline void reduceRGB888to565(tVector3<char>& vec){
	vec.r &= 0xF8;
	vec.g &= 0xFC;
	vec.b &= 0xF8;
}

inline void scaleRGB888to565(tVector4<char>& vec){
	vec.r = ( vec.r >> 3 );
	vec.g = ( vec.g >> 2 );
	vec.b = ( vec.b >> 3 );
}

inline void reduceRGB888to565(tVector4<char>& vec){
	vec.r &= 0xF8;
	vec.g &= 0xFC;
	vec.b &= 0xF8;
}


inline void scaleRGB565to888(tVector3<unsigned char>& vec){
	vec.r = ( vec.r << 3 );
	vec.g = ( vec.g << 2 );
	vec.b = ( vec.b << 3 );
}

inline void reduceRGB888to565(tVector3<unsigned char>& vec){
	vec.r &= 0xF8;
	vec.g &= 0xFC;
	vec.b &= 0xF8;
}

inline void scaleRGB565to888(tVector4<unsigned char>& vec){
	vec.r = ( vec.r << 3 );
	vec.g = ( vec.g << 2 );
	vec.b = ( vec.b << 3 );
}

inline void reduceRGB888to565(tVector4<unsigned char>& vec){
	vec.r &= 0xF8;
	vec.g &= 0xFC;
	vec.b &= 0xF8;
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



//to ivec
template<typename T>
ivec4 toivec4(const tVector4<T>& vec){
	return PG::UTIL::ivec4((int)vec.x,(int)vec.y,(int)vec.z,(int)vec.a);
}

template<typename T>
ivec4 toivec4(const tVector3<T>& vec){
	return PG::UTIL::ivec4((int)vec.x,(int)vec.y,(int)vec.z,0);
}

template<typename T>
ivec4 toivec4(const tVector2<T>& vec){
	return PG::UTIL::ivec4((int)vec.x,(int)vec.y,0,0);
}

//to uvec
//4
template<typename T>
uvec4 touvec4(const tVector4<T>& vec){
	return PG::UTIL::uvec4((unsigned int)vec.x,(unsigned int)vec.y,(unsigned int)vec.z,(unsigned int)vec.a);
}

template<typename T>
uvec4 touvec4(const tVector3<T>& vec){
	return PG::UTIL::uvec4((unsigned int)vec.x,(unsigned int)vec.y,(unsigned int)vec.z,0);
}

template<typename T>
uvec4 touvec4(const tVector2<T>& vec){
	return PG::UTIL::uvec4((unsigned int)vec.x,(unsigned int)vec.y,0,0);
}
//3
template<typename T>
uvec3 touvec3(const tVector4<T>& vec){
	return PG::UTIL::uvec3((unsigned int)vec.x,(unsigned int)vec.y,(unsigned int)vec.z);
}

template<typename T>
uvec3 touvec3(const tVector3<T>& vec){
	return PG::UTIL::uvec3((unsigned int)vec.x,(unsigned int)vec.y,(unsigned int)vec.z,0);
}

template<typename T>
uvec3 touvec3(const tVector2<T>& vec){
	return PG::UTIL::uvec3((unsigned int)vec.x,(unsigned int)vec.y,0);
}
//2
template<typename T>
uvec2 touvec2(const tVector4<T>& vec){
	return PG::UTIL::uvec2((unsigned int)vec.x,(unsigned int)vec.y);
}

template<typename T>
uvec2 touvec2(const tVector3<T>& vec){
	return PG::UTIL::uvec2((unsigned int)vec.x,(unsigned int)vec.y);
}

template<typename T>
uvec2 touvec2(const tVector2<T>& vec){
	return PG::UTIL::uvec2((unsigned int)vec.x,(unsigned int)vec.y);
}

//to vec
//4
template<typename T>
vec4 tovec4(const tVector4<T>& vec){
	return PG::UTIL::vec4((float)vec.x,(float)vec.y,(float)vec.z,(float)vec.a);
}

template<typename T>
vec4 tovec4(const tVector3<T>& vec){
	return PG::UTIL::vec4((float)vec.x,(float)vec.y,(float)vec.z,0);
}

template<typename T>
vec4 tovec4(const tVector2<T>& vec){
	return PG::UTIL::vec4((float)vec.x,(float)vec.y,0,0);
}
//3
template<typename T>
vec3 tovec3(const tVector4<T>& vec){
	return PG::UTIL::vec3((float)vec.x,(float)vec.y,(float)vec.z);
}

template<typename T>
vec3 tovec3(const tVector3<T>& vec){
	return PG::UTIL::vec3((float)vec.x,(float)vec.y,(float)vec.z);
}

template<typename T>
vec3 tovec3(const tVector2<T>& vec){
	return PG::UTIL::vec3((float)vec.x,(float)vec.y,0);
}
//2
template<typename T>
vec2 tovec2(const tVector4<T>& vec){
	return PG::UTIL::vec2((float)vec.x,(float)vec.y);
}

template<typename T>
vec2 tovec2(const tVector3<T>& vec){
	return PG::UTIL::vec2((float)vec.x,(float)vec.y);
}

template<typename T>
vec2 tovec2(const tVector2<T>& vec){
	return PG::UTIL::vec2((float)vec.x,(float)vec.y);
}


template<typename T>
rgba torgba(const tVector4<T>& vec){
	return PG::UTIL::rgba(vec.x,vec.y,vec.z,vec.a);
}

template<typename T>
rgba torgba(const tVector3<T>& vec){
	return PG::UTIL::rgba(vec.x,vec.y,vec.z,255);
}

template<typename T>
rgba torgba(const tVector2<T>& vec){
	return PG::UTIL::rgba(vec.x,vec.y,0,255);
}


template<typename T, template <typename> class Vec>
T const * value_ptr(const Vec<T>& vec){
	return &(vec.x);
}


} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_VECTORUTIL_H_ */
