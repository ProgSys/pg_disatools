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
#include <Files/PG_TX2.h>
#include <Files/PG_S3Compression.h>
#include <Util/PG_Exception.h>


#include <Stream/PG_StreamIn.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInByteArray.h>

#include <Stream/PG_StreamOut.h>
#include <Stream/PG_StreamOutByteFile.h>

#include <Util/PG_ImageUtil.h>

namespace PG
{
	namespace FILE
	{

		bool readTX2Header(PG::STREAM::In* instream, tx2Image::tx2header& header) {
			header.width = instream->readUnsignedShort();
			header.height = instream->readUnsignedShort();

			//invalid with or height
			if (header.width <= 0 || header.width > 15000 || header.height <= 0 || header.height > 15000) {
				PG_ERROR_STREAM("Invalid width or height!");
				return true;
			}

			header.type = static_cast<tx2Type>(instream->readUnsignedShort());
			instream->skip(2);
			header.colortableSize = instream->readUnsignedShort();
			const unsigned short numberOfColortables = instream->readUnsignedShort();

			header.colortables.resize((numberOfColortables) ? numberOfColortables :
				(header.type == tx2Type::BGRA || header.type == tx2Type::DXT1 || header.type == tx2Type::DXT4 || header.type == tx2Type::DXT5 || header.type == tx2Type::TX2ERROR) ? 0 : 1);

			instream->skip(4);


			for (ColorTable& colortable : header.colortables) {
				colortable.resize(header.colortableSize);
				instream->read((char*)&colortable[0], header.colortableSize * sizeof(PG::UTIL::rgba));


				if (header.type == tx2Type::COLORTABLE_BGRA16 || header.type == tx2Type::COLORTABLE_BGRA256)
					//flip R and B channel
					for (PG::UTIL::rgba& color : colortable) {
						const char r = color.r;
						color.r = color.b;
						color.b = r;
					}
			}



			return false;
		}


