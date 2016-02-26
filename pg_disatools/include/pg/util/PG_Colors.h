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

#ifndef INCLUDE_PG_UTIL_PG_COLORS_H_
#define INCLUDE_PG_UTIL_PG_COLORS_H_

#include <ostream>

namespace PG {
namespace UTIL {

struct simpleRGBA{
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
	unsigned char a = 0;
};

//forward deceleration
struct RGBA;

struct RGBColor{
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;

	RGBColor(unsigned char _r = 0, unsigned char _g = 0, unsigned char _b = 0):
	r(_r),g(_g), b(_b){

	}

	void scaleRGB565();
};

/*!
 * @brief Simple RGB color.
 */
struct RGB: public RGBColor{
	RGB() = default;
	RGB(const RGB& rgb) = default;
	RGB(unsigned char _r, unsigned char _g, unsigned char _b);
	RGB(const RGBA& rgba);

	RGB operator+(const RGB& rgb) const;

	RGB operator-(const RGB& rgb) const;

	RGB operator*(float f) const;

	RGB operator/(float f) const;

	void operator+=(const RGB& rgb);

	void operator-=(const RGB& rgb);

	bool operator==(const RGB& rgb) const;

	void operator=(const RGB& rgb);


	friend std::ostream& operator<<(std::ostream& o,const RGB& rgb){
		return o<<"("<<(int)rgb.r<<", "<<(int)rgb.g<<", "<<(int)rgb.b<<")";
	}

	virtual ~RGB() = default;
};

/*!
 * @brief Simple RGBA color.
 */
struct RGBA: public RGBColor{
	unsigned char a = 255;

	RGBA() = default;
	RGBA(const RGBA& rgba) = default;
	RGBA(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255);
	RGBA(const RGB& rgb, unsigned char _a = 255);
	RGBA(const simpleRGBA& rgba);

	RGBA operator+(const RGBA& rgba) const;

	RGBA operator-(const RGBA& rgba) const;

	RGBA operator*(float f) const;

	RGBA operator/(float f) const;

	void operator+=(const RGBA& rgba);

	void operator-=(const RGBA& rgba);

	bool operator==(const RGBA& rgba) const;


	void operator=(const RGBA& rgba);
	void operator=(const simpleRGBA& rgba);

	friend std::ostream& operator<<(std::ostream& o,const RGBA& rgba){
		return o<<"("<<(int)rgba.r<<", "<<(int)rgba.g<<", "<<(int)rgba.b<<", "<<(int)rgba.a<<")";
	}

	virtual ~RGBA() = default;
};

inline PG::UTIL::RGBA interpolate(const PG::UTIL::RGBA& A,const PG::UTIL::RGBA& B, float v){
	const float a = (1.0f-v);
	return PG::UTIL::RGBA(
			(unsigned char) (A.r*a+B.r*v),
			(unsigned char) (A.g*a+B.g*v),
			(unsigned char) (A.b*a+B.b*v),
			(unsigned char) (A.a*a+B.a*v)
			);
}

inline unsigned char interpolate(unsigned char a, unsigned char b, float v){
	return a*(1-v)+b*v;
}

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_COLORS_H_ */
