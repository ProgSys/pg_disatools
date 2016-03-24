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
#include <pg/files/PG_StartDAT.h>

#include <algorithm>
#include <pg/stream/PG_StreamInByteFile.h>
#include <pg/stream/PG_StreamOutByteFile.h>
#include <pg/util/PG_StringUtil.h>
#include <pg/util/PG_Exception.h>
#include <pg/files/PG_TX2.h>

namespace PG {
namespace FILE {

inline void skipSCVColumn(PG::STREAM::InByteFile& reader){
	char c;
	while( !reader.eof() && (c = reader.readChar()) != ',' && c != '\n'  ){}
}

inline std::string readSCVColumn(PG::STREAM::InByteFile& reader){
	std::string str;
	char c;
	while( !reader.eof() && (c = reader.readChar()) != ','  && c != '\n' ){
		str.push_back(c);
	}
	std::transform(str.begin(), str.end(), str.begin(), ::toupper );
	return str;
}

inline void skipSCVLine(PG::STREAM::InByteFile& reader){
	char c;
	while( !reader.eof() && (c = reader.readChar()) != '\n' ){}
}


StartDAT::StartDAT() {

}

StartDAT::StartDAT(const PG::UTIL::File& file){
	open(file);
}


void StartDAT::readFileNames(){
	std::string filename = m_file.getFile();
	std::transform(filename.begin(), filename.end(), filename.begin(), ::toupper );

	PG::STREAM::InByteFile reader("resources/"+filename+".csv");
	if(reader.isopen()){
		skipSCVLine(reader);

		while(!reader.eof()){
			skipSCVColumn(reader);
			m_namesTable.push_back(readSCVColumn(reader));
			skipSCVLine(reader);
		}

	}
	reader.close();

}

bool StartDAT::insert(const PG::UTIL::File& file){
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No START.DAT file opened.");
		return true;
	}

	if(!file.exists()){
		PG_ERROR_STREAM("File doesn't exist!");
		return true;
	}

	PG::UTIL::File ufile = file.toUpper();
	auto it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [ufile](const fileInfo& info){
		return info.getName() == ufile.getName();
	});


	m_changed = true;

	if(it != m_fileInfos.end()){
		(*it).size = file.size();
		(*it).externalFile = file;
		return false;
	}

	fileInfo info;
	info.name = ufile.getFile();
	info.size = file.size();
	info.offset = 0;
	info.externalFile = file;

	m_fileInfos.push_back(info);
	return false;
}

bool StartDAT::remove(const PG::UTIL::File& file){
	PG::UTIL::File ufile = file.toUpper();

	auto it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [ufile](const fileInfo& info){
		return info.getName() == ufile.getPath();
	});

	if(it != m_fileInfos.end()){
		m_fileInfos.erase(it);
		m_changed = true;
		return false;
	}

	return true;
}

bool StartDAT::save(){
	save(m_file);
}