		bool decompressTX2(PG::STREAM::In* instream, const tx2Image::tx2header& img, PG::UTIL::RGBAImage& imageOut) {
			if (img.type == tx2Type::DXT1) {
				//DXT1
				instream->skip(img.colortableSize * 4);

				const unsigned int number_of_blocks_width = (img.width / 4);
				const unsigned int number_of_blocks_height = (img.height / 4);
				const unsigned int number_of_blocks_4x4 = number_of_blocks_width * number_of_blocks_height;

				//wrong file size?
				if (instream->size() < (number_of_blocks_4x4 * 8 + 16)) {
					PG_ERROR_STREAM("File too small! (" << instream->size() << " < " << (number_of_blocks_4x4 * 8 + 16) << ")");
					return true;
				}

				std::vector<PG::FILE::DXT1block> blocks(number_of_blocks_4x4);
				instream->read((char*)&blocks[0], number_of_blocks_4x4 * sizeof(PG::FILE::DXT1block));

				PG::FILE::decompressS3(img.width, img.height, blocks, imageOut);

			}
			else if (img.type == tx2Type::DXT4 || img.type == tx2Type::DXT5) {
				//DXT5
				instream->skip(img.colortableSize * 4);

				const unsigned int number_of_blocks_width = (img.width / 4);
				const unsigned int number_of_blocks_height = (img.height / 4);
				const unsigned int number_of_blocks_4x4 = number_of_blocks_width * number_of_blocks_height;

				//wrong file size?
				if (instream->size() < (number_of_blocks_4x4 * 16 + 16)) {
					PG_ERROR_STREAM("File too small! (" << instream->size() << " < " << (number_of_blocks_4x4 * 16 + 16) << ")");
					return true;
				}

				std::vector<PG::FILE::DXT5block> blocks(number_of_blocks_4x4);
				instream->read((char*)&blocks[0], number_of_blocks_4x4 * sizeof(PG::FILE::DXT5block));


				PG::FILE::decompressS3(img.width, img.height, blocks, imageOut);

			}
			else if (img.type == tx2Type::BGRA) {
				//BGRA
				if (instream->size() < (img.width*img.height * 4 + 16)) {
					PG_ERROR_STREAM("File too small! (" << instream->size() << " < " << (img.width*img.height * 4 + 16) << ")");
					return true;
				}

				imageOut.resize(img.width, img.height);
				instream->read((char*)&imageOut[0], imageOut.size() * sizeof(PG::UTIL::rgba));

				//flip r and b
				for (PG::UTIL::rgba& rgba : imageOut) {
					const unsigned char r = rgba.r;
					rgba.r = rgba.b;
					rgba.b = r;
				}
			}
			else if (img.type == tx2Type::COLORTABLE_BGRA16 || img.type == tx2Type::COLORTABLE_RGBA16) {
				//lookup table RGBA with max 16 values
				if (img.colortableSize > 16) {
					PG_WARN_STREAM("Color table is too big!  (" << img.colortableSize << " but should be 16)");
				}

				const unsigned int total_number_of_bytes = (img.width*img.height) / 2; // every byte holds two table values
				if (instream->size() < (total_number_of_bytes + img.colortableSize + 16)) {
					PG_ERROR_STREAM("File too small! (" << instream->size() << " < " << (total_number_of_bytes + img.colortableSize + 16) << ") ");
					return true;
				}

				imageOut.resize(img.width, img.height);


				for (unsigned int i = 0; i < total_number_of_bytes; ++i) {
					const char c = instream->readUnsignedChar();

					const unsigned int pos = i * 2;
					imageOut[pos] = img.colortables[0][c & 0x0F];
					imageOut[pos + 1] = img.colortables[0][(c >> 4) & 0x0F];
				}

			}
			else if (img.type == tx2Type::COLORTABLE_BGRA256 || img.type == tx2Type::COLORTABLE_RGBA256) {
				//lookup table RGBA

				if (img.colortableSize > 256) {
					PG_ERROR_STREAM("Color table is too big!");
					return true;
				}

				const unsigned int total_number_of_bytes = (img.width*img.height);
				if (instream->size() < (total_number_of_bytes + img.colortableSize + 16)) {
					PG_ERROR_STREAM("File too small! (" << instream->size() << " < " << (total_number_of_bytes + img.colortableSize + 16) << ") ");
					return true;
				}


				imageOut.resize(img.width, img.height);

				for (unsigned int i = 0; i < total_number_of_bytes; i++) {
					imageOut[i] = img.colortables[0][instream->readUnsignedChar()];
				}


			}
			else {
				PG_ERROR_STREAM("Unknown format not supported!");
				return true;
			}

			return false;
		}


		bool decompressTX2(PG::STREAM::In* instream, PG::UTIL::RGBAImage& imageOut) {
			if (instream == nullptr) {
				PG_ERROR_STREAM("InStream is nullptr!");
				return true;
			}
			if (instream->size() < 16) {
				PG_ERROR_STREAM("File is smaller then 16 byte!");
				return true;
			}

			tx2Image::tx2header header;
			if (readTX2Header(instream, header))
				return true;


			return decompressTX2(instream, header, imageOut);
		}

		bool decompressTX2(const tx2Image& img, PG::UTIL::RGBAImage& imageOut) {

			PG::STREAM::InByteArray reader(&img.data[0], img.data.size());
			const bool s = decompressTX2((PG::STREAM::In*) &reader, img.header, imageOut);
			return s;
		}

		bool decompressTX2(const PG::UTIL::File& file, PG::UTIL::RGBAImage& imageOut) {
			try {
				PG::STREAM::InByteFile reader(file);
				const bool s = decompressTX2((PG::STREAM::In*) &reader, imageOut);
				reader.close();
				return s;
			}
			catch (PG::UTIL::Exception& e) {
				PG_ERROR_STREAM(e.what());
				PG_ERROR_STREAM("Couldn't read tx2 image.")
					return true;
			}
			catch (...) {
				PG_ERROR_STREAM("Couldn't read tx2 image.")
					return true;
			}

			return false;

		}

		bool decompressTX2(const std::string& file, PG::UTIL::RGBAImage& imageOut) {
			return decompressTX2(PG::UTIL::File(file), imageOut);
		}

