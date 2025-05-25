#pragma once

#include <vector>
#include <string>
#include <Stream/PG_StreamIn.h>
#include <Stream/PG_StreamOut.h>
#include <Util/PG_Exception.h>

namespace PG_DisaTalkScript {
	std::vector<unsigned char> compile(const std::string& code) throw(PG_CompileException);
	std::vector<unsigned char> compile(PG::STREAM::In* codeStream) throw(PG_CompileException);


	std::string decompile(const std::vector<unsigned char>& code) throw(PG_CompileException);
	std::string decompile(PG::STREAM::In* codeStream) throw(PG_CompileException);
};