bool StartDAT::save(const PG::UTIL::File& targetfile){
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No START.DAT file opened.");
		return true;
	}

	//do we overwrtie the original or do we save as?
	bool overwrite = false;
	PG::UTIL::File target;
	if(targetfile == m_file){
		target = PG::UTIL::File(m_file.getPath()+"_buffer");
		overwrite = true;
	}else{
		target = targetfile;
	}

	//newFile infos
	std::vector<fileInfo> fileInfos;
	fileInfos.reserve(m_fileInfos.size());
	char* c = nullptr;

	try{
		PG::STREAM::OutByteFile writer(target);

		writer.writeInt(0); //reserve space for number of files
		//writer.writeInt(131072);
		for(unsigned int i = 0; i < m_fileInfos.size(); ++i){
			writer.writeInt(0);
		}

		for(unsigned int i = 0; i < 4 - (1+m_fileInfos.size())%4; ++i){
			writer.writeInt(0);
		}

		unsigned int header_offset = sizeof(int);

		PG::STREAM::InByteFile reader_dat(m_file);
		auto it = m_fileInfos.begin();
		for(unsigned int i = 0; i < m_fileInfos.size(); ++i){
			const fileInfo& info = (*it);
			const unsigned int offset = writer.getPosition();
			fileInfo info_new;
			info_new.name = info.name;
			info_new.size = info.size;

			if(info.isExternalFile()){
				PG::STREAM::InByteFile reader_file(info.externalFile);
				if(!reader_file.isopen()){
					PG_ERROR_STREAM("Couldn't read external file '"<<info.externalFile<<"'! Skipping it!");
					continue;
				}


				const unsigned int file_size = reader_file.size();

				if(file_size == 0){
					PG_ERROR_STREAM("File '"<<info.externalFile<<"' is empty! Skipping it!");
					continue;
				}

				if(info_new.size != file_size){
					PG_WARN_STREAM("File '"<<info.externalFile<<"' size is different than expected! ("<<info.getSize()<<" != "<<file_size<<")");
					info_new.size = file_size;
				}


				c = new char[file_size];
				reader_file.read(c,file_size);
				reader_file.close();
				writer.write(c,file_size);
				delete c;
				c = nullptr;

			}else{

				reader_dat.seek(info.offset);
				c = new char[info.size];
				reader_dat.read(c,info.size);
				writer.write(c,info.size);
				delete c;
				c = nullptr;


			}

			info_new.offset = offset;
			fileInfos.push_back(info_new);

			//write header
			writer.setPosition(header_offset);
			writer.writeInt(info_new.offset);
			writer.setPosition(info_new.offset+info_new.size);
			header_offset += sizeof(int);


			it++;
		}
		reader_dat.close();

		writer.setPosition(0);
		writer.writeInt(fileInfos.size());
		writer.close();



	}catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't save PSPFS archive! : "<<e.what());
		 target.remove();
		 return true;
	}catch (...) {
		 PG_ERROR_STREAM("Couldn't save PSPFS archive!");
		 target.remove();
		 return true;
	}

	 if(overwrite){
		 m_file.remove();
		 target.rename(m_file.getPath());
		 m_fileInfos = fileInfos;
		 m_changed = false;
	 }

	return false;
}

inline bool isIMY(PG::STREAM::InByteFile& reader){
	return reader.readString(3) == "IMY";
}

inline bool isTX2(PG::STREAM::InByteFile& reader){
	const unsigned short width = reader.readUnsignedShort();
	const unsigned short height = reader.readUnsignedShort();

	const unsigned short type = reader.readUnsignedShort();
	const unsigned short same = reader.readUnsignedShort();

	const unsigned short colorTableSize = reader.readUnsignedShort();
	reader.skip(2);

	if(width == 0 || height == 0)
		return false;
	//if(reader.readUnsignedInt() != 65536) //width*height)
		//return false;

	switch (type) {
		case tx2Type::DXT1:
		{
			if(width%4 != 0 || height%4 != 0 || colorTableSize != 0 )
				return false;
		}
			break;
		case tx2Type::DXT5:
		{
			if(width%4 != 0 || height%4 != 0 || colorTableSize != 0)
				return false;
		}
			break;
		case tx2Type::BGRA:
		{
			if(( colorTableSize != 0 && colorTableSize != 16) )
				return false;
		}
			break;
		case tx2Type::COLORTABLERGBA256:
		{
			if(colorTableSize == 0 || colorTableSize > 500)
				return false;
		}
			break;
		case tx2Type::COLORTABLEBGRA256:
		{
			if(colorTableSize == 0 || colorTableSize > 500)
				return false;
		}
			break;
		case tx2Type::COLORTABLEBGRA16:
		{
			if(colorTableSize != 16  && colorTableSize != 256)
				return false;
		}
			break;
		case tx2Type::COLORTABLERGBA16:
		{
			if(colorTableSize != 16 && colorTableSize != 256)
				return false;
		}
			break;
		default:
			return false;
			break;
	}

	return true;
}

