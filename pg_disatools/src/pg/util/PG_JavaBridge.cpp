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
#include <pg/util/PG_JavaBridge.h>
#include <stdexcept>
#include <vector>

#include <pg/util/PG_Image.h>
#include <pg/files/PG_TX2.h>

namespace PG {
namespace UTIL {

void simpleCall(){
	int x = 3;
	return;
}

int multiply(float a, float b, float & result){
	result = a * b;
	return 0;
}

char* uncompressTX2Image(const char* bytes, int length, int& width, int& height){
	RGBAImage image;
	PG_INFO_STREAM("Size: "<<(unsigned int)length<<" l: "<<length);
	if(PG::FILE::decompressTX2(bytes, (unsigned int)length, image)){
		width = 0;
		height = 0;
		PG_ERROR_STREAM("Couldn't decompress TX2 image.")
		return nullptr;
	}

	width = image.getWidth();
	height = image.getHeight();

	char* byteImage = new char[image.size()*sizeof(rgba)];
	memcpy(byteImage, &image[0], image.size()*sizeof(rgba) );

	return byteImage;
}

} /* namespace UTIL */
} /* namespace PG */
