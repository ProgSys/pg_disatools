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
#ifndef INCLUDE_PG_UTIL_PG_VECTOR_H_
#define INCLUDE_PG_UTIL_PG_VECTOR_H_

#include <ostream>
#include <vector>
#include <Util/PG_Exception.h>

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

	union { T x, r, s, width; };
	union { T y, g, t, height; };

	tVector2(T x = 0, T y  = 0);

	tVector2(const tVector2<T>& tvec);
	tVector2(const tVector3<T>& tvec);
	tVector2(const tVector4<T>& tvec);
	tVector2(const std::vector<T>& tvec);

	unsigned int length() const;
	T& operator[](unsigned int index);
	T const& operator[](unsigned int index) const;

	void operator=(const tVector2<T>& tvec);
	void operator=(const tVector3<T>& tvec);
	void operator=(const tVector4<T>& tvec);

	tVector2<T> operator+(const tVector2<T>& tvec) const;

	tVector2<T> operator-(const tVector2<T>& tvec) const;

	template<typename D>
	tVector2<T> operator*(D f) const;

	template<typename D>
	tVector2<T> operator/(D f) const;

	void operator+=(const tVector2<T>& tvec);

	void operator-=(const tVector2<T>& tvec);

	bool operator==(const tVector2<T>& tvec) const;

	void clear(T value = 0);

	void dump(std::ostream& o) const;

	friend std::ostream& operator<<(std::ostream& o,const tVector2<T>& tvec){
		tvec.dump(o);
		return o;
	}

	//virtual ~tVector2() = default;
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

	tVector3(const tVector2<T>& tvec, T zIn = 0);
	tVector3(const tVector3<T>& tvec);
	tVector3(const tVector4<T>& tvec);
	tVector3(const std::vector<T>& tvec);

	unsigned int length() const;
	T& operator[](unsigned int index);
	T const& operator[](unsigned int index) const;

	void operator=(const tVector2<T>& tvec);
	void operator=(const tVector3<T>& tvec);
	void operator=(const tVector4<T>& tvec);

	tVector3<T> operator+(const tVector3<T>& tvec) const;

	tVector3<T> operator-(const tVector3<T>& tvec) const;

	template<typename D>
	tVector3<T> operator*(D f) const;

	template<typename D>
	tVector3<T> operator/(D f) const;

	void operator+=(const tVector3<T>& tvec);

	void operator-=(const tVector3<T>& tvec);

	bool operator==(const tVector3<T>& tvec) const;

	void clear(T value = 0);

	void dump(std::ostream& o) const;

	friend std::ostream& operator<<(std::ostream& o,const tVector3<T>& tvec){
		tvec.dump(o);
		return o;
	}

	//virtual ~tVector3() = default;
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

	tVector4(const tVector2<T>& tvec, T zIn = 0, T wIn = 1);
	tVector4(const tVector3<T>& tvec, T wIn = 1);
	tVector4(const tVector4<T>& tvec);

	unsigned int length() const;
	T& operator[](unsigned int index);
	T const& operator[](unsigned int index) const;

	void operator=(const tVector2<T>& tvec);
	void operator=(const tVector3<T>& tvec);
	void operator=(const tVector4<T>& tvec);

	tVector4<T> operator+(const tVector4<T>& tvec) const;

	tVector4<T> operator-(const tVector4<T>& tvec) const;

	template<typename D>
	tVector4<T> operator*(D f) const;

	template<typename D>
	tVector4<T> operator/(D f) const;

	void operator+=(const tVector4<T>& tvec);

	void operator-=(const tVector4<T>& tvec);

	bool operator==(const tVector4<T>& tvec) const;
	bool equal(const tVector4<T>& tvec) const;
	bool equalXYZ(const tVector4<T>& tvec) const;
	bool equalRGB(const tVector4<T>& tvec) const;

	void clear(T value = 0);

	T const& max() const;
	T const& maxRGB() const;

	T const& min() const;
	T const& minRGB() const;

	void dump(std::ostream& o) const;

	friend std::ostream& operator<<(std::ostream& o,const tVector4<T>& tvec){
		tvec.dump(o);
		return o;
	}

	//virtual ~tVector4() = default;
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
tVector2<T>::tVector2(const std::vector<T>& tvec){
	if(tvec.size() >= 2){
		x = tvec[0];
		y = tvec[1];
	}else if(tvec.size() == 1){
		x = tvec[0];
		y = 0;
	}else{
		x = 0;
		y = 0;
	}
}