		bool decompressTX2(const std::vector<char>& bytesIn, PG::UTIL::RGBAImage& imageOut) {
			PG::STREAM::InByteArray reader(&bytesIn[0], bytesIn.size());
			const bool s = decompressTX2((PG::STREAM::In*) &reader, imageOut);
			return s;
		}

		bool decompressTX2(const char* bytesIn, unsigned int lenghtIn, PG::UTIL::RGBAImage& imageOut) {
			PG::STREAM::InByteArray reader(bytesIn, lenghtIn);
			const bool s = decompressTX2((PG::STREAM::In*) &reader, imageOut);
			return s;
		}



		bool compressTX2(const PG::UTIL::RGBAImage& imageIn, tx2Type compressionTypeIn, std::vector<char>& bytesOut) {
			
			if (compressionTypeIn == DXT1) {
				const unsigned short width = imageIn.getWidth();
				const unsigned short height = imageIn.getHeight();
				const unsigned int number_of_blocks_width = (width / 4);
				const unsigned int number_of_blocks_height = (height / 4);
				const unsigned int number_of_blocks_4x4 = number_of_blocks_width * number_of_blocks_height;

				bytesOut.resize(16 + number_of_blocks_4x4 * 8);

				memcpy(&bytesOut[0], &width, 2);
				memcpy(&bytesOut[2], &height, 2);

				bytesOut[6] = log10(width) / 0.301029995;
				bytesOut[7] = log10(height) / 0.301029995;
				const unsigned int size = width * height;
				memcpy(&bytesOut[12], &size, 4);
				std::vector<DXT1block> blocks;

				if (PG::FILE::compressS3(imageIn, blocks)) {
					PG_ERROR_STREAM("Couldn't compress image.");
					return true;
				}
				memcpy(&bytesOut[16], &blocks[0], number_of_blocks_4x4 * 8);
			}
			else if (compressionTypeIn == DXT4 || compressionTypeIn == DXT5) {
				const unsigned short width = imageIn.getWidth();
				const unsigned short height = imageIn.getHeight();
				const unsigned int number_of_blocks_width = (width / 4);
				const unsigned int number_of_blocks_height = (height / 4);
				const unsigned int number_of_blocks_4x4 = number_of_blocks_width * number_of_blocks_height;

				bytesOut.resize(16 + number_of_blocks_4x4 * 16);

				memcpy(&bytesOut[0], &width, 2);
				memcpy(&bytesOut[2], &height, 2);
				bytesOut[4] = 0x02;
				bytesOut[6] = log10(width) / 0.301029995;
				bytesOut[7] = log10(height) / 0.301029995;
				const unsigned int dimantion = width * 4 * 16;
				memcpy(&bytesOut[12], &dimantion, 4);
				std::vector<DXT5block> blocks;

				if (PG::FILE::compressS3(imageIn, blocks)) {
					PG_ERROR_STREAM("Couldn't compress image.");
					return true;
				}
				memcpy(&bytesOut[16], &blocks[0], number_of_blocks_4x4 * 16);
			}
			else if (compressionTypeIn == BGRA) {
				const unsigned short width = imageIn.getWidth();
				const unsigned short height = imageIn.getHeight();
				const unsigned int size = width * height * 4;
				bytesOut.resize(16 + size);

				memcpy(&bytesOut[0], &width, 2);
				memcpy(&bytesOut[2], &height, 2);
				bytesOut[4] = 0x03;
				bytesOut[6] = log10(width) / 0.301029995;
				bytesOut[7] = log10(height) / 0.301029995;
				const unsigned int dimantion = width * height;
				memcpy(&bytesOut[12], &dimantion, 4);

				unsigned int i = 16;
				for (const PG::UTIL::rgba& color : imageIn) {
					bytesOut[i] = color.b;
					bytesOut[i + 1] = color.g;
					bytesOut[i + 2] = color.r;
					bytesOut[i + 3] = color.a;
					i += 4;
				}

			}
			else {
				PG_ERROR_STREAM("Compression type not supported, please choose another one.");
				return true;
			}

			return false;
		}


