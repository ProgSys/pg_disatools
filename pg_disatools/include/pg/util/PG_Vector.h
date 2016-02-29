/*
 * Simple vector classes.
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

#ifndef INCLUDE_PG_UTIL_PG_VECTOR_H_
#define INCLUDE_PG_UTIL_PG_VECTOR_H_

#include <ostream>

namespace PG {
namespace UTIL {

// forward deceleration
template<typename T>
struct tVector2;

template<typename T>
struct tVector3;

template<typename T>
struct tVector4;

/*!
 * @brief A simple three dimension vector.
 */
template<typename T>
struct tVector2{

	union { T x, r, s; };
	union { T y, g, t; };

	tVector2(T x = 0, T y  = 0);

	tVector2(const tVector2<T>& tvec);
	tVector2(const tVector3<T>& tvec);
	tVector2(const tVector4<T>& tvec);

	tVector2<T> operator+(const tVector2<T>& tvec) const;

	tVector2<T> operator-(const tVector2<T>& tvec) const;

	tVector2<T> operator*(float f) const;

	tVector2<T> operator/(float f) const;

	void operator+=(const tVector2<T>& tvec);

	void operator-=(const tVector2<T>& tvec);

	bool operator==(const tVector2<T>& tvec) const;

	void operator=(const tVector2<T>& tvec);
	void operator=(const tVector3<T>& tvec);
	void operator=(const tVector4<T>& tvec);

	void dump(std::ostream& o) const;

	friend std::ostream& operator<<(std::ostream& o,const tVector2<T>& tvec){
		tvec.dump(o);
		return o;
	}

	virtual ~tVector2() = default;
};

/*!
 * @brief A simple three dimension vector.
 */
template<typename T>
struct tVector3{

	union { T x, r, s; };
	union { T y, g, t; };
	union { T z, b, p; };

	tVector3(T x = 0, T y  = 0, T z = 0);

	tVector3(const tVector2<T>& tvec);
	tVector3(const tVector3<T>& tvec);
	tVector3(const tVector4<T>& tvec);

	tVector3<T> operator+(const tVector3<T>& tvec) const;

	tVector3<T> operator-(const tVector3<T>& tvec) const;

	tVector3<T> operator*(float f) const;

	tVector3<T> operator/(float f) const;

	void operator+=(const tVector3<T>& tvec);

	void operator-=(const tVector3<T>& tvec);

	bool operator==(const tVector3<T>& tvec) const;

	void operator=(const tVector2<T>& tvec);
	void operator=(const tVector3<T>& tvec);
	void operator=(const tVector4<T>& tvec);

	void dump(std::ostream& o) const;

	friend std::ostream& operator<<(std::ostream& o,const tVector3<T>& tvec){
		tvec.dump(o);
		return o;
	}

	virtual ~tVector3() = default;
};

/*!
 * @brief A simple four dimension vector.
 */
template<typename T>
struct tVector4{

	union { T x, r, s; };
	union { T y, g, t; };
	union { T z, b, p; };
	union { T w, a, q; };

	//tVector4(): x(0),y(0), z(0), w(0) {};
	tVector4(T x = 0, T y  = 0, T z = 0, T w = 0);

	tVector4(const tVector2<T>& tvec);
	tVector4(const tVector3<T>& tvec);
	tVector4(const tVector4<T>& tvec);

	tVector4<T> operator+(const tVector4<T>& tvec) const;

	tVector4<T> operator-(const tVector4<T>& tvec) const;

	tVector4<T> operator*(float f) const;

	tVector4<T> operator/(float f) const;

	void operator+=(const tVector4<T>& tvec);

	void operator-=(const tVector4<T>& tvec);

	bool operator==(const tVector4<T>& tvec) const;

	void operator=(const tVector2<T>& tvec);
	void operator=(const tVector3<T>& tvec);
	void operator=(const tVector4<T>& tvec);

	void dump(std::ostream& o) const;

	friend std::ostream& operator<<(std::ostream& o,const tVector4<T>& tvec){
		tvec.dump(o);
		return o;
	}

	virtual ~tVector4() = default;
};

/////////////////////// VECTOR 2 DEFINITIONS ///////////////////////
template<typename T>
tVector2<T>::tVector2(T x, T y): x(x),y(y) {};

