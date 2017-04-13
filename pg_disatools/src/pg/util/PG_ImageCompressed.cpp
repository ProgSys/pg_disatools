/*
 * PG_ImageCompressed.cpp
 *
 *  Created on: 14.01.2017
 *      Author: ProgSys
 */

#include <pg/util/PG_ImageCompressed.h>
#include <pg/files/PG_S3Compression.h>
#include <pg/util/PG_Exception.h>


#include <pg/stream/PG_StreamIn.h>
#include <pg/stream/PG_StreamInByteFile.h>
#include <pg/stream/PG_StreamInByteArray.h>

#include <pg/stream/PG_StreamOut.h>
#include <pg/stream/PG_StreamOutByteFile.h>

#include <pg/util/PG_ImageUtil.h>
#include <pg/files/PG_IMY.h>
#include <pg/files/PG_ImageFiles.h>

namespace PG {
namespace UTIL {

ImageCompressed::ImageCompressed() {}

bool ImageCompressed::save(const std::string& path){
	PG::STREAM::OutByteFile writer(path);
	if(!writer.isOpen()) return FAILURE;
	bool b = save(&writer);
	writer.close();
	return b;
}

bool ImageCompressed::open(const std::string& path){
	PG::STREAM::InByteFile reader(path);
	if(!reader.isopen()) return FAILURE;
	bool b = open(&reader);
	reader.close();
	return b;
}

bool ImageCompressed::open(const PG::UTIL::RGBAImage& image_in, ImageCompressed::dataType type){
	return open((char*)&image_in[0], image_in.getWidth(), image_in.getHeight(), type);
}

ImageCompressed::~ImageCompressed() {}


TX2ImageB::TX2ImageB(){

}
TX2ImageB::TX2ImageB(const std::string& path){
	ImageCompressed::open(path);
}
TX2ImageB::TX2ImageB(PG::STREAM::In* in){
	open(in);
}
TX2ImageB::TX2ImageB(PG::UTIL::RGBAImage& image_in, ImageCompressed::dataType type){
	ImageCompressed::open(image_in, type);
}

TX2ImageB::TX2ImageB(const char* image_in, unsigned int width, unsigned int height, ImageCompressed::dataType type){
	open(image_in,width,height, type);
}

bool TX2ImageB::save(PG::STREAM::Out* out){
	if(m_data.empty()) {
		PG_ERROR_STREAM("No data!");
		return FAILURE;
	}

	m_header.number_of_color_tabels = m_colortables.size();
	if(!m_colortables.empty()) m_header.color_table_size = m_colortables[0].size();

	out->write((char*)&m_header, sizeof(PG::FILE::tx2HeaderOrg));

	for(PG::FILE::ColorTable& table: m_colortables){
		out->write((char*)&table[0], table.size()*sizeof(PG::UTIL::rgba));
	}

	out->write((char*)&m_data, m_data.size());
	return SUCCESS;
}


bool TX2ImageB::open(PG::STREAM::In* in){
	in->read((char*)&m_header, sizeof(PG::FILE::tx2HeaderOrg));
	if(m_header.height > 10000 || m_header.width > 10000){
		PG_ERROR_STREAM("Height or Width is too big!");
		return FAILURE;
	}
	if(!(m_header.type == 0 || m_header.type == 2 || m_header.type == 3 ||
			m_header.type == 16 || m_header.type == 17 || m_header.type == 256 || m_header.type == 257)){
		PG_ERROR_STREAM("TX2 type '"<<(int)m_header.type<<"' is not supported!");
		return FAILURE;
	}

	m_colortables.resize(m_header.number_of_color_tabels);
	for(PG::FILE::ColorTable& table: m_colortables){
		table.resize(m_header.color_table_size);
		in->read((char*)&table[0], table.size()*sizeof(PG::UTIL::rgba));
	}

	const unsigned int restSize = in->size() - in->pos();
	m_data.resize(restSize);
	in->read((char*)&m_data[0], m_data.size());

	return SUCCESS;
}

bool TX2ImageB::open(const char* image_in, unsigned int width, unsigned int height, ImageCompressed::dataType type){
	switch (type) {
		case ImageCompressed::BGRA:
		{
			PG_INFO_STREAM("Convert to BGRA")
			m_header.type = 3;
			m_header.width = width;
			m_header.height = height;
			m_header.power_of_width = log2(m_header.width);
			m_header.power_of_height = log2(m_header.height);

			m_header.color_table_size = 0;
			m_header.number_of_color_tabels = 0;

			m_header.uncompressedSize = m_header.width*m_header.height*4;

			m_data.clear();
			m_data.resize(m_header.uncompressedSize);

			for(unsigned int i = 0; i < m_data.size(); i+=4){
				m_data[i  ] = image_in[i+2];
				m_data[i+1] = image_in[i+1];
				m_data[i+2] = image_in[i  ];
				m_data[i+3] = image_in[i+3];
			}

		}
			break;
		case ImageCompressed::DXT1:
		{
			m_header.type = 0;
			m_header.width = width;
			m_header.height = height;
			m_header.power_of_width = log2(m_header.width);
			m_header.power_of_height = log2(m_header.height);

			m_header.color_table_size = 0;
			m_header.number_of_color_tabels = 0;

			m_header.uncompressedSize = m_header.width*m_header.height*4;

			m_data.clear();

			std::vector<PG::FILE::DXT1block> blocks;
			//PG::UTIL::RGBAImage img(width,height);
			//memcpy(&img[0],image_in, width*height*4);
			//PG::FILE::saveTGA("D:/Users/ProgSys/ownCloudHome/Public/Work/Modding/Criminal Girls Invite Only/IMY/DXT_comptest.tga", img);
			//if(PG::FILE::compressS3(width,height,image_in,blocks)){
			if(PG::FILE::compressS3(width,height,image_in,blocks)){
				PG_ERROR_STREAM("Couldn't compress image.");
				return FAILURE;
			}

			const unsigned int number_of_blocks_width = (m_header.width/4);
			const unsigned int number_of_blocks_height = (m_header.height/4);
			const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;
			m_data.resize(number_of_blocks_4x4*8);
			memcpy(&m_data[0], &blocks[0], number_of_blocks_4x4*8);

		}
			break;
		case ImageCompressed::DXT5:
		{
			m_header.type = 2;
			m_header.width = width;
			m_header.height = height;
			m_header.power_of_width = log2(m_header.width);
			m_header.power_of_height = log2(m_header.height);

			m_header.color_table_size = 0;
			m_header.number_of_color_tabels = 0;

			m_header.uncompressedSize = m_header.width*m_header.height*4;

			const unsigned int number_of_blocks_width = (m_header.width/4);
			const unsigned int number_of_blocks_height = (m_header.height/4);
			const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

			m_data.resize(number_of_blocks_4x4*16);

			std::vector<PG::FILE::DXT5block> blocks;

			if(PG::FILE::compressS3<PG::FILE::DXT5block>(width,height,image_in,blocks)){
				PG_ERROR_STREAM("Couldn't compress image.");
				return FAILURE;
			}
			memcpy(&m_data[0], &blocks[0], number_of_blocks_4x4*16);
		}
		break;
		case ImageCompressed::COLORTABLE_RGBA16:
		case ImageCompressed::COLORTABLE_BGRA16:
		{
			PG_INFO_STREAM("COLORTABLE_RGBA16");
			m_header.type = (type == COLORTABLE_BGRA16)? 16 : 17;
			m_header.width = width;
			m_header.height = height;
			m_header.power_of_width = log2(m_header.width);
			m_header.power_of_height = log2(m_header.height);

			m_header.color_table_size = 16;
			m_header.number_of_color_tabels = 1;

			m_header.uncompressedSize = m_header.width*m_header.height*4;

			//std::map<PG::UTIL>
		}
		break;
		default:
		{
			PG_ERROR_STREAM("Target format not supported!");
			return FAILURE;
		}
			break;
	}
	return SUCCESS;
}

bool TX2ImageB::getRGBA(char* image_out, unsigned int colorTableID) const{
	return PG::FILE::decompressTX2(m_header, m_colortables, m_data, image_out, colorTableID);
}
bool TX2ImageB::getRGBA(PG::UTIL::RGBAImage& image_out, unsigned int colorTableID) const{
	image_out.resize(getWidth(),getHeight());
	return getRGBA( (char*)&image_out[0],colorTableID);
}

ImageCompressed::fileType TX2ImageB::getFileType() const{
	return ImageCompressed::TX2;
}
ImageCompressed::dataType TX2ImageB::getDataType() const{
	switch (m_header.type) {
		case 0: return DXT1;
		case 2: return DXT5;
		case 3: return BGRA;
		case 16: return COLORTABLE_BGRA16;
		case 17: return COLORTABLE_RGBA16;
		case 256: return COLORTABLE_BGRA256;
		case 257: return COLORTABLE_RGBA256;
	}
	throw_Exception("Unknown file type!");
}

void TX2ImageB::setDataType(dataType type){
	switch (type) {
		case DXT1: {m_header.type = 0;} break;
		case DXT5: {m_header.type = 2;} break;
		case BGRA: {m_header.type = 3;} break;
		case COLORTABLE_BGRA16: {m_header.type = 16;} break;
		case COLORTABLE_RGBA16: {m_header.type = 17;} break;
		case COLORTABLE_BGRA256: {m_header.type = 256;} break;
		case COLORTABLE_RGBA256: {m_header.type = 267;} break;
	}
}

unsigned int TX2ImageB::getWidth() const{
	return m_header.width;
}
unsigned int TX2ImageB::getHeight() const{
	return m_header.height;
}

TX2ImageB::~TX2ImageB(){

}

//////////////////// IMY /////////////////////

IMYImage::IMYImage(){}
IMYImage::IMYImage(const std::string& path){
	ImageCompressed::open(path);
}
IMYImage::IMYImage(PG::STREAM::In* in){
	open(in);
}
IMYImage::IMYImage(const PG::UTIL::RGBAImage& image_in, ImageCompressed::dataType type){
	ImageCompressed::open(image_in,type);
}

IMYImage::IMYImage(const char* image_in, unsigned int width, unsigned int height, ImageCompressed::dataType type){
	open(image_in,width,height,type);
}


bool IMYImage::save(PG::STREAM::Out* out){
	out->writeString("IMY\0");
	out->write((char*)&m_header, sizeof(PG::FILE::imyHeader));
	{
		char zeros[16] = {};
		out->write((char*)&zeros, 16);
	}

	out->write((char*)&m_data[0], m_data.size());
	return SUCCESS;
}

bool IMYImage::open(PG::STREAM::In* in){
	const std::string magic = in->readString(3);
	if(magic != "IMY"){
		PG_INFO_STREAM("Magic number isn't 'IMY' but '"<<magic<<"'!");
		return FAILURE;
	}

	return PG::FILE::readIMYData(in, m_header, m_colortables, m_data);
}

bool IMYImage::open(const char* image_in, unsigned int width, unsigned int height, ImageCompressed::dataType type){
	return FAILURE;
}

bool IMYImage::getRGBA(char* image_out, unsigned int colorTableID) const{
	return  PG::FILE::decompressIMY2(m_header, m_colortables,
			m_data, image_out, colorTableID);
}
bool IMYImage::getRGBA(PG::UTIL::RGBAImage& image_out, unsigned int colorTableID) const{
	return  PG::FILE::decompressIMY2(m_header, m_colortables,
			m_data, image_out, colorTableID);
}

ImageCompressed::fileType IMYImage::getFileType() const{
	return ImageCompressed::IMY;
}
ImageCompressed::dataType IMYImage::getDataType() const{
	return ImageCompressed::COLORTABLE_RGBA256;
}

void IMYImage::setDataType(dataType type){
	return;
}

unsigned int IMYImage::getWidth() const{
	return m_header.width;
}
unsigned int IMYImage::getHeight() const{
	return m_header.height;
}

IMYImage::~IMYImage(){

}

ImageCompressed* convertTo(ImageCompressed* img, ImageCompressed::fileType type, ImageCompressed::dataType data){

	return nullptr;
}


} /* namespace UTIL */
} /* namespace PG */
