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
#include <pg/util/PG_Colors.h>

namespace PG {
namespace UTIL {

void RGBColor::scaleRGB565(){
	r = ( r << 3 ) | ( r >> 2 );
	g = ( g << 2 ) | ( g >> 4 );
	b = ( b << 3 ) | ( b >> 2 );
}

RGB::RGB(unsigned char _r, unsigned char _g, unsigned char _b):
		RGBColor(_r,_g,_b){}

RGB::RGB(const RGBA& rgba):
		RGBColor(rgba.r,rgba.g,rgba.b){}

RGB RGB::operator+(const RGB& rgb) const{
	return RGB(r+rgb.r, g+rgb.g, b+rgb.b);
}

RGB RGB::operator-(const RGB& rgb) const{
	return RGB(r-rgb.r, g-rgb.g, b-rgb.b);
}

RGB RGB::operator*(float f) const{
	return RGB(r*f, g*f, b*f);
}

RGB RGB::operator/(float f) const{
	return RGB(r/f, g/f, b/f);
}

void RGB::operator+=(const RGB& rgb){
	//r = ( (((unsigned short)rgb.r)+r)  > 255)? 255: (rgb.r + r );
	//g = ( (((unsigned short)rgb.g)+g)  > 255)? 255: (rgb.r + r );
	//b = ( (((unsigned short)rgb.b)+b)  > 255)? 255: (rgb.r + r );
	r += rgb.r;
	g += rgb.g;
	b += rgb.b;
}

void RGB::operator-=(const RGB& rgb){
	r -= rgb.r;
	g -= rgb.g;
	b -= rgb.b;
}

bool RGB::operator==(const RGB& rgb) const{
	return r == rgb.r && g == rgb.g && b == rgb.b;
}

void RGB::operator=(const RGB& rgb){
	r = rgb.r;
	g = rgb.g;
	b = rgb.b;
}



RGBA::RGBA(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a):
		RGBColor(_r,_g,_b),a(_a){}

RGBA::RGBA(const RGB& rgb, unsigned char _a):
		RGBColor(rgb.r,rgb.g,rgb.b),a(_a){}

RGBA::RGBA(const simpleRGBA& rgba):
	RGBColor(rgba.r,rgba.g,rgba.b),a(rgba.a){}

RGBA RGBA::operator+(const RGBA& rgba) const{
	return RGBA(r+rgba.r, g+rgba.g, b+rgba.b, a+rgba.a);
}

RGBA RGBA::operator-(const RGBA& rgba) const{
	return RGBA(r-rgba.r, g-rgba.g, b-rgba.b, a-rgba.a);
}

RGBA RGBA::operator*(float f) const{
	return RGBA(r*f, g*f, b*f, a*f);
}

RGBA RGBA::operator/(float f) const{
	return RGBA(r/f, g/f, b/f, a/f);
}

void RGBA::operator+=(const RGBA& rgba){
	r += rgba.r;
	g += rgba.g;
	b += rgba.b;
	a += rgba.a;
}

void RGBA::operator-=(const RGBA& rgba){
	r -= rgba.r;
	g -= rgba.g;
	b -= rgba.b;
	a -= rgba.a;
}

bool RGBA::operator==(const RGBA& rgba) const{
	return r == rgba.r && g == rgba.g && b == rgba.b && a == rgba.a;
}

void RGBA::operator=(const RGBA& rgba){
	r = rgba.r;
	g = rgba.g;
	b = rgba.b;
	a = rgba.a;
}

void RGBA::operator=(const simpleRGBA& rgba){
	r = rgba.r;
	g = rgba.g;
	b = rgba.b;
	a = rgba.a;
}


} /* namespace UTIL */
} /* namespace PG */
