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


cmake_minimum_required(VERSION 3.1)

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})

add_definitions(-D__FILENAME__="NoFileName")#\\"$\(<F\)\\"

message( "PROJECT_LIBRARIES_PATH: " ${PROJECT_LIBRARIES_PATH} )

include_directories(
    ${PROJECT_LIBRARIES_PATH}
)

file(GLOB_RECURSE SRC *.cpp *.h)

add_executable(${ProjectId} ${SRC} ${EXTERNAL_CODE}) 
target_compile_features(${ProjectId} PUBLIC cxx_std_17)
project_source_group(SRC)
#target_link_libraries(${ProjectId} SDL2)

set_target_properties(${ProjectId} PROPERTIES FOLDER "executable/")

target_link_libraries(
    ${ProjectId}
    ${ALL_LIBRARIES}
    #${NFD_DIR}/src/libnfd_d.a
    #${GTK3_LIBRARIES}
)