		bool compressTX2(const PG::UTIL::RGBAImage& imageIn, tx2Type compressionTypeIn, tx2Image& imageOut) {
			imageOut.header.type = compressionTypeIn;
			if (compressionTypeIn == DXT1) {
				const unsigned short width = imageIn.getWidth();
				const unsigned short height = imageIn.getHeight();
				const unsigned int number_of_blocks_width = (width / 4);
				const unsigned int number_of_blocks_height = (height / 4);
				const unsigned int number_of_blocks_4x4 = number_of_blocks_width * number_of_blocks_height;

				imageOut.data.resize(number_of_blocks_4x4 * 8);
				imageOut.header.width = width;
				imageOut.header.height = height;

				const unsigned int size = width * height;

				std::vector<DXT1block> blocks;

				if (PG::FILE::compressS3(imageIn, blocks)) {
					PG_ERROR_STREAM("Couldn't compress image.");
					imageOut.header.type = TX2ERROR;
					return FAILURE;
				}
				memcpy(&imageOut.data[0], &blocks[0], number_of_blocks_4x4 * 8);
				//clean up
				imageOut.header.colortables.clear();
				imageOut.header.colortableSize = 0;
			}
			else if (compressionTypeIn == DXT5) {
				const unsigned short width = imageIn.getWidth();
				const unsigned short height = imageIn.getHeight();
				const unsigned int number_of_blocks_width = (width / 4);
				const unsigned int number_of_blocks_height = (height / 4);
				const unsigned int number_of_blocks_4x4 = number_of_blocks_width * number_of_blocks_height;

				imageOut.data.resize(number_of_blocks_4x4 * 16);
				imageOut.header.width = width;
				imageOut.header.height = height;

				std::vector<DXT5block> blocks;

				if (PG::FILE::compressS3(imageIn, blocks)) {
					PG_ERROR_STREAM("Couldn't compress image.");
					imageOut.header.type = TX2ERROR;
					return FAILURE;
				}
				memcpy(&imageOut.data[0], &blocks[0], number_of_blocks_4x4 * 16);
				//clean up
				imageOut.header.colortables.clear();
				imageOut.header.colortableSize = 0;
			}
			else if (compressionTypeIn == BGRA || compressionTypeIn == FONT) {
				imageOut.header.type = BGRA;
				imageOut.header.width = imageIn.getWidth();
				imageOut.header.height = imageIn.getHeight();
				if(compressionTypeIn == FONT)
					imageOut.header.colortableSize = 16;
				imageOut.data.resize(imageOut.header.width*imageOut.header.height * 4);

				unsigned int i = 0;
				for (const PG::UTIL::rgba& color : imageIn) {
					imageOut.data[i] = color.b;
					imageOut.data[i + 1] = color.g;
					imageOut.data[i + 2] = color.r;
					imageOut.data[i + 3] = color.a;
					i += 4;
				}
				//clean up
				imageOut.header.colortables.clear();
			}
			else if (compressionTypeIn == COLORTABLE_RGBA256 || compressionTypeIn == COLORTABLE_BGRA256) {
				PG::UTIL::RGBAImage imgReduced = imageIn;
				//PG::UTIL::reduceColors(imgReduced,256);


				std::vector<PG::UTIL::rgba> colortable;
				colortable.reserve(256);
				imageOut.header.type = compressionTypeIn;
				imageOut.header.width = imgReduced.getWidth();
				imageOut.header.height = imgReduced.getHeight();

				for (const PG::UTIL::rgba& color : imgReduced) {
					bool found = false;
					for (const PG::UTIL::rgba& color2 : colortable)
						if (color == color2) {
							found = true;
							break;
						}
					if (colortable.size() >= 256) {
						PG_INFO_STREAM("There are more then 256 colors inside color table!");
						break;
					}
					if (!found) colortable.push_back(color);
				}

				colortable.resize(256);

				imageOut.data.resize(imageOut.header.width*imageOut.header.height);
				imageOut.header.colortableSize = 256;
				imageOut.header.colortables.insert(imageOut.header.colortables.begin(), colortable);

				auto it = imageOut.data.begin();
				for (const PG::UTIL::rgba& color : imgReduced) {
					unsigned int i = 0;
					for (unsigned int a = 0; a < colortable.size(); a++) {
						if (color == colortable[a]) {
							i = a;
							break;
						}
					}
					(*it) = i;
					it++;
				}
			}
			else if (compressionTypeIn == COLORTABLE_RGBA16 || compressionTypeIn == COLORTABLE_BGRA16) {
				PG::UTIL::RGBAImage imgReduced = imageIn;
				//PG::UTIL::reduceColors(imgReduced,16);

				std::vector<PG::UTIL::rgba> colortable;
				colortable.reserve(16);
				imageOut.header.width = imgReduced.getWidth();
				imageOut.header.height = imgReduced.getHeight();

				for (const PG::UTIL::rgba& color : imgReduced) {
					bool found = false;
					for (const PG::UTIL::rgba& color2 : colortable)
						if (color == color2) {
							found = true;
							break;
						}
					if (colortable.size() >= 16) {
						PG_INFO_STREAM("There are more then 16 colors inside color table!");
						break;
					}
					if (!found) colortable.push_back(color);
				}

				colortable.resize(16);

				imageOut.data.resize(imageOut.header.width*imageOut.header.height*0.5);
				imageOut.header.colortableSize = 16;
				imageOut.header.colortables.insert(imageOut.header.colortables.begin(), colortable);

				unsigned int i = 0;
				for (char& c : imageOut.data) {
					const PG::UTIL::rgba& color1 = imgReduced[i];
					const PG::UTIL::rgba& color2 = imgReduced[i + 1];
					unsigned int i1 = 0;
					unsigned int i2 = 0;
					for (unsigned int a = 0; a < colortable.size(); a++) {
						if (color1 == colortable[a]) {
							i1 = a;
							break;
						}
					}
					for (unsigned int a = 0; a < colortable.size(); a++) {
						if (color2 == colortable[a]) {
							i2 = a;
							break;
						}
					}

					c = (i1 & 0x0F) | ((i2 << 4) & 0xF0);
					i += 2;
				}

			}
			else {
				PG_ERROR_STREAM("Compression type not supported, please choose another one.");
				imageOut.header.type = TX2ERROR;
				return FAILURE;
			}

		

			return SUCCESS;
		}

