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
#include <pg/files/PG_ImageFiles.h>
#include <sstream>
#include <pg/util/PG_BinaryFileWriter.h>

namespace PG {
namespace FILE {

void saveTGA(const std::string& filepath, const PG::UTIL::RGBAImage& image){
	PG::UTIL::BinaryFileWriter writer(filepath);
	writer.writeInt(131072);
	writer.writeInt(0);
	writer.writeInt(0);
	writer.writeShort(image.getWidth());
	writer.writeShort(image.getHeight());
	writer.writeShort(2080);


	for(unsigned int y = 0; y < image.getHeight(); ++y ){
		for(unsigned int x = 0; x < image.getWidth(); ++x ){
			const unsigned int index = ((image.getHeight()-y-1)*image.getWidth()+x); //y-flip
			const PG::UTIL::rgba& pix = image[index];

			writer.writeChar(pix.b);
			writer.writeChar(pix.g);
			writer.writeChar(pix.r);
			writer.writeChar(pix.a);
		}
	}

	writer.writeLongLong(0);
	writer.writeLongLong(0);
	writer.writeString("TRUEVISION-XFILE.");
}

void savePGM(const std::string& filepath, const PG::UTIL::RGBAImage& image){
	//return savePPM(outfilename);
	PG::UTIL::BinaryFileWriter writer(filepath);
	std::stringstream o;
	o<<"P6\n"<<image.getWidth()<<"\n"<<image.getHeight()<<"\n255\n";
	writer.writeString(o.str());

	for(const PG::UTIL::rgba& pix: image){
		writer.writeChar(pix.r);
		writer.writeChar(pix.g);
		writer.writeChar(pix.b);
	}
}

} /* namespace FILE */
} /* namespace PG */
