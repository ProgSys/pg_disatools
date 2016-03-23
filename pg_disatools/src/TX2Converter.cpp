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
#include <iomanip>


#include <pg/files/PG_Convert.h>
#include <pg/util/PG_ByteInFileStream.h>
#include <pg/util/PG_Exception.h>
#include <pg/files/PG_TX2.h>

using namespace PG;

#define STROUT(x) std::cout << x << std::endl
#define STRERR(x) std::cout <<"["<<__LINE__ <<":ERROR] "<< x << std::endl

inline bool changeFileExtrention(std::string& str, PG::FILE::fileFormat format){
	auto it = str.find_last_of(".");
	str = str.substr(0, it+1);

	if(str.empty() || it == 0 || it >= str.size()){
		STRERR("Input file path has wrong syntax!");
		return true;
	}

	switch (format) {
		case PG::FILE::fileFormat::TGA:
			str += "tga";
			break;
		case PG::FILE::fileFormat::PNM:
			str += "pnm";
			break;
		case PG::FILE::fileFormat::VTF:
			str += "vtf";
			break;
		case PG::FILE::fileFormat::TX2:
			str += "tx2";
			break;
		default:
			STRERR("Unknown format ... wait what??");
			return 1;
			break;
	}

	return 0;
}

void printUsage(){
	STROUT("\nAllows you to convert Disgaea PC *.TX2 textures into *.TGA (BGRA8888) or *.PNM (P3/P6 RBG888 No alpha) and back!");
	STROUT("Make a backup of your files before using this tool!");
	STROUT("Version: 0.4 (early access pre alpha thingy dood) \n");

	STROUT("Usage: ");
	STROUT("  * "<< std::left << std::setw(43)<< "'-tx2 <path_to_TX2>':"<< "Decompress a *.TX2 file into a *.TGA image.");
	STROUT("  * "<< std::left << std::setw(43)<< "'-tx2to <path_to_TX2> <TGA | PNM>':"<< "Decompress a *.TX2 file into a *.TGA or *.PNM image.");

	STROUT("  * "<< std::left << std::setw(43)<< "'-img <path_to_IMG> <DXT1 | DXT5 | BGRA>':"<< "Compresses a *.TGA or *.PNM file into to a DXT1, DXT5 or BGRA *.TX2 file.");

	STROUT("  * "<< std::left << std::setw(43)<< "'-folder <path_to_folder> (-dir)':"<< "(Optional) Target output folder.");

	STROUT("  * "<< std::left << std::setw(43)<< "'-output <file_name> (-out)':"<< "(Optional) Target output path and filename. Overwrites the -folder command.");
	STROUT("  * "<< std::left << std::setw(43)<< "'-help (-p)':"<< "Prints the usage.");
	STROUT("  * "<< std::left << std::setw(43)<< "'-license (-l)':"<< "Prints the license.");
	STROUT("    On default the file will be save into the same folder as the input file, just with a different file extension. ");

	STROUT("Examples: ");
	STROUT("  *  '.\\TX2Converter.exe -tx2 'C:\\Users\\ProgSys\\Desktop\\Disgaea\\texture_analysis\\BU3202.TX2'' ");
	STROUT("  *  '.\\TX2Converter.exe -tx2to 'C:\\Users\\ProgSys\\Desktop\\Disgaea\\texture_analysis\\BU3202.TX2' PNM -dir 'C:\\Users\\ProgSys\\Desktop\\Disgaea\\texturesout''");
	STROUT("  *  '.\\TX2Converter.exe -img 'C:\\Users\\ProgSys\\Desktop\\Disgaea\\texture_analysis\\image.tga' DXT5'");
}