		tx2Image::~tx2Image() {}

		bool tx2Image::save(const std::string& file) const {
			if (file.empty()) return FAILURE;
			try {
				PG::STREAM::OutByteFile writer(file);
				if (!writer.isOpen()) return FAILURE;

				writer.writeShort(header.width);
				writer.writeShort(header.height);
				writer.writeShort(header.type);

				writer.writeChar(log10(header.width) / 0.301029995);
				writer.writeChar(log10(header.height) / 0.301029995);
				writer.writeShort(header.colortableSize);
				writer.writeShort(header.colortables.size());
				
				writer.writeInt(65536);

				for (const ColorTable& table : header.colortables) {
					if (header.type == COLORTABLE_BGRA16 || header.type == COLORTABLE_BGRA256) {
						for (const PG::UTIL::rgba& color : table) {
							writer.writeChar(color.b);
							writer.writeChar(color.g);
							writer.writeChar(color.r);
							writer.writeChar(color.a);
						}
					}
					else
						writer.write((char*)&table[0].r, table.size() * sizeof(PG::UTIL::rgba));
				}

				writer.write((char*)&data[0], data.size());

				writer.close();
			}
			catch (PG::UTIL::Exception& e) {
				PG_ERROR_STREAM(e.what());
				PG_ERROR_STREAM("Couldn't write tx2 image.")
					return FAILURE;
			}
			catch (...) {
				PG_ERROR_STREAM("Couldn't write tx2 image.")
					return FAILURE;
			}


			return SUCCESS;
		}
		bool tx2Image::open(const std::string& file) {
			if (file.empty()) return FAILURE;


			try {
				PG::STREAM::InByteFile reader(file);
				if (!reader.isopen()) return FAILURE;
				readTX2Header(&reader, header);

				switch (header.type) {
				case tx2Type::DXT1:
				{
					const unsigned int number_of_blocks_width = (header.width / 4);
					const unsigned int number_of_blocks_height = (header.height / 4);
					const unsigned int number_of_blocks_4x4 = number_of_blocks_width * number_of_blocks_height;

					//wrong file size?
					if (reader.size() < (number_of_blocks_4x4 * 8 + 16)) {
						PG_ERROR_STREAM("File too small! (" << reader.size() << " < " << (number_of_blocks_4x4 * 8 + 16) << ")");
						return true;
					}

					data.resize(number_of_blocks_4x4 * sizeof(PG::FILE::DXT1block));
				}
				break;
				case tx2Type::DXT4:
				case tx2Type::DXT5:
				{
					const unsigned int number_of_blocks_width = (header.width / 4);
					const unsigned int number_of_blocks_height = (header.height / 4);
					const unsigned int number_of_blocks_4x4 = number_of_blocks_width * number_of_blocks_height;

					//wrong file size?
					if (reader.size() < (number_of_blocks_4x4 * 16 + 16)) {
						PG_ERROR_STREAM("File too small! (" << reader.size() << " < " << (number_of_blocks_4x4 * 16 + 16) << ")");
						return true;
					}

					data.resize(number_of_blocks_4x4 * sizeof(PG::FILE::DXT5block));

				}
				break;
				case tx2Type::BGRA:
				{
					data.resize(header.width*header.height * sizeof(PG::FILE::rgba));
				}
				break;
				case tx2Type::COLORTABLE_BGRA16:
				case tx2Type::COLORTABLE_RGBA16:
				{
					data.resize(header.width*header.height*0.5);
				}
				break;
				case tx2Type::COLORTABLE_BGRA256:
				case tx2Type::COLORTABLE_RGBA256:
				{
					data.resize(header.width*header.height);
				}
				break;
				default:
				{
					reader.close();
					return FAILURE;
				}
				break;
				}

				reader.read((char*)&data[0], data.size());

				reader.close();
			}
			catch (PG::UTIL::Exception& e) {
				PG_ERROR_STREAM(e.what());
				PG_ERROR_STREAM("Couldn't read tx2 image.")
					return FAILURE;
			}
			catch (...) {
				PG_ERROR_STREAM("Couldn't read tx2 image.")
					return FAILURE;
			}

			return SUCCESS;
		}

