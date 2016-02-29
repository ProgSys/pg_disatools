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

#include <iostream>
#include <string>
#include <algorithm>


#include <pg/files/PG_Convert.h>
#include <pg/util/PG_ByteInFileStream.h>
#include <pg/util/PG_Exception.h>

using namespace PG;

#define STROUT(x) std::cout << x << std::endl
#define STRERR(x) std::cout <<"["<<__LINE__ <<":ERROR]"<< x << std::endl

inline bool changeFileExtrention(std::string& str, PG::FILE::outFileFormat format){
	auto it = str.find_last_of(".");
	str = str.substr(0, it+1);

	if(str.empty() || it == 0 || it >= str.size()){
		STRERR("Input file path has wrong syntax!");
		return true;
	}

	switch (format) {
		case PG::FILE::outFileFormat::TGA:
			str += "tga";
			break;
		case PG::FILE::outFileFormat::PGM:
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

	STROUT(" The MIT License (MIT)\n\n"
			" Copyright (c) 2016 ProgSys\n\n Permission is hereby granted, free of charge, to any person obtaining a copy\n"
			" of this software and associated documentation files (the \"Software\"), to deal\n"
			" in the Software without restriction, including without limitation the rights\n"
			" to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
			" copies of the Software, and to permit persons to whom the Software is\n"
			" furnished to do so, subject to the following conditions:\n\n"

			" The above copyright notice and this permission notice shall be included in all\n"
			" copies or substantial portions of the Software.\n\n"
			" THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
			" IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
			" FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
			" AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
			" LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
			" OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
			" SOFTWARE."
			);
}

int main(int argc, char* argv[]){

	if(argc == 1){
		printInfo();
		return 0;
	}

	std::string target = "";
	std::string dir = "";
	std::string out = "";
	PG::FILE::outFileFormat format = PG::FILE::outFileFormat::TGA;

	//read arguments
	for(unsigned int i = 1; i < argc; i++){
		if(std::strcmp(argv[i],"-in") == 0  && (i+1)<argc){
			target = std::string(argv[i+1]);
			i += 1;
		}else if( (std::strcmp(argv[i],"-f") == 0 || std::strcmp(argv[i],"-format") == 0) && (i+1)<argc){
			std::string f(argv[i+1]);
			std::transform(f.begin(), f.end(), f.begin(), ::tolower);
			if(f == "tga" || f == ".tga" || f == "*.tga"){
				format = PG::FILE::outFileFormat::TGA;
			}else if(f == "pgm" || f == ".pgm" || f == "*.pgm"){
				format = PG::FILE::outFileFormat::PGM;
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
		if(PG::FILE::convertDSATexture(target, out, format)){
			STRERR("Couldn't save texture! ");
			return 1;
		}
	} catch (PG::UTIL::Exception& e) {
		STRERR("Couldn't save texture: "<<e.what());
		return 1;
	}catch (...) {
		STRERR("Couldn't save texture! ");
		return 1;
	}


	STROUT("Saved to: '"<<out<<"'");

}


