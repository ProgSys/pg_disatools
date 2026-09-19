/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2024 ProgSys
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
#pragma once
#include <Disa/fileformats/PG_DisaFileContainer.h>

struct PG_DisaMPDPack {
	PG_ListModelLocation mpd; //map file
	PG_ListModelLocation mppSD; //textures
	PG_ListModelLocation mppHD; //textures

	PG_DisaFileEntry getMPD() const;
	PG_DisaFileEntry getMPPSD() const;
	PG_DisaFileEntry getMPPHD() const;
};

struct PG_DisaMPPStyle {
	int topIndex = 1;
	int sideIndex = 2;
	int decalIndex = 0;
};

struct PG_DisaMPPPack {
	PG_ListModelLocation mpp; //textures

	PG_DisaFileEntry getMPP() const;
	PG_DisaMPPStyle getStyle() const;
};

/// A geometry carries its own textures, so there is nothing to pair it with
struct PG_DisaGEOPack {
	PG_ListModelLocation geo; //model and its textures

	PG_DisaFileEntry getGEO() const;
};