		unsigned int tx2Image::getRGBADataSize() const {
			return header.width*header.height * sizeof(PG::UTIL::rgba);
		}

		void tx2Image::getRGBAData(char* rgbaOut, unsigned short colortable) const {
			switch (header.type) {
			case tx2Type::DXT1:
			{
				const unsigned int number_of_blocks_width = (header.width / 4);
				const unsigned int number_of_blocks_height = (header.height / 4);
				const unsigned int number_of_blocks_4x4 = number_of_blocks_width * number_of_blocks_height;

				PG::FILE::decompressS3<PG::FILE::DXT1block>(header.width, header.height, data, rgbaOut);
			}
			break;
			case tx2Type::DXT4:
			case tx2Type::DXT5:
			{

				const unsigned int number_of_blocks_width = (header.width / 4);
				const unsigned int number_of_blocks_height = (header.height / 4);
				const unsigned int number_of_blocks_4x4 = number_of_blocks_width * number_of_blocks_height;

				PG::FILE::decompressS3<PG::FILE::DXT5block>(header.width, header.height, data, rgbaOut);
			}
			break;
			case tx2Type::BGRA:
			{
				for (unsigned int i = 0; i < data.size(); i += 4) {
					rgbaOut[i] = data[i + 2];
					rgbaOut[i + 1] = data[i + 1];
					rgbaOut[i + 2] = data[i];
					rgbaOut[i + 3] = data[i + 3];
				}
			}
			break;
			case tx2Type::COLORTABLE_RGBA16:
			case tx2Type::COLORTABLE_BGRA16:
			{
				if (colortable >= header.colortables.size()) return;
				const ColorTable& table = header.colortables[colortable];
				for (unsigned int i = 0; i < data.size(); ++i) {
					const unsigned char c = data[i];

					const unsigned int pos1 = i * 2 * sizeof(PG::UTIL::rgba);
					const unsigned int pos2 = pos1 + sizeof(PG::UTIL::rgba);
					const PG::UTIL::rgba& co1 = table[c & 0x0F];
					const PG::UTIL::rgba& co2 = table[(c >> 4) & 0x0F];
					rgbaOut[pos1] = co1.r;
					rgbaOut[pos1 + 1] = co1.g;
					rgbaOut[pos1 + 2] = co1.b;
					rgbaOut[pos1 + 3] = co1.a;

					rgbaOut[pos2] = co2.r;
					rgbaOut[pos2 + 1] = co2.g;
					rgbaOut[pos2 + 2] = co2.b;
					rgbaOut[pos2 + 3] = co2.a;
				}
			}
			break;
			case tx2Type::COLORTABLE_RGBA256:
			case tx2Type::COLORTABLE_BGRA256:
			{
				if (colortable >= header.colortables.size()) return;

				const ColorTable& table = header.colortables[colortable];
				for (unsigned int i = 0; i < data.size(); ++i) {
					const unsigned int pos = i * sizeof(PG::UTIL::rgba);
					assert_Test("Color table offset out of bound!", (unsigned char)data[i] >= 256);
					assert_Test(SSTR("Color table offset out of table bound! (" << (unsigned char)data[i] << " != " << table.size() << ")"), ((unsigned char)data[i]) >= table.size());
					const PG::UTIL::rgba& co = table[(unsigned char)data[i]];
					rgbaOut[pos] = co.r;
					rgbaOut[pos + 1] = co.g;
					rgbaOut[pos + 2] = co.b;
					rgbaOut[pos + 3] = co.a;
				}
			}
			break;
			default:
				break;
			}
		}


