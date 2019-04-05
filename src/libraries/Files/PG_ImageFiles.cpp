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
#include <Files/PG_ImageFiles.h>
#include <sstream>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>

namespace PG {
namespace FILE {

bool saveTGA(const std::string& filepath, const PG::UTIL::RGBAImage& image){
	try{
		PG::STREAM::OutByteFile writer(filepath);
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
	}catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't save TGA Image! : "<<e.what());
		 return false;
	}catch (...) {
		 PG_ERROR_STREAM("Couldn't save TGA Image!");
		 return false;
	}
	return true;
}

bool saveTGA(const std::string& filepath, const PG::UTIL::IDImage& image){
	try{
		PG::STREAM::OutByteFile writer(filepath);
		writer.writeInt(131072);
		writer.writeInt(0);
		writer.writeInt(0);
		writer.writeShort(image.getWidth());
		writer.writeShort(image.getHeight());
		writer.writeShort(2080);


		for(unsigned int y = 0; y < image.getHeight(); ++y ){
			for(unsigned int x = 0; x < image.getWidth(); ++x ){
				const unsigned int index = ((image.getHeight()-y-1)*image.getWidth()+x); //y-flip
				const unsigned char pix = image[index];

				writer.writeChar(pix);
				writer.writeChar(pix);
				writer.writeChar(pix);
				writer.writeChar(255_c);
			}
		}

		writer.writeLongLong(0);
		writer.writeLongLong(0);
		writer.writeString("TRUEVISION-XFILE.");
	}catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't save TGA Image! : "<<e.what());
		 return false;
	}catch (...) {
		 PG_ERROR_STREAM("Couldn't save TGA Image!");
		 return false;
	}
	return true;
}

bool loadTGA(const std::string& filepath, PG::UTIL::RGBAImage& imageOut){
	PG::STREAM::InByteFile reader(filepath);

	if(reader.readUnsignedInt() != 131072){
		PG_ERROR_STREAM("TGA has wrong format, needs to be BGRA8888 (32 bit).");
		return false;
	}
	reader.readUnsignedInt();
	reader.readUnsignedInt();
	const unsigned int width = reader.readUnsignedShort();
	const unsigned int height = reader.readUnsignedShort();

	if(reader.readUnsignedShort() != 2080){
		PG_ERROR_STREAM("TGA has wrong format, needs to be BGRA8888 (32 bit).");
		return false;
	}

	imageOut.resize(width, height);

	for(unsigned int y = 0; y < height; ++y ){
		for(unsigned int x = 0; x < width; ++x ){
			const unsigned int index = ((height-y-1)*width+x); //y-flip
			PG::UTIL::rgba& pix = imageOut[index];

			pix.b = reader.readUnsignedChar();
			pix.g = reader.readUnsignedChar();
			pix.r = reader.readUnsignedChar();
			pix.a = reader.readUnsignedChar();
		}
	}

	return true;
}

bool saveNetPNM(const std::string& filepath, const PG::UTIL::RGBAImage& image){
	//return savePPM(outfilename);
	PG::STREAM::OutByteFile writer(filepath);
	std::stringstream o;
	o<<"P6\n"<<image.getWidth()<<"\n"<<image.getHeight()<<"\n255\n";
	writer.writeString(o.str());

	for(const PG::UTIL::rgba& pix: image){
		writer.writeChar(pix.r);
		writer.writeChar(pix.g);
		writer.writeChar(pix.b);
	}
	return true;
}

bool loadNetPNM(const std::string& filepath, PG::UTIL::RGBAImage& imageOut){
	PG::STREAM::InByteFile reader(filepath);

	char type = 0;
	{
		std::string strtype = reader.readString(2);
		if(strtype == "P3"){
			type = 1;
		}else if(strtype == "P6"){
			type = 2;
		}else{
			PG_ERROR_STREAM("Only PNM/PBM P3 or P6 is supported!");
			return false;
		}
	}


	reader.skip(1);

	unsigned int width = 0;
	unsigned int height = 0;

	{
		std::stringstream o;
		char c;
		while(  (c = reader.readChar()) != '\n' && !reader.eof()){
			o<<c;
		}
		o >> width;
		o.clear();
		while( (c = reader.readChar()) != '\n' && !reader.eof()){
			o<<c;
		}
		o >> height;

		//PG_INFO_STREAM(width <<" "<<height);

		if(reader.readString(3) != "255"){
			PG_ERROR_STREAM("Max value should be 255!");
			return false;
		}
		reader.skip(1);
	}

	imageOut.resize(width, height);
	unsigned int size = width*height*3;
	//read values
	if(type == 1){
		std::stringstream o;
		char c = 0;
		for(unsigned int i = 0; i < width*height; ++i){
			if(reader.eof()) break;

			PG::UTIL::rgba& pixel = imageOut[i];
			pixel.a = 255;
			while(  (c == ' ' || (c = reader.readChar()) != '\n') && !reader.eof() ){
				o<<c;
			}
			o>>pixel.r;
			o.clear();
			while(  (c == ' ' || (c = reader.readChar()) != '\n') && !reader.eof() ){
				o<<c;
			}
			o>>pixel.g;
			o.clear();
			while(  (c == ' ' || (c = reader.readChar()) != '\n') && !reader.eof() ){
				o<<c;
			}
			o>>pixel.b;
			o.clear();

		}


	}else if(type == 2){
		if(reader.size() < (reader.pos()+size)){
			PG_WARN_STREAM(reader.pos() << " "<<size);
			PG_WARN_STREAM("Given image is too small! ("<<reader.size()<<" < " << (reader.pos()+size)<<")");
			size =  reader.size() - reader.pos();
		}
		for(unsigned int i = 0; i < size; i+=3){
			//const unsigned int index = ;
			PG::UTIL::rgba& pixel = imageOut[i/3];

			reader.read((char*)&pixel.r, 3);
			//pixel.r = reader.readChar();
			//pixel.g = reader.readChar();
			//pixel.b = reader.readChar();
			pixel.a = 255;
		}

	}else
		return false;

	return true;
}

} /* namespace FILE */
} /* namespace PG */
