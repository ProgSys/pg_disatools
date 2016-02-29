/*
 * Allows you to convert Disgaea PC *.TX2 textures to *.TGA (BGRA8888) or *.PGM (P6 RBG888 No alpha).
 *
 *  Copyright (C) 2016  ProgSys
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>
#include <algorithm>


#include <pg/files/PG_S3File.h>
#include <pg/util/PG_BinaryFileTokenizer.h>
#include <pg/util/PG_Exception.h>

using namespace PG;

#define STROUT(x) std::cout << x << std::endl
#define STRERR(x) std::cout <<"["<<__LINE__ <<":ERROR]"<< x << std::endl

inline bool changeFileExtrention(std::string& str, PG::FILE::S3File::outFormat format){
	auto it = str.find_last_of(".");
	str = str.substr(0, it+1);

	if(str.empty() || it == 0 || it >= str.size()){
		STRERR("Input file path has wrong syntax!");
		return true;
	}

	switch (format) {
		case PG::FILE::S3File::TGA:
			str += "tga";
			break;
		case PG::FILE::S3File::PGM:
			str += "pgm";
			break;
		default:
			STRERR("Unknown format ... wait what??");
			return 1;
			break;
	}

	return 0;
}

void printInfo(){
	STROUT("\nAllows you to convert Disgaea PC *.TX2 textures to *.TGA (BGRA8888) or *.PGM (P6 RBG888 No alpha).");
	STROUT("Sadly the other way around is not supported yet.");
	STROUT("Make a backup before using this tool!");
	STROUT("Version: 0.0.00000000000000000000001 (early access pre alpha thingy dood) \n");

	STROUT("Usage: ");
	STROUT("  * '-in': Target path to the *.TX2 file.");
	STROUT("  * '-format (-f)':   (Optional) Output file format can be TGA or PGM. Default TGA.");

	STROUT("  * '-folder (-dir)': (Optional) Target output folder.");
	STROUT("                      On default the file will be save into the same folder as the input file, just with a different file extension. ");

	STROUT("  * '-output (-out)': (Optional) Target output path and filename. Overwrites the -folder command.");
	STROUT("                      On default the file will be save into the same folder as the input file, just with a different file extension. ");

	STROUT("Examples: ");
	STROUT("  *  '.\\TX2andMPPConverter.exe -in 'C:\\Users\\ProgSys\\Desktop\\Disgaea\\texture_analysis\\BU3202.TX2' ");
	STROUT("  *  '.\\TX2andMPPConverter.exe -in 'C:\\Users\\ProgSys\\Desktop\\Disgaea\\texture_analysis\\BU3202.TX2' -dir 'C:\\Users\\ProgSys\\Desktop\\Disgaea\\texturesout' -f pgm");


	STROUT("\n");

	STROUT("License: ");
	STROUT("\n\nGNU General Public License (GPL): \n");

	STROUT("Copyright (C) 2016  ProgSys\n\n"
	    " This program comes with ABSOLUTELY NO WARRANTY; for details type 'show1'.\n"
	    " This is free software, and you are welcome to redistribute it\n"
	    " under certain conditions; type 'show2' for details.\n");
}

int main(int argc, char* argv[]){

	if(argc == 1){
		printInfo();
		return 0;
	}

	std::string target = "";
	std::string dir = "";
	std::string out = "";
	PG::FILE::S3File::outFormat format = PG::FILE::S3File::TGA;

	//read arguments
	for(unsigned int i = 1; i < argc; i++){
		if(std::strcmp(argv[i],"-in") == 0  && (i+1)<argc){
			target = std::string(argv[i+1]);
			i += 1;
		}else if( (std::strcmp(argv[i],"-f") == 0 || std::strcmp(argv[i],"-format") == 0) && (i+1)<argc){
			std::string f(argv[i+1]);
			std::transform(f.begin(), f.end(), f.begin(), ::tolower);
			if(f == "tga" || f == ".tga" || f == "*.tga"){
				format = PG::FILE::S3File::TGA;
			}else if(f == "pgm" || f == ".pgm" || f == "*.pgm"){
				format = PG::FILE::S3File::PGM;
			}else{
				STRERR("Unknown file format given: '"<<f<<"'! Can only be 'tga' or 'pgm'.");
				return 1;
			}
			i += 1;
		}else if( (std::strcmp(argv[i],"-dir") == 0 || std::strcmp(argv[i],"-folder") == 0) && (i+1)<argc ){
			dir = std::string(argv[i+1]);
			i += 1;
		}else if( (std::strcmp(argv[i],"-out") == 0 || std::strcmp(argv[i],"-output") == 0) && (i+1)<argc){
			out = std::string(argv[i+1]);
			i += 1;
		}else if( (std::strcmp(argv[i],"show1")) == 0 ){
			STROUT("This program is distributed in the hope that it will be useful,\n"
					"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
					"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
					"GNU General Public License for more details.");
			return 0;
		}else if( (std::strcmp(argv[i],"show2")) == 0 ){
			STROUT("This program is free software: you can redistribute it and/or modify\n"
					"it under the terms of the GNU General Public License as published by\n"
					"the Free Software Foundation, either version 3 of the License, or\n"
					"(at your option) any later version.");
			return 0;
		}


	}

	if(target.empty()){
		STRERR("Input file path is empty!");
		return 1;
	}

	if(out.empty()){
		if(dir.empty()){
			out = target;
			if(changeFileExtrention(out, format) ) return 1;
		}else{

			if(dir[dir.size()-1] == '/' || dir[dir.size()-1] == '\\')
				dir.erase(dir.size()-1);

			auto it = target.find_last_of("\\/");
			if(it == 0 || it >= target.size()){
				STRERR("Input file path has wrong syntax!");
				return 1;
			}

			out = dir+"/"+ target.substr(it+1);
			if(changeFileExtrention(out, format) ) return 1;
		}
	}

	try {
		PG::FILE::S3File file(target);
		file.save(out, format);
	} catch (PG::UTIL::Exception& e) {
		STRERR("Couldn't save texture: "<<e.what());
		return 1;
	}catch (...) {
		STRERR("Couldn't save texture! ");
		return 1;
	}


	STROUT("Saved to: '"<<out<<"'");

}