template<typename T>
unsigned int tVector2<T>::length() const{
	return 2;
}

template<typename T>
T& tVector2<T>::operator[](unsigned int index){
	assert_Test("Out of bounds!", (index >= length()) );
	return (&x)[index];
}

template<typename T>
T const& tVector2<T>::operator[](unsigned int index) const{
	assert_Test("Out of bounds!", (index >= length()) );
	return (&x)[index];
}

template<typename T>
tVector2<T> tVector2<T>::operator+(const tVector2<T>& tvec) const{
	return tVector2<T>(x+tvec.x, y+tvec.y);
}

template<typename T>
tVector2<T> tVector2<T>::operator-(const tVector2<T>& tvec) const{
	return tVector2<T>(x-tvec.x, y-tvec.y);
}

template<typename T>
template<typename D>
tVector2<T> tVector2<T>::operator*(D f) const{
	return tVector2<T>(x*f, y*f);
}

template<typename T>
template<typename D>
tVector2<T> tVector2<T>::operator/(D f) const{
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
void tVector2<T>::clear(T value){
	x = value;
	y = value;
}

template<typename T>
void tVector2<T>::dump(std::ostream& o) const{
	o<<"("<<x<<", "<<y<<")";
}

template<typename T>
tVector2<T> operator*(T f, const tVector2<T>& tvec){
	return tVector2<T>(f*tvec.x, f*tvec.y);
}


/////////////////////// VECTOR 3 DEFINITIONS ///////////////////////
template<typename T>
tVector3<T>::tVector3(T x, T y, T z): x(x),y(y), z(z) {};

template<typename T>
tVector3<T>::tVector3(const tVector2<T>& tvec, T zIn): x(tvec.x),y(tvec.y), z(zIn) {};

template<typename T>
tVector3<T>::tVector3(const tVector3<T>& tvec): x(tvec.x),y(tvec.y), z(tvec.z){};

template<typename T>
tVector3<T>::tVector3(const tVector4<T>& tvec): x(tvec.x),y(tvec.y), z(tvec.z) {};

template<typename T>
tVector3<T>::tVector3(const std::vector<T>& tvec){
	if(tvec.size() >= 3){
		x = tvec[0];
		y = tvec[1];
		z = tvec[2];
	}else if(tvec.size() == 2){
		x = tvec[0];
		y = tvec[1];
		z = 0;
	}else if(tvec.size() == 1){
		x = tvec[0];
		y = 0;
		z = 0;
	}else{
		x = 0;
		y = 0;
		z = 0;
	}
}

template<typename T>
unsigned int tVector3<T>::length() const{
	return 3;
}

template<typename T>
T& tVector3<T>::operator[](unsigned int index){
	assert_Test("Out of bounds!", (index >= length()) );
	return (&x)[index];
}

template<typename T>
T const& tVector3<T>::operator[](unsigned int index) const{
	assert_Test("Out of bounds!", (index >= length()) );
	return (&x)[index];
}

template<typename T>
tVector3<T> tVector3<T>::operator+(const tVector3<T>& tvec) const{
	return tVector3<T>(x+tvec.x, y+tvec.y, z+tvec.z);
}

template<typename T>
tVector3<T> tVector3<T>::operator-(const tVector3<T>& tvec) const{
	return tVector3<T>(x-tvec.x, y-tvec.y, z-tvec.z);
}

template<typename T>
template<typename D>
tVector3<T> tVector3<T>::operator*(D f) const{
	return tVector3<T>(x*f, y*f, z*f);
}

template<typename T>
template<typename D>
tVector3<T> tVector3<T>::operator/(D f) const{
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
void tVector3<T>::clear(T value){
	x = value;
	y = value;
	z = value;
}

template<typename T>
tVector3<T> operator*(T f, const tVector3<T>& tvec){
	return tVector3<T>(f*tvec.x, f*tvec.y, f*tvec.z);
}


/////////////////////// VECTOR 4 DEFINITIONS ///////////////////////
template<typename T>
tVector4<T>::tVector4(T x, T y, T z, T w): x(x),y(y), z(z), w(w) {};

template<typename T>
tVector4<T>::tVector4(const tVector2<T>& tvec, T zIn,T wIn): x(tvec.x),y(tvec.y), z(zIn), w(wIn) {};

template<typename T>
tVector4<T>::tVector4(const tVector3<T>& tvec,T wIn): x(tvec.x),y(tvec.y), z(tvec.z), w(wIn) {};

template<typename T>
tVector4<T>::tVector4(const tVector4<T>& tvec): x(tvec.x),y(tvec.y), z(tvec.z), w(tvec.w) {};

template<typename T>
unsigned int tVector4<T>::length() const{
	return 4;
}

template<typename T>
T& tVector4<T>::operator[](unsigned int index){
	assert_Test("Out of bounds!", (index >= length()) );
	return (&x)[index];
}

template<typename T>
T const& tVector4<T>::operator[](unsigned int index) const{
	assert_Test("Out of bounds!", (index >= length()) );
	return (&x)[index];
}

template<typename T>
tVector4<T> tVector4<T>::operator+(const tVector4<T>& tvec) const{
	return tVector4<T>(x+tvec.x, y+tvec.y, z+tvec.z, w+tvec.w);
}

template<typename T>
tVector4<T> tVector4<T>::operator-(const tVector4<T>& tvec) const{
	return tVector4<T>(x-tvec.x, y-tvec.y, z-tvec.z, w-tvec.w);
}

template<typename T>
template<typename D>
tVector4<T> tVector4<T>::operator*(D f) const{
	return tVector4<T>(x*f, y*f, z*f, w*f);
}


template<typename T>
template<typename D>
tVector4<T> tVector4<T>::operator/(D f) const{
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
bool tVector4<T>::equal(const tVector4<T>& tvec) const{
	return x == tvec.x && y == tvec.y && z == tvec.z && w == tvec.w;
}

template<typename T>
bool tVector4<T>::equalXYZ(const tVector4<T>& tvec) const{
	return x == tvec.x && y == tvec.y && z == tvec.z;
}

template<typename T>
bool tVector4<T>::equalRGB(const tVector4<T>& tvec) const{
	return equalXYZ(tvec);
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
void tVector4<T>::clear(T value){
	x = value;
	y = value;
	z = value;
	w = value;
}

template<typename T>
T const& tVector4<T>::max() const{
	if(x > y){
		if(x > z){
			if(x > a){
				return x;
			}else
				return a;
		}else{
			if(z > a){
				return z;
			}else
				return a;
		}
	}else{
		if(y > z){
			if(y > a){
				return y;
			}else
				return a;
		}else{
			if(z > a){
				return z;
			}else
				return a;
		}
	}
}

template<typename T>
T const& tVector4<T>::maxRGB() const{
	if(x > y){
		if(x > z){
			return x;
		}else
			return z;
	}else{
		if(y > z){
			return y;
		}else
			return z;
	}
}

template<typename T>
T const& tVector4<T>::min() const{
	if(x < y){
		if(x < z){
			if(x < a){
				return x;
			}else
				return a;
		}else{
			if(z < a){
				return z;
			}else
				return a;
		}
	}else{
		if(y < z){
			if(y < a){
				return y;
			}else
				return a;
		}else{
			if(z < a){
				return z;
			}else
				return a;
		}
	}
}

template<typename T>
T const& tVector4<T>::minRGB() const{
	if(x < y){
		if(x < z){
			return x;
		}else
			return z;
	}else{
		if(y < z){
			return y;
		}else
			return z;
	}
}

template<typename T>
tVector4<T> operator*(T f, const tVector4<T>& tvec){
	return tVector4<T>(f*tvec.x, f*tvec.y, f*tvec.z, f*tvec.w);
}

/*
template<typename T>
void tVector4<T>::dump(std::ostream& o) const{
	o<<"("<<x<<", "<<y<<", "<<z<<", "<<w<<")";
}*/

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
