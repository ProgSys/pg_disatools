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

# A cmake tempalte for a c++ project
string(REPLACE "/" ";" p2list "${CMAKE_SOURCE_DIR}")
string(REPLACE "\\" ";" p2list "${p2list}")
list(REVERSE p2list)
list(GET p2list 0 first)
list(GET p2list 1 ProjectId)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})

include(${CMAKE_MODULE_PATH}/doxygen.cmake)
include(${CMAKE_MODULE_PATH}/macros.cmake)

set(CMAKE_CONFIGURATION_TYPES Debug;Release)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++0x") # Use c++11


# == OpenGL==
link_dependency(OpenGL3)

# The OpenGL Extension Wrangler Library
add_subdirectory("${PROJECT_EXTERNALS_PATH}/glew-cmake")
link_dependency(GLEW) 

# Simple DirectMedia Layer
add_subdirectory("${PROJECT_EXTERNALS_PATH}/SDL-mirror")
link_dependency(SDL) 

# PNG load
set(LODEPNG  "${PROJECT_EXTERNALS_PATH}/lodepng/lodepng.h" "${PROJECT_EXTERNALS_PATH}/lodepng/lodepng.cpp")

#GLM
# Add GLM
file(GLOB_RECURSE GLM
    "${PROJECT_EXTERNALS_PATH}/GLM/glm/*.hpp")
include_directories("${PROJECT_EXTERNALS_PATH}/GLM")
#link_dependency(GLM)

add_definitions(
        -DGLM_FORCE_RADIANS
        -DGLM_FORCE_PURE
        -DNOMINMAX
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if("${CMAKE_SYSTEM}" MATCHES "Linux")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
	include_directories(${GTK3_INCLUDE_DIRS})
	link_directories(${GTK3_LIBRARY_DIRS})
	# Add other flags to the compiler
	add_definitions(${GTK3_CFLAGS_OTHER})
endif()


# Collect all code
set(EXTERNAL_CODE
        ${LODEPNG}
        ${GLM}
        ${GLEW}
)

# Include externals folder
include_directories(${PROJECT_EXTERNALS_PATH})


include_directories(
    ${CORE_PATH}
)

if("${CMAKE_SYSTEM}" MATCHES "Linux")
	find_package(X11)
        set(ALL_LIBRARIES ${ALL_LIBRARIES} -lGL -lGLU ${GLFW3_LIBRARY} ${X11_LIBRARIES} Xcursor Xinerama Xrandr Xxf86vm Xi pthread -ldl assimp ${DevIL} ${LIBIGL_LIBRARIES} ${LIBIGL_EXTRA_LIBRARIES})
endif()

add_definitions(-DPROJECT_SHADERS_PATH="${PROJECT_SHADERS_PATH}")
add_definitions(-DPROJECT_RESOURCES_PATH="${PROJECT_RESOURCES_PATH}")

#if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  add_definitions(-Wall -Wextra)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
  # using Intel C++
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  add_definitions(/W2)
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
GENERATE_SUBDIRS(ALL_LIBRARIES ${PROJECT_LIBRARIES_PATH} ${PROJECT_BINARY_DIR}/libraries)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
GENERATE_SUBDIRS(ALL_EXECUTABLES ${PROJECT_EXECUTABLES_PATH} ${PROJECT_BINARY_DIR}/executables)


file (COPY "${CMAKE_MODULE_PATH}/gdb_prettyprinter.py" DESTINATION ${PROJECT_BINARY_DIR})