void printLicence(){
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

void printInfo(){
	printUsage();
	STROUT("\n");
	printLicence();
}

int main(int argc, char* argv[]){

	if(argc == 1){
		printInfo();
		return 0;
	}



	std::string targetfile = "";
	bool compress = false;
	std::string dir = "";
	std::string out = "";

	PG::FILE::fileFormat format = PG::FILE::fileFormat::TGA;
	PG::FILE::tx2Type comressionType = PG::FILE::tx2Type::DXT1;

	//read arguments
	for(unsigned int i = 1; i < argc; i++){
		if(std::strcmp(argv[i],"-tx2") == 0  && (i+1)<argc){
			targetfile = std::string(argv[i+1]);
			compress = false;
			i += 1;
		}else if(std::strcmp(argv[i],"-tx2to") == 0  && (i+2)<argc){
			targetfile = std::string(argv[i+1]);

			std::string f(argv[i+2]);
			std::transform(f.begin(), f.end(), f.begin(), ::tolower);
			if(f == "tga" || f == ".tga" || f == "*.tga"){
				format = PG::FILE::fileFormat::TGA;
			}else if(f == "pgm" || f == ".pgm" || f == "*.pgm" || f == "pnm" || f == ".pnm" || f == "*.pnm"){
				format = PG::FILE::fileFormat::PNM;
			}else{
				STRERR("Unknown file format given: '"<<f<<"'! Can only be 'tga' or 'pgm'.");
				return 1;
			}
			i += 1;

			compress = false;
			i += 1;
		}else if( (std::strcmp(argv[i],"-img") == 0 || std::strcmp(argv[i],"-tga") == 0 || std::strcmp(argv[i],"-pnm") == 0 )  && (i+2)<argc){
			targetfile = std::string(argv[i+1]);

			std::string f(argv[i+2]);
			std::transform(f.begin(), f.end(), f.begin(), ::tolower);
			if(f == "dxt1"){
				comressionType = PG::FILE::tx2Type::DXT1;
			}else if(f == "dxt5"){
				comressionType = PG::FILE::tx2Type::DXT5;
			}else if(f == "bgra"){
				comressionType = PG::FILE::tx2Type::BGRA;
			}else{
				STRERR("Unknown compression type '"<<f<<"'! Only 'DXT1', 'DXT5' and 'BGRA' are supported!");
				return 1;
			}

			compress = true;
			i += 2;
		}else if( (std::strcmp(argv[i],"-c") == 0 || std::strcmp(argv[i],"-compression") == 0) && (i+1)<argc){
			std::string f(argv[i+1]);
			std::transform(f.begin(), f.end(), f.begin(), ::tolower);
			if(f == "dxt1"){
				comressionType = PG::FILE::tx2Type::DXT1;
			}else if(f == "dxt5"){
				comressionType = PG::FILE::tx2Type::DXT5;
			}else if(f == "bgra"){
				comressionType = PG::FILE::tx2Type::BGRA;
			}else{
				STRERR("Unknown compression type '"<<f<<"'! Only 'DXT1', 'DXT5' and 'BGRA' are supported!");
				return 1;
			}
			i += 1;
		}else if( (std::strcmp(argv[i],"-dir") == 0 || std::strcmp(argv[i],"-folder") == 0) && (i+1)<argc ){
			dir = std::string(argv[i+1]);
			i += 1;
		}else if( (std::strcmp(argv[i],"-out") == 0 || std::strcmp(argv[i],"-output") == 0) && (i+1)<argc){
			out = std::string(argv[i+1]);
			i += 1;
		}else if( (std::strcmp(argv[i],"-help") == 0 || std::strcmp(argv[i],"-h" ) == 0 || std::strcmp(argv[i],"help" ) == 0)){
			printUsage();
			return 0;
		}else if( (std::strcmp(argv[i],"-license") == 0 || std::strcmp(argv[i],"-l") == 0)){
			printLicence();
			return 0;
		}else{
			STRERR("Unknown command '"<<argv[i]<<"'!");
			return 1;
		}
	}

	if(targetfile.empty()){
		STRERR("Input file path is empty!");
		return 1;
	}



	if(compress){
		//compress TGA888 image to TX2
		if(out.empty()){
			if(dir.empty()){
				out = targetfile;
				if(changeFileExtrention(out, PG::FILE::fileFormat::TX2) ) return 1;
			}else{
				if(dir[dir.size()-1] == '/' || dir[dir.size()-1] == '\\')
					dir.erase(dir.size()-1);
				auto it = targetfile.find_last_of("\\/");
				if(it == 0 || it >= targetfile.size()){
					STRERR("Input file path has wrong syntax!");
					return 1;
				}

				out = dir+"/"+ targetfile.substr(it+1);
				if(changeFileExtrention(out, PG::FILE::fileFormat::TX2) ) return 1;
			}

		}

		PG::UTIL::File file(out);
		if(file.exists())
			STROUT("[WARNING] Output file already exists! File will be overwritten!");

		STROUT("Converting to TX2. Please wait...");
		if(PG::FILE::convertImageToTX2(targetfile, out, comressionType)){
			STRERR("Couldn't save image! ");
			return 1;
		}

	}else{

		//decompress TX2
		if(out.empty()){
			if(dir.empty()){
				out = targetfile;
				if(changeFileExtrention(out, format) ) return 1;
			}else{

				if(dir[dir.size()-1] == '/' || dir[dir.size()-1] == '\\')
					dir.erase(dir.size()-1);

				auto it = targetfile.find_last_of("\\/");
				if(it == 0 || it >= targetfile.size()){
					STRERR("Input file path has wrong syntax!");
					return 1;
				}

				out = dir+"/"+ targetfile.substr(it+1);
				if(changeFileExtrention(out, format) ) return 1;
			}
		}

		PG::UTIL::File file(out);
		if(file.exists())
			STROUT("[WARNING] Output file already exists! File will be overwritten!");

		try {
			if(PG::FILE::convertTX2ToImage(targetfile, out, format)){
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
	}
	STROUT("Saved to: '"<<out<<"'");

}


