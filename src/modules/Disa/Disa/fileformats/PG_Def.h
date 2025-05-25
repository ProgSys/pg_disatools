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

#include <string>
#include <vector>
#include <Util/Color/RGBA.h>
#include <Stream/PG_StreamIn.h>

namespace PG_DefDefinition {

	struct RowFormat {
		enum Type {
			INVALID, /// Error in parsing
			ZERO, /// data that is allways zero, used for passing
			INT, /// int data
			UINT, /// unsigned int data
			SHIFT_JIS, /// A shift jis string, can be fixed or dynamic size
			UNICODE, /// A unicode string, can be fixed or dynamic size
			HEX /// A plain hex string, can be fixed or dynamic size
		};

		Type type = INVALID;
		unsigned short byteSize = 0;
		unsigned short dynamicLength = 0; //defines the byte size of the counter for strings. If zero then the string is fixed.

		static RowFormat create(const std::string& name, const std::vector<std::string>& args);
	};

	struct HeaderFormat {
		enum Type { KEEP, ROW_SIZE };

		RowFormat format;
		Type headerType = KEEP;
	};

	struct ColumnFormat {
		enum ColumnFLAGS { NONE, NOEDIT };
		enum ColumnType { DEFAULT, INDEX };

		std::string name = "Noname";
		ColumnFLAGS flag = NONE;
		ColumnType type = DEFAULT;
		int formatIndex = -1;

		bool hasColor = false;
		glm::rgb colorMinBackground = glm::rgb(255, 255, 255);
		glm::rgb colorMaxBackground = glm::rgb(255, 255, 255);
		int colorMin = -1;
		int colorMax = -1;

		bool operator==(const std::string& str) const {
			return name == str;
		}
	};


}

/**
* @brief Parses definition files for TableDat files
*/
struct PG_Def {
	std::vector<PG_DefDefinition::ColumnFormat> header;
	std::vector<PG_DefDefinition::HeaderFormat> headerFormats;
	std::vector<PG_DefDefinition::RowFormat> formats;

	void clear() {
		header.clear();
		headerFormats.clear();
		formats.clear();
	}

	static PG_Def parse(PG::STREAM::In* reader);
};
