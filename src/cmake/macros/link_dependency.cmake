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

MACRO(link name)
	include(${CMAKE_MODULE_PATH}/external/Link${name}.cmake)
ENDMACRO()

MACRO(link_dependency name)
	if("${ARGN}" STREQUAL "")
		set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{${name}_ROOT})
	else()
		set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{${ARGN}})
	endif()
        find_package(${name} REQUIRED)
	include(${CMAKE_MODULE_PATH}/external/Link${name}.cmake)
ENDMACRO()

MACRO(link_dependency_QUIET name)
	if("${ARGN}" STREQUAL "")
		set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{${name}_ROOT})
	else()
		set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{${ARGN}})
	endif()
        find_package(${name} QUIET)
    if(${name}_FOUND)
		include(${CMAKE_MODULE_PATH}/external/Link${name}.cmake)	
	endif()	
ENDMACRO()

MACRO(target_extearnal_link name target)
	include("${CMAKE_MODULE_PATH}/external projects/Link${name}.cmake")
	linkFunc(TARGET ${target})
ENDMACRO()