		void tx2Image::setWithRGBA(unsigned short width, unsigned short height, const char* rgbaIn) {
			clear(tx2Type::BGRA);
			header.width = width;
			header.height = height;

			data.resize(width*height * 4);
			for (unsigned int i = 0; i < data.size(); i += 4) {
				data[i] = rgbaIn[i + 2];
				data[i + 1] = rgbaIn[i + 1];
				data[i + 2] = rgbaIn[i];
				data[i + 3] = rgbaIn[i + 3];
			}

		}

		bool tx2Image::convertTo(tx2Type compressionTypeIn, unsigned int colorTableIndex) {
			if (compressionTypeIn == tx2Type::TX2ERROR || compressionTypeIn == header.type || getWidth() == 0 || getHeight() == 0) return FAILURE;

			if ((compressionTypeIn == COLORTABLE_RGBA16 || compressionTypeIn == COLORTABLE_BGRA16) && (header.type == COLORTABLE_RGBA16 || header.type == COLORTABLE_BGRA16)) {
				header.type = compressionTypeIn;
			}
			else if ((compressionTypeIn == COLORTABLE_RGBA256 || compressionTypeIn == COLORTABLE_BGRA256) && (header.type == COLORTABLE_RGBA256 || header.type == COLORTABLE_BGRA256)) {
				header.type = compressionTypeIn;

			}
			else if ((compressionTypeIn == COLORTABLE_RGBA256 || compressionTypeIn == COLORTABLE_BGRA256) && (header.type == COLORTABLE_RGBA16 || header.type == COLORTABLE_BGRA16)) {
				header.type = compressionTypeIn;
				header.colortableSize = 256;
				for (ColorTable& table : header.colortables)
					table.resize(256);

				std::vector<char> newdata(getWidth()*getHeight());
				for (unsigned int i = 0; i < data.size(); ++i) {
					const char c = data[i];

					const unsigned int pos = i * 2;
					newdata[pos] = c & 0x0F;
					newdata[pos + 1] = (c >> 4) & 0x0F;
				}
				data = newdata;

			}
			else {
				PG::UTIL::RGBAImage rgbaImage(getWidth(), getHeight());
				if (colorTableIndex >= header.colortables.size()) colorTableIndex = 0;
				getRGBAData((char*)&rgbaImage[0].r, colorTableIndex);
				compressTX2(rgbaImage, compressionTypeIn, *this);
			}

			return SUCCESS;

		}

		void tx2Image::clear(tx2Type type) {
			data.clear();
			header.colortableSize = 0;
			header.colortables.clear();
			header.height = 0;
			header.width = 0;
			header.type = type;
		}


	} /* namespace FILE */
} /* namespace PG */
