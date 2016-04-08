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
#include <pg/files/PG_SpriteSheetDAT.h>

#include <algorithm>
#include <pg/stream/PG_StreamInByteFile.h>
#include <pg/stream/PG_StreamOutByteFile.h>
#include <pg/util/PG_StringUtil.h>
#include <pg/util/PG_Exception.h>
#include <pg/files/PG_FileTests.h>

namespace PG {
namespace FILE {

SpriteSheetDAT::SpriteSheetDAT() {
	// TODO Auto-generated constructor stub

}

SpriteSheetDAT::SpriteSheetDAT(const PG::UTIL::File& file){
	open(file);
}

bool SpriteSheetDAT::save(const PG::UTIL::File& targetfile, PercentIndicator* percent){
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
		for(unsigned int i = 0; i < m_fileInfos.size(); ++i)
			writer.writeInt(0);

		for(unsigned short s: m_chractersIDs)
			writer.writeShort(s);

		unsigned int header_offset = sizeof(int);

		PG::STREAM::InByteFile reader_dat(m_file);
		auto it = m_fileInfos.begin();
		for(unsigned int i = 0; i < m_fileInfos.size(); ++i){
			const fileInfo& info = (*it);
			const unsigned int offset = writer.getPosition();
			fileInfo info_new = info;
			info_new.externalFile.clear();

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

bool SpriteSheetDAT::open(const PG::UTIL::File& file, PercentIndicator* percent){
	clear();
	m_file = file;

	if(!m_file.exists() ){
		 PG_ERROR_STREAM("Target file '"<<file<<"' does not exist!");
		 m_file.clear();
		 return true;
	}


	try{
		PG::STREAM::InByteFile reader(m_file);
		if(!reader.isopen()) return true;

		const unsigned int file_size = reader.size();
		const unsigned int number_of_files = reader.readUnsignedInt();

		const unsigned int header_size = 4+number_of_files*sizeof(int);

		if(number_of_files > 9000){
			PG_ERROR_STREAM("Too many files inside!");
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

			info.fileType = fileInfo::SH;
			m_fileInfos.push_back(info);
		}

		//PG_MARK;
		m_chractersIDs.resize(number_of_files);
		reader.read((char*) &m_chractersIDs[0],m_chractersIDs.size()*sizeof(short));
		//read ids

		auto it = m_fileInfos.begin();
		for(unsigned short s: m_chractersIDs){
			std::stringstream o;
			if(s == 1){
				o<<"0001_LAHARL.SH";
			}else if(s == 2){
				o<<"0002_ETNA.SH";
			}else if(s == 3){
				o<<"0003_FLONNE.SH";
			}else if(s == 4){
				o<<"0004_GORDON.SH";
			}else if(s == 5){
				o<<"0005_JENNIFER.SH";
			}else if(s == 6){
				o<<"0006_MOD-KRISAN_PLEINAIR.SH";
			}else if(s == 1010){
				o<<"1010_BRAWLER_MALE.SH";
			}else if(s == 1020){
				o<<"1020_BRAWLER_FEMALE.SH";
			}else if(s == 1030){
				o<<"1030_WARRIOR_MALE.SH";
			}else if(s == 1040){
				o<<"1040_WARRIOR_FEMALE.SH";
			}else if(s == 1050){
				o<<"1050_MAJIN.SH";
			}else if(s == 1060){
				o<<"1060_NINJA.SH";
			}else if(s == 1070){
				o<<"1070_RONIN.SH";
			}else if(s == 1080){
				o<<"1080_KNIGHT.SH";
			}else if(s == 1090){
				o<<"1090_ARCHER.SH";
			}else if(s == 1100){
				o<<"1100_SKULL.SH";
			}else if(s == 1110){
				o<<"1110_MAGE.SH";
			}else if(s == 1120){
				o<<"1120_CLERIC_MALE.SH";
			}else if(s == 1130){
				o<<"1130_CLERIC_FEMALE.SH";
			}else if(s == 1140){
				o<<"1140_SCOUT.SH";
			}else if(s == 1150){
				o<<"1150_RECRUIT.SH";
			}else if(s == 1160){
				o<<"1160_ANGEL.SH";
			}else if(s == 1170){
				o<<"1170_ROGUE.SH";
			}else if(s == 2000){
				o<<"2000_HOBBIT.SH";
			}else if(s == 2010){
				o<<"2010_GHOST.SH";
			}else if(s == 2020){
				o<<"2020_GOLEM.SH";
			}else if(s == 2030){
				o<<"2030_MANEATER.SH";
			}else if(s == 2040){
				o<<"2040_IMP.SH";
			}else if(s == 2050){
				o<<"2050_KILLERARMOR.SH";
			}else if(s == 2060){
				o<<"2060_PUMPKIN.SH";
			}else if(s == 2070){
				o<<"2070_GARGOYLE.SH";
			}else if(s == 2080){
				o<<"2080_NIGHTMARE.SH";
			}else if(s == 2090){
				o<<"2090_SERPENT.SH";
			}else if(s == 2100){
				o<<"2100_CYCLOPS.SH";
			}else if(s == 2110){
				o<<"2110_DRAGON.SH";
			}else if(s == 2120){
				o<<"2120_ZOMBIE.SH";
			}else if(s == 2130){
				o<<"2130_BRUTE.SH";
			}else if(s == 2140){
				o<<"2140_BAAL.SH";
			}else if(s == 2150){
				o<<"2150_MANTICORE.SH";
			}else if(s == 2160){
				o<<"2160_EMPUSA.SH";
			}else if(s == 2170){
				o<<"2170_NEKOMATA.SH";
			}else if(s == 2180){
				o<<"2180_VAMPIRE.SH";
			}else if(s == 2190){
				o<<"2190_EFREET.SH";
			}else if(s == 2200){
				o<<"2200_THURSDAY.SH";
			}else if(s == 2210){
				o<<"2210_PRINNY.SH";
			}else if(s == 2216){
				o<<"2216_PRINNY_KURTIS.SH";
			}else if(s == 2500){
				o<<"2500_VYERS.SH";
			}else if(s == 2570){
				o<<"2570_VULCANUS.SH";
			}else if(s == 2580){
				o<<"2580_LAMINGTON.SH";
			}else if(s == 2590){
				o<<"2590_KURTIS.SH";
			}else if(s == 2600){
				o<<"2600_DONJOAQUIN.SH";
			}else if(s == 2610){
				o<<"2610_PRISMRANGER.SH";
			}else if(s == 2916){
				o<<"2916_PRINNYMAID.SH";
			}else if(s == 2917){
				o<<"2917_DIEMENSIONPHONE.SH";
			}else if(s == 3000){
				o<<"3000_PRIERE.SH";
			}else if(s == 3010){
				o<<"3010_MARJOLY.SH";
			}else if(s == 3020){
				o<<"3020_ADELL.SH";
			}else if(s == 3030){
				o<<"3030_ROZALIN.SH";
			}else if(s == 3040){
				o<<"3040_ZETTA.SH";
			}else if(s == 3190){
				o<<"3190_MOD-KRISAN_ASAGI.SH";
			}else if(s == 3500){
				o<<"3500_PLEINAIR.SH";
			}else{
				o<<"SPRITE_SHEET";
				o.fill('0');
				o.width(4);
				o<<s<<".SH";
			}
			(*it).name = o.str();
			it++;
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

bool SpriteSheetDAT::insert(const PG::UTIL::File& file){
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No file opened.");
		return FAILURE;
	}

	if(!file.exists()){
		PG_ERROR_STREAM("File doesn't exist!");
		return FAILURE;
	}

	PG::UTIL::File fileName = PG::UTIL::File(file.getFile()).toUpper();
	//file is already inside?
	auto it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [fileName](const fileInfo& info){
		return info.getName() == fileName;
	});

	if(it != m_fileInfos.end()){
		(*it).externalFile = file;
	}else{
		m_lastError = "You can only replace sprite sheets!";
		return FAILURE;
	}

	m_changed = true;

	return SUCCESS;
}

bool SpriteSheetDAT::insert(const PG::UTIL::File& file, unsigned short id){
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No file opened.");
		return FAILURE;
	}

	if(!file.exists()){
		PG_ERROR_STREAM("File doesn't exist!");
		return FAILURE;
	}

	PG::UTIL::File fileName = PG::UTIL::File(file.getFile()).toUpper();
	//file is already inside?
	auto it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [fileName](const fileInfo& info){
		return info.getName() == fileName;
	});

	if(it != m_fileInfos.end()){
		(*it).externalFile = file;
	}else{
		fileInfo info;
		info.name = file.getFile();
		info.externalFile = file;
		info.size = file.size();
		m_fileInfos.push_back(info);
		m_chractersIDs.push_back(id);
	}

	m_changed = true;

	return SUCCESS;
}

void SpriteSheetDAT::clear(){
	m_chractersIDs.clear();
	ExtractorBase::clear();
}


SpriteSheetDAT::~SpriteSheetDAT() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
