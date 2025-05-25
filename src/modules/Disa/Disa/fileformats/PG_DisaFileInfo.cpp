/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2020 ProgSys
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

#include "PG_DisaFileInfo.h"


PG_DisaFileInfo::PG_DisaFileInfo(const PG_File& externalFile) : m_name(externalFile), m_size(externalFile.size()) {
    determinTypeFromFileExtention();
}

void PG_DisaFileInfo::setType(const std::string& extention) {
	if (extention == "mpd") setType(MPD);
	else if (extention == "mpp") setType(MPP);
	else if (extention == "json") setType(JSON);
	else if (extention == "png") setType(PNG);
	else if (extention == "tx2") setType(TX2);
	else if (extention == "geo") setType(GEO);
	else if (extention == "tscript") setType(TALK_SCRIPT);
	else if (extention == "script") setType(SCRIPT);
}