template<typename T>
tVector2<T>::tVector2(const tVector2<T>& tvec): x(tvec.x),y(tvec.y) {};

template<typename T>
tVector2<T>::tVector2(const tVector3<T>& tvec): x(tvec.x),y(tvec.y){};

template<typename T>
tVector2<T>::tVector2(const tVector4<T>& tvec): x(tvec.x),y(tvec.y) {};

template<typename T>
tVector2<T> tVector2<T>::operator+(const tVector2<T>& tvec) const{
	return tVector2<T>(x+tvec.x, y+tvec.y);
}

template<typename T>
tVector2<T> tVector2<T>::operator-(const tVector2<T>& tvec) const{
	return tVector2<T>(x-tvec.x, y-tvec.y);
}

template<typename T>
tVector2<T> tVector2<T>::operator*(float f) const{
	return tVector2<T>(x*f, y*f);
}

template<typename T>
tVector2<T> tVector2<T>::operator/(float f) const{
	return tVector2<T>(x/f, y/f);
}

template<typename T>
void tVector2<T>::operator+=(const tVector2<T>& tvec){
	x += tvec.x;
	y += tvec.y;
}

template<typename T>
void tVector2<T>::operator-=(const tVector2<T>& tvec){
	x -= tvec.x;
	y -= tvec.y;
}

template<typename T>
bool tVector2<T>::operator==(const tVector2<T>& tvec) const{
	return x == tvec.x && y == tvec.y;
}

template<typename T>
void tVector2<T>::operator=(const tVector2<T>& tvec){
	x = tvec.x;
	y = tvec.y;
}

template<typename T>
void tVector2<T>::operator=(const tVector3<T>& tvec){
	x = tvec.x;
	y = tvec.y;
}

template<typename T>
void tVector2<T>::operator=(const tVector4<T>& tvec){
	x = tvec.x;
	y = tvec.y;
}

template<typename T>
void tVector2<T>::dump(std::ostream& o) const{
	//o<<"("<<x<<", "<<y<<")";
}



/////////////////////// VECTOR 3 DEFINITIONS ///////////////////////
template<typename T>
tVector3<T>::tVector3(T x, T y, T z): x(x),y(y), z(z) {};

template<typename T>
tVector3<T>::tVector3(const tVector2<T>& tvec): x(tvec.x),y(tvec.y), z(0) {};

template<typename T>
tVector3<T>::tVector3(const tVector3<T>& tvec): x(tvec.x),y(tvec.y), z(tvec.z){};

template<typename T>
tVector3<T>::tVector3(const tVector4<T>& tvec): x(tvec.x),y(tvec.y), z(tvec.z) {};

template<typename T>
tVector3<T> tVector3<T>::operator+(const tVector3<T>& tvec) const{
	return tVector3<T>(x+tvec.x, y+tvec.y, z+tvec.z);
}

template<typename T>
tVector3<T> tVector3<T>::operator-(const tVector3<T>& tvec) const{
	return tVector3<T>(x-tvec.x, y-tvec.y, z-tvec.z);
}

template<typename T>
tVector3<T> tVector3<T>::operator*(float f) const{
	return tVector3<T>(x*f, y*f, z*f);
}

template<typename T>
tVector3<T> tVector3<T>::operator/(float f) const{
	return tVector3<T>(x/f, y/f, z/f);
}

template<typename T>
void tVector3<T>::operator+=(const tVector3<T>& tvec){
	x += tvec.x;
	y += tvec.y;
	z += tvec.z;
}

template<typename T>
void tVector3<T>::operator-=(const tVector3<T>& tvec){
	x -= tvec.x;
	y -= tvec.y;
	z -= tvec.z;
}

template<typename T>
bool tVector3<T>::operator==(const tVector3<T>& tvec) const{
	return x == tvec.x && y == tvec.y && z == tvec.z;
}

template<typename T>
void tVector3<T>::operator=(const tVector2<T>& tvec){
	x = tvec.x;
	y = tvec.y;
}

template<typename T>
void tVector3<T>::operator=(const tVector3<T>& tvec){
	x = tvec.x;
	y = tvec.y;
	z = tvec.z;
}

