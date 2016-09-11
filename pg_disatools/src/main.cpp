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

#include <pg/util/PG_Image.h>
#include <pg/files/PG_MPD.h>


#define OUTSTR(x) std::cout << x << std::endl



int main(int argc, char* argv[]){
	OUTSTR("Start");

	PG::UTIL::vec4 v(1,2,3);
	v = 0.4f * v;
	v =  v * 1.4f;
	OUTSTR(v);
	OUTSTR("END");
	/*

	PG::FILE::MPD mpd;

	mpd.open("D:/Users/ProgSys/ownCloudHome/Public/Work/Modding/Disgaea/Maps/MP00202.MPD");
	mpd.dump("mpdDump.txt");


	*/
}
