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
#include <pg/files/PG_IMY.h>
#include <pg/files/PG_ImageFiles.h>
#include <pg/files/PG_PSPFS.h>
#include <algorithm>
#include <pg/util/PG_Array.h>
#include <fstream>
#include <pg/util/PG_Vector.h>
#include <pg/util/PG_VectorUtil.h>
#include <pg/stream/PG_StreamInByteFile.h>

#include <pg/util/PG_Matrix.h>

#include <cstdlib>
#include <cmath>
#include <iomanip>

#include <pg/files/PG_SpriteSheet.h>
#include <pg/util/PG_Image.h>


#define OUTSTR(x) std::cout << x << std::endl


struct face{
	PG::UTIL::ivec3 A;
	PG::UTIL::ivec3 B;
	PG::UTIL::ivec3 C;
};

struct objectGroup{
	std::string name; //g
	std::string matname; //g
	std::vector<PG::UTIL::vec3> verteces; //v
	std::vector<PG::UTIL::vec3> normals; //vn
	std::vector<PG::UTIL::vec2> uvw; //vt

	std::vector<face> faces;
};

struct obj{
	std::string mtllib;

	std::vector<objectGroup> groups;
};


unsigned int skipBlank(const std::string& str, unsigned int start){
	while(start < str.size()){
		const char c = str[start];
		if(c  != ' ' && c != '\t') break;
		start++;
	}
	return start;
}

unsigned int readUntilBlank(const std::string& str, unsigned int start, std::string& strOut){
	start = skipBlank(str,start);
	strOut.clear();
	while(start < str.size()){
		const char c = str[start];
		if(c  == ' ' || c == '\t') break;
		strOut.push_back(c);
		start++;
	}
	return start;
}

unsigned int readUntilSlash(const std::string& str, unsigned int start, std::string& strOut){
	start = skipBlank(str,start);
	strOut.clear();
	while(start < str.size()){
		const char c = str[start];
		if(c == '/' || c  == ' ' || c == '\t') break;
		strOut.push_back(c);
		start++;
	}
	return start;
}

unsigned int readInt(const std::string& str, std::vector<int>& fOut, unsigned int start = 0){
	start = skipBlank(str,start);
	fOut.clear();


	while(start < str.size()){
		std::string num;
		start = readUntilSlash(str,start, num);
		start++;
		start = skipBlank(str,start);
		fOut.push_back(atoi(num.c_str()));
	}

	return start;
}

unsigned int readFloat(const std::string& str, std::vector<float>& fOut, unsigned int start = 0){
	start = skipBlank(str,start);
	fOut.clear();


	while(start < str.size()){
		std::string num;
		start = readUntilSlash(str,start, num);
		start++;
		start = skipBlank(str,start);
		fOut.push_back(atof(num.c_str()));
	}

	return start;
}

PG::UTIL::vec3 readVec3(const std::string& str, unsigned int start = 0){
	std::vector<float> fOut;
	readFloat(str,fOut,start);
	return PG::UTIL::vec3(fOut);
}

PG::UTIL::vec2 readVec2(const std::string& str, unsigned int start = 0){
	std::vector<float> fOut;
	readFloat(str,fOut,start);
	return PG::UTIL::vec2(fOut);
}

PG::UTIL::ivec3 readiVec3(const std::string& str, unsigned int start = 0){
	std::vector<int> iOut;
	readInt(str,iOut,start);
	return PG::UTIL::ivec3(iOut);
}

face readFace(const std::string& str, unsigned int start = 0){

	std::vector<int> iOut;
	readInt(str,iOut,start);

	face f;
	if(iOut.size() == 9){
		f.A = PG::UTIL::ivec3(iOut[0],iOut[1],iOut[2]);
		f.B = PG::UTIL::ivec3(iOut[3],iOut[4],iOut[5]);
		f.C = PG::UTIL::ivec3(iOut[6],iOut[7],iOut[8]);
	}else{
		OUTSTR("Error: Face size too small : "<<iOut.size());

	}
	return f;
}

inline bool readGroup(std::ifstream& objfile, objectGroup& group,std::string& line){
	while ( getline (objfile,line) ){
		const unsigned int pos = objfile.tellg();
		unsigned int currentPos = 0;
		std::string currentString;
		currentPos = readUntilBlank(line,currentPos, currentString );
		if(currentString == "v"){
			group.verteces.push_back(readVec3(line,currentPos));
		}else if(currentString == "vt"){
			group.uvw.push_back(readVec2(line,currentPos));
		}else if(currentString == "vn"){
			group.normals.push_back(readVec3(line,currentPos));
		}else if(currentString == "f"){
			group.faces.push_back(readFace(line,currentPos));
		}else if(currentString == "usemtl"){
			currentPos = readUntilBlank(line,currentPos, currentString );
			group.matname = currentString;
		}else if(currentString == "g"){
			return true;
		}

	}
	return false;
}