template<typename T>
void tVector3<T>::operator=(const tVector4<T>& tvec){
	x = tvec.x;
	y = tvec.y;
	z = tvec.z;
}

template<typename T>
void tVector3<T>::dump(std::ostream& o) const{
	//o<<"("<<x<<", "<<y<<", "<<z<<")";
}




/////////////////////// VECTOR 4 DEFINITIONS ///////////////////////
template<typename T>
tVector4<T>::tVector4(T x, T y, T z, T w): x(x),y(y), z(z), w(w) {};

template<typename T>
tVector4<T>::tVector4(const tVector2<T>& tvec): x(tvec.x),y(tvec.y), z(0), w(0) {};

template<typename T>
tVector4<T>::tVector4(const tVector3<T>& tvec): x(tvec.x),y(tvec.y), z(tvec.z), w(0) {};

template<typename T>
tVector4<T>::tVector4(const tVector4<T>& tvec): x(tvec.x),y(tvec.y), z(tvec.z), w(tvec.w) {};

template<typename T>
tVector4<T> tVector4<T>::operator+(const tVector4<T>& tvec) const{
	return tVector4<T>(x+tvec.x, y+tvec.y, z+tvec.z, w+tvec.w);
}

template<typename T>
tVector4<T> tVector4<T>::operator-(const tVector4<T>& tvec) const{
	return tVector4<T>(x-tvec.x, y-tvec.y, z-tvec.z, w-tvec.w);
}

template<typename T>
tVector4<T> tVector4<T>::operator*(float f) const{
	return tVector4<T>(x*f, y*f, z*f, w*f);
}

template<typename T>
tVector4<T> tVector4<T>::operator/(float f) const{
	return tVector4<T>(x/f, y/f, z/f, w/f);
}

template<typename T>
void tVector4<T>::operator+=(const tVector4<T>& tvec){
	x += tvec.x;
	y += tvec.y;
	z += tvec.z;
	w += tvec.w;
}

template<typename T>
void tVector4<T>::operator-=(const tVector4<T>& tvec){
	x -= tvec.x;
	y -= tvec.y;
	z -= tvec.z;
	w -= tvec.w;
}

template<typename T>
bool tVector4<T>::operator==(const tVector4<T>& tvec) const{
	return x == tvec.x && y == tvec.y && z == tvec.z && w == tvec.w;
}

template<typename T>
void tVector4<T>::operator=(const tVector2<T>& tvec){
	x = tvec.x;
	y = tvec.y;
}

template<typename T>
void tVector4<T>::operator=(const tVector3<T>& tvec){
	x = tvec.x;
	y = tvec.y;
	z = tvec.z;
}

template<typename T>
void tVector4<T>::operator=(const tVector4<T>& tvec){
	x = tvec.x;
	y = tvec.y;
	z = tvec.z;
	w = tvec.w;
}

template<typename T>
void tVector4<T>::dump(std::ostream& o) const{
	//o<<"("<<x<<", "<<y<<", "<<z<<", "<<w<<")";
}

typedef tVector2<unsigned char> uv;
typedef tVector3<unsigned char> rgb;
typedef tVector4<unsigned char> rgba;

typedef tVector2<short> svec2;
typedef tVector3<short> svec3;
typedef tVector4<short> svec4;

typedef tVector2<unsigned short> usvec2;
typedef tVector3<unsigned short> usvec3;
typedef tVector4<unsigned short> usvec4;

typedef tVector2<int> ivec2;
typedef tVector3<int> ivec3;
typedef tVector4<int> ivec4;

typedef tVector2<unsigned int> uvec2;
typedef tVector3<unsigned int> uvec3;
typedef tVector4<unsigned int> uvec4;

typedef tVector2<long> lvec2;
typedef tVector3<long> lvec3;
typedef tVector4<long> lvec4;

typedef tVector2<unsigned long> ulvec2;
typedef tVector3<unsigned long> ulvec3;
typedef tVector4<unsigned long> ulvec4;

typedef tVector2<float> vec2;
typedef tVector3<float> vec3;
typedef tVector4<float> vec4;

typedef tVector2<double> dvec2;
typedef tVector3<double> dvec3;
typedef tVector4<double> dvec4;

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_VECTOR_H_ */
