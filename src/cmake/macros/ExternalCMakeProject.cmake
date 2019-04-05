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
 
FUNCTION(ExternalCMakeProject_Add)
  cmake_parse_arguments(PARSED_ARGS "NOLINK" "NAME;SOURCE_DIR" "CMAKE_ARGS" ${ARGN} )
 
   if(NOT PARSED_ARGS_NAME)
		message(FATAL_ERROR "You must provide a NAME")
    endif(NOT PARSED_ARGS_NAME)
	
	if(NOT PARSED_ARGS_SOURCE_DIR)
		message(FATAL_ERROR "You must provide a SOURCE_DIR, location of CMakeLists.txt")
    endif(NOT PARSED_ARGS_SOURCE_DIR)
	
	include(ExternalProject)

	set(EXTERNAL_${PARSED_ARGS_NAME}_BUILD_DIR ${PARSED_ARGS_SOURCE_DIR}/build/${CMAKE_GENERATOR} CACHE PATH "Path to compiled binarys" )
	set(EXTERNAL_${PARSED_ARGS_NAME}_SOURCE_DIR ${PARSED_ARGS_SOURCE_DIR} CACHE PATH "Path to CMakeLists.txt")
	
	ExternalProject_Add(${PARSED_ARGS_NAME} 
		PREFIX ${PARSED_ARGS_NAME}
		#--Configure step-------------
		SOURCE_DIR ${PARSED_ARGS_SOURCE_DIR} 
		CMAKE_GENERATOR ${CMAKE_GENERATOR}
		CMAKE_ARGS ${PARSED_ARGS_CMAKE_ARGS}
		#--Build step-----------------
		LOG_BUILD 1
		BINARY_DIR ${EXTERNAL_${PARSED_ARGS_NAME}_BUILD_DIR}
		#--Install step---------------
		UPDATE_COMMAND "" # Skip annoying updates for every build
		INSTALL_COMMAND ""
	)
	if(NOT ${PARSED_ARGS_NOLINK})
		include("${CMAKE_MODULE_PATH}/external projects/Link${PARSED_ARGS_NAME}.cmake")
		linkFunc()
	endif()
ENDFUNCTION()

FUNCTION(ExternalCMakeGitProject_Add)
  cmake_parse_arguments(PARSED_ARGS "NOLINK" "NAME;SOURCE_DIR;GIT_URL" "CMAKE_ARGS" ${ARGN} )
 
   if(NOT PARSED_ARGS_NAME)
		message(FATAL_ERROR "You must provide a NAME")
    endif(NOT PARSED_ARGS_NAME)
	
	if(NOT PARSED_ARGS_GIT_URL)
		message(FATAL_ERROR "You must provide a URL to a git repo (GIT_URL)")
    endif(NOT PARSED_ARGS_GIT_URL)
	
	if(NOT PARSED_ARGS_SOURCE_DIR)
		message(FATAL_ERROR "You must provide a SOURCE_DIR, location of CMakeLists.txt")
    endif(NOT PARSED_ARGS_SOURCE_DIR)
	
	include(ExternalProject)

	set(EXTERNAL_${PARSED_ARGS_NAME}_BUILD_DIR ${PARSED_ARGS_SOURCE_DIR}/build/${CMAKE_GENERATOR} CACHE PATH "Path to compiled binarys" )
	set(EXTERNAL_${PARSED_ARGS_NAME}_SOURCE_DIR ${PARSED_ARGS_SOURCE_DIR} CACHE PATH "Path to CMakeLists.txt")
	
	ExternalProject_Add(${PARSED_ARGS_NAME} 
		PREFIX ${PARSED_ARGS_NAME}
		#--Download step-------------
		GIT_REPOSITORY  ${PARSED_ARGS_GIT_URL}
		GIT_SHALLOW 1
		#--Configure step-------------
		SOURCE_DIR ${PARSED_ARGS_SOURCE_DIR} 
		CMAKE_GENERATOR ${CMAKE_GENERATOR}
		CMAKE_ARGS ${PARSED_ARGS_CMAKE_ARGS}
		#--Build step-----------------
		LOG_BUILD 1
		BINARY_DIR ${EXTERNAL_${PARSED_ARGS_NAME}_BUILD_DIR}
		#--Install step---------------
		UPDATE_COMMAND "" # Skip annoying updates for every build
		INSTALL_COMMAND ""
	)
	
	
	ExternalProject_Add_Step(
		${PARSED_ARGS_NAME} ${PARSED_ARGS_NAME}
		COMMAND ${CMAKE_COMMAND} -E make_directory EXTERNAL_${PARSED_ARGS_NAME}_BUILD_DIR
	)
	
	if(NOT ${PARSED_ARGS_NOLINK})
		include("${CMAKE_MODULE_PATH}/external projects/Link${PARSED_ARGS_NAME}.cmake")
		linkFunc()
	endif()
ENDFUNCTION()