int main(int argc, char* argv[]){
	OUTSTR("Start");


	obj objectFile;

	std::string line;
	//std::ifstream objfile ("D:/Users/ProgSys/Modding/Modelle/film/Campus/Campus2.1_2.obj");
	std::ifstream objfile ("D:/Users/ProgSys/Modding/Modelle/film/Campus/Campus2.1_campus.obj");
	//std::ifstream objfile ("D:/Users/ProgSys/Modding/Modelle/film/Campus/objTest.obj");
	if (objfile.is_open()){
		while ( getline (objfile,line) ){
			//cout << line << '\n';
			unsigned int currentPos = 0;
			std::string currentString;
			currentPos = readUntilBlank(line,currentPos, currentString );
			//OUTSTR("Type: '"<<currentString<<"'");
			if(currentString == "mtllib"){
				currentPos = readUntilBlank(line,currentPos, currentString );
				objectFile.mtllib = currentString;
			}else if(currentString == "g"){
				bool readGroups = true;
				while(readGroups){
					currentPos = readUntilBlank(line,2, currentString );
					objectGroup group;
					group.name = currentString;
					OUTSTR("Group: "<<currentString);
					readGroups = readGroup(objfile,group, line);
					objectFile.groups.push_back(group);
				}
			}
		}
		objfile.close();
	}


	ofstream objwrite;
	objwrite.open ("D:/Users/ProgSys/Modding/Modelle/film/Campus/Campus2.1_campus_PG.obj");

	objwrite << "# PG Convert\n";
	objwrite <<"mtllib "<< objectFile.mtllib<< "\n";
	OUTSTR("Groups to export: "<<objectFile.groups.size());
	unsigned int sumverteces = 0;
	unsigned int sumuvw = 0;
	unsigned int sumnormals = 0;
	for(const objectGroup& group: objectFile.groups){
		//test
		/*
		if(group.verteces.size() != group.normals.size()){
			OUTSTR("Group: "<<group.name<<" Number of verteces and normals is not the same!");
			continue;
		}
		else if(group.verteces.size() != group.uvw.size()){
			OUTSTR("Group: "<<group.name<<" Number of verteces and uvw's is not the same!");
			continue;
		}
		*/

		sumverteces += group.verteces.size();
		sumuvw += group.uvw.size();
		sumnormals += group.normals.size();
		bool test = false;

		for(const face& f: group.faces){
			//v vt vn

			if(f.A.x > sumverteces || f.B.x > sumverteces || f.C.x > sumverteces){
				OUTSTR("Group: "<<group.name<<" Too many verteces! MAX: "<<group.verteces.size()<<" ("<<f.A.x<<", "<<f.B.x<<", "<<f.C.x<<")");
				test = true;
				break;
			}
			if(f.A.y > sumuvw || f.B.y > sumuvw || f.C.y > sumuvw){
				OUTSTR("Group: "<<group.name<<" Too many uvw!");
				test = true;
				break;
			}
			if(f.A.z > sumnormals || f.B.z > sumnormals || f.C.z > sumnormals){
				OUTSTR("Group: "<<group.name<<" Too many normals!");
				test = true;
				break;
			}
		}
		if(test){
			OUTSTR("Group: "<<group.name<<" The face is build wrong!");
			continue;
		}

		//write
		objwrite <<"g "<< group.name<< "\n";

		for(const PG::UTIL::vec3& v: group.verteces){
			objwrite <<"v "<< v.x<<" "<< v.y<<" "<< v.z<< "\n";
		}
		for(const PG::UTIL::vec3& vn: group.normals){
			objwrite <<"vn "<< vn.x<<" "<< vn.y<<" "<< vn.z<< "\n";
		}

		for(const PG::UTIL::vec2& vt: group.uvw){
			objwrite <<"vt "<< vt.x<<" "<< vt.y<< "\n";
		}
		if(!group.matname.empty())
			objwrite <<"usemtl "<< group.matname<< "\n";


		for(const face& f: group.faces){
			objwrite <<"f "
					<< f.A.x<<"/"<<f.A.y<<"/"<<f.A.z<<" "
					<< f.B.x<<"/"<<f.B.y<<"/"<<f.B.z<<" "
					<< f.C.x<<"/"<<f.C.y<<"/"<<f.C.z<<" "
					<< "\n";
		}
	}

	objwrite.close();


	OUTSTR("END");
}
