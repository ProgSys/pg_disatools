#[[
 * The MIT License (MIT)
 *
 *	Copyright (c) 2019 ProgSys
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
 

cmake_minimum_required(VERSION 4.0)

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId} VERSION ${PROJECT_GLOBAL_NUMBER})

if(NOT DEFINED RESOURCE_FILES OR UNIX)
	SET(RESOURCE_FILES "")
endif()

if(NOT DEFINED EXECUTABLE_TITLE)
	SET(EXECUTABLE_TITLE ${ProjectId})
endif()

message( "PROJECT_LIBRARIES_PATH: " ${PROJECT_LIBRARIES_PATH} )

include_directories(
    ${PROJECT_LIBRARIES_PATH}
)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADER *.h)


# find and build the given UI files
set(UI_SRCS "") #all srcs files
file(GLOB children_files RELATIVE ${PROJECT_UI_PATH} ${PROJECT_UI_PATH}/*)
FOREACH(ui_file ${UI_FILES})
	if(NOT IS_DIRECTORY ${PROJECT_UI_PATH}/${ui_file})
		if(${ui_file} MATCHES "..*[.]ui")
			string(REPLACE ".ui" "" ui_file_name ${ui_file})
			QT5_WRAP_UI("UI_${ui_file_name}_Src" ${PROJECT_UI_PATH}/${ui_file})
			set(UI_SRCS ${UI_SRCS} "${UI_${ui_file_name}_Src}") 
		endif()
	endif()
ENDFOREACH()

if(WIN32)
	option(PROJECT_START_FROM_WINMAIN "If on the console will be hidden")
	if(PROJECT_START_FROM_WINMAIN)
		add_executable(${ProjectId} WIN32 ${SOURCES} ${HEADER} ${EXTERNAL_CODE} ${UI_SRCS} )
	else()
		add_executable(${ProjectId} ${SOURCES} ${HEADER} ${EXTERNAL_CODE} ${UI_SRCS} )
	endif()
else()
	add_executable(${ProjectId} ${SOURCES} ${HEADER} ${EXTERNAL_CODE} ${UI_SRCS} )
endif()

project_source_group(SRC)

target_link_libraries(
    ${ProjectId}
    ${ALL_LIBRARIES}
	${QT_LIBRARIES}
    ${GTK3_LIBRARIES}
    ${RESOURCE_FILES}
)
set_target_properties(${ProjectId} PROPERTIES FOLDER "executable/")
set_target_properties(${ProjectId} PROPERTIES OUTPUT_NAME ${EXECUTABLE_TITLE})
