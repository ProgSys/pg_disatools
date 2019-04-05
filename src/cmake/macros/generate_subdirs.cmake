#[[
 * The MIT License (MIT)
 *
 *	Copyright (c) 2018 ProgSys
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this file, CMake project files and associated documentation files (the "Software"), to deal
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
 ]]
 
 MACRO(GENERATE_SUBDIRS result curdir bindir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(names "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
		IF (NOT ${child} MATCHES "\\..*")
			if(EXISTS ${curdir}/${child}/CMakeLists.txt)
				string(REPLACE " " "_" child ${child})
				SET(names ${names} ${child})
				message("BUILD FOR '${child}' GENERATED")
			endif()
		ENDIF()
    ENDIF()
  ENDFOREACH()
  SET(${result} ${${result}} ${names})
  	FOREACH(n ${names})
  		add_subdirectory(${curdir}/${n} ${bindir}/${n})
  	ENDFOREACH()
ENDMACRO()