bool StartDAT::open(const PG::UTIL::File& file){
	clear();
	m_file = file;

	if(!m_file.exists() ){
		 PG_ERROR_STREAM("Target file '"<<file<<"' does not exist!");
		 m_file.clear();
		 return true;
	}

	readFileNames();

	try{
		PG::STREAM::InByteFile reader(m_file);
		if(!reader.isopen()) return true;

		const unsigned int file_size = reader.size();
		const unsigned int number_of_files = reader.readUnsignedInt();
		const unsigned int header_size = 4+number_of_files*sizeof(int);

		if(number_of_files > 9000){
			PG_ERROR_STREAM("START.DAT is too big!");
			return true;
		}
		//reader.skip(sizeof(int));

		unsigned int lastOffset = reader.readUnsignedInt();
		for(unsigned int i = 0; i < number_of_files; ++i){
			fileInfo info;
			info.offset = lastOffset;
			if(i != number_of_files-1)
				lastOffset = reader.readUnsignedInt();
			else
				lastOffset = file_size;

			if(lastOffset < info.offset){
				PG_ERROR_STREAM("File offset order is wrong! ("<<lastOffset <<" < "<<info.offset<<") at: "<<reader.pos());
				return true;
			}

			info.size = lastOffset-info.offset;

			if(info.offset < header_size){
				PG_ERROR_STREAM("File offset is smaller then the header! ("<<info.offset <<" < "<<header_size<<")");
				return true;
			}

			if((info.offset+info.size) > file_size){
				PG_ERROR_STREAM("File is outside the range ("<< (info.offset+info.size) << " > " << file_size <<")!");
				return true;
			}

			const unsigned int currentPos = reader.pos();
			if( i < m_namesTable.size() && !m_namesTable[i].empty() ){
				info.name = m_namesTable[i];
				reader.seek(info.offset);
				if(isTX2(reader)){
					if(info.name.getFileExtension() != "TX2")
						info.name = info.name.getName()+".TX2";
				}else
					if(info.name.getFileExtension() == "TX2")
						info.name = info.name.getName()+".NOT";

			}else{
				std::stringstream o;
				o<<"FILE";
				o.fill('0');
				o.width(4);
				o<<i<<".";

				reader.seek(info.offset);

				if(isIMY(reader)){
					o<<"IMY";
				}
				reader.seek(info.offset);
				if(isTX2(reader)){
					o<<"TX2";
				}else
					o<<"DAT";

				info.name = o.str();
			}


			reader.seek(currentPos);


			m_fileInfos.push_back(info);
		}

		reader.close();


		if(m_fileInfos.empty()){
			PG_ERROR_STREAM("No files found inside!");
			clear();
			return true;
		}


	}catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM("Couldn't read given PSPFS file '"<<file<<"': "<<e.what());
		clear();
		return true;
	}catch (...) {
		PG_ERROR_STREAM("Couldn't read given PSPFS file '"<<file<<"'!");
		clear();;
		return true;
	}
	PG_INFO_STREAM("Opening done");


	return false;
}


void StartDAT::clear(){
	m_changed = false;
	m_file.clear();
	m_fileInfos.clear();
	m_namesTable.clear();
}

bool StartDAT::isEmpty() const{
	return m_fileInfos.empty();
}

const PG::UTIL::File& StartDAT::getOpendFile() const{
	return m_file;
}

unsigned int StartDAT::size() const{
	return m_fileInfos.size();
}

bool StartDAT::find(const PG::UTIL::File& file, fileInfo& infoOut) const{
	PG::UTIL::File ufile = file.toUpper();

	auto it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [ufile](const fileInfo& info){
		return info.getName() == ufile.getPath();
	});

	if(it != m_fileInfos.end()){
		infoOut = (*it);
		return true;
	}

	return false;

}

const fileInfo& StartDAT::get(unsigned int index) const{
	return m_fileInfos[index];
}

StartDAT::~StartDAT() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
