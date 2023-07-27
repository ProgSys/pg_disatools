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
 

# A cmake tempalte for a c++ project using QT and glew(OpenGl).
cmake_minimum_required(VERSION 3.1)
string(REPLACE "/" ";" p2list "${CMAKE_SOURCE_DIR}")
string(REPLACE "\\" ";" p2list "${p2list}")
list(REVERSE p2list)
list(GET p2list 0 first)
list(GET p2list 1 ProjectId)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId} VERSION ${PROJECT_GLOBAL_NUMBER})


# ===================================
# ======== Project settings =========
# ===================================


set(PROJECT_EXTERNALS_PATH ${CMAKE_SOURCE_DIR}/externals CACHE PATH "Project specific path. Set manually if it was not found.")
set(PROJECT_RESOURCES_PATH ${CMAKE_SOURCE_DIR}/../resources CACHE PATH "Project specific path. Set manually if it was not found.")
set(PROJECT_EXECUTABLES_PATH ${CMAKE_SOURCE_DIR}/executables CACHE PATH "Project specific path. Set manually if it was not found.")
set(PROJECT_LIBRARIES_PATH ${CMAKE_SOURCE_DIR}/libraries CACHE PATH "Project specific path. Set manually if it was not found.")
set(PROJECT_TESTS_PATH ${CMAKE_SOURCE_DIR}/tests CACHE PATH "Project specific path. Set manually if it was not found.")
set(PROJECT_SHADERS_PATH ${PROJECT_RESOURCES_PATH}/shaders CACHE PATH "Project specific path. Set manually if it was not found.")

set(PROJECT_ICONS_PATH ${CMAKE_SOURCE_DIR}/icons CACHE PATH "Project specific path. Set manually if it was not found.")
set(PROJECT_UI_PATH ${CMAKE_SOURCE_DIR}/ui CACHE PATH "Project specific path. Set manually if it was not found.") 

include(${CMAKE_MODULE_PATH}/doxygen.cmake)
include(${CMAKE_MODULE_PATH}/macros/macros.cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


set(CMAKE_CONFIGURATION_TYPES Debug;Release)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++17") # Use c++17
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ===================================
# ========== Dependencys ============
# ===================================

# == OpenGL==
link_dependency(OpenGL3)

# == The OpenGL Extension Wrangler Library (https://github.com/Perlmint/glew-cmake.git) ==
#link_dependency(GLEW) 
link_dependency_QUIET(GLEW) 


# pre build
  add_subdirectory("${PROJECT_EXTERNALS_PATH}/glew")
if(NOT GLEW_FOUND)
  message(STATUS " ")
  message(STATUS "===========================================================================================================")
  message(STATUS "Project created only for GLEW. Please build GLEW for release and debug, then rerun CMake to build the rest!")
  message(STATUS "===========================================================================================================")
  message(STATUS " ") 
  return()
endif()
if (UNIX)
   set(ALL_LIBRARIES ${ALL_LIBRARIES} -lGL)
endif (UNIX)

# == QT gui ==
#don't forget to install it windows or linux
set(QTCP_PROJECT_DIR ${PROJECT_SOURCE_DIR})

set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# find QT5 cmake files on windows
if (WIN32)

if(MINGW)
find_path(QT_CMAKE_DIR Qt5/Qt5Config.cmake
  PATHS "F:/Qt" "D:/Qt" "C:/Qt" "E:/Qt" 
  PATH_SUFFIXES "5.15.1/mingw73_64/lib/cmake" 
  DOC "The path to the QT cmake files."
)
elseif(MSVC)
find_path(QT_CMAKE_DIR Qt5/Qt5Config.cmake
  PATHS "F:/Qt" "D:/Qt" "C:/Qt" "E:/Qt" 
  PATH_SUFFIXES "5.15.1/msvc2019_64/lib/cmake" 
  DOC "The path to the QT cmake files."
)
endif()
set (CMAKE_PREFIX_PATH ${QT_CMAKE_DIR}) 
endif(WIN32)

#find_package(Qt5Core REQUIRED )
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)

find_package(Qt5Quick REQUIRED)
find_package(Qt5QuickWidgets REQUIRED)

message( "Qt5Widgets_INCLUDES: " ${Qt5Widgets_INCLUDES} ${QT_QTOPENGL_INCLUDE_DIR})

include_directories(
	${Qt5Widgets_INCLUDES}
	${QT_QTOPENGL_INCLUDE_DIR} 
)

add_definitions(${Qt5Widgets_DEFINITIONS})

set(ALL_LIBRARIES ${ALL_LIBRARIES}) 
set(QT_LIBRARIES Qt5::Widgets Qt5::OpenGL Qt5::Qml Qt5::Quick Qt5::QuickWidgets) 

# Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
	option(PROJECT_FULLY_OPTIMIZE "Should the binary be fully optimized? Warning compiling will take considerably longer!")
	if(PROJECT_FULLY_OPTIMIZE)
		set(CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS} "-O3")        ## Optimize
		string (REPLACE ";" " " CMAKE_CXX_FLAGS_STR "${CMAKE_CXX_FLAGS}")
		set(CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS_STR})
		set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary
	else()
		#-DCMAKE_C_FLAGS="-g2" -DCMAKE_CXX_FLAGS="-g2"
		# set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS} "-g2")
		set(CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS} "-g2")
		string (REPLACE ";" " " CMAKE_CXX_FLAGS_STR "${CMAKE_CXX_FLAGS}")
		set(CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS_STR})
	endif()
endif()


#GLM math libaray (https://github.com/g-truc/glm)
link_dependency(GLM) 


#add_definitions(
#        -DGLM_FORCE_RADIANS
#        -DGLM_FORCE_PURE
 #       -DNOMINMAX
#		-DGLM_ENABLE_EXPERIMENTAL
#)


# Collect all code
set(EXTERNAL_CODE
        ${GLEW}
        ${MOCSrcs}
        ${UI_SRCS}
)
message( "PROJECT_EXTERNALS_PATH: " ${PROJECT_EXTERNALS_PATH})
# Include externals folder
include_directories(${PROJECT_EXTERNALS_PATH})

message( "CORE_PATH: " ${CORE_PATH})
include_directories(
    ${CORE_PATH}
)

add_definitions(-DPROJECT_SHADERS_PATH="${PROJECT_SHADERS_PATH}")
add_definitions(-DPROJECT_RESOURCES_PATH="${PROJECT_RESOURCES_PATH}")
add_definitions(-DDEFAULT_RESOURCES_PATH="${PROJECT_DEFAULT_RESOURCES_PATH}" -DPROJECT_VERSION="${PROJECT_VERSION}")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_definitions(-Wall -Wextra)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/W2)
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
GENERATE_SUBDIRS(ALL_LIBRARIES ${PROJECT_LIBRARIES_PATH} ${PROJECT_BINARY_DIR}/libraries)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
GENERATE_SUBDIRS(ALL_EXECUTABLES ${PROJECT_EXECUTABLES_PATH} ${PROJECT_BINARY_DIR}/executables)

set(TESTS_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
GENERATE_SUBDIRS(ALL_TESTS ${PROJECT_TESTS_PATH} ${PROJECT_BINARY_DIR}/tests)

message( "QT_CMAKE_DIR: " ${QT_CMAKE_DIR} )
