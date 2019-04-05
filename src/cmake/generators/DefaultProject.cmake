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

# ===================================
# ============= Project =============
# ===================================
string(REPLACE "/" ";" p2list "${CMAKE_SOURCE_DIR}")
string(REPLACE "\\" ";" p2list "${p2list}")
list(REVERSE p2list)
list(GET p2list 0 first)
list(GET p2list 1 ProjectId)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})


# ===================================
# ======== Project settings =========
# ===================================

include(${CMAKE_MODULE_PATH}/doxygen.cmake)
include(${CMAKE_MODULE_PATH}/macros.cmake)

set(CMAKE_CONFIGURATION_TYPES Debug;Release)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

if(MSVC)
  target_compile_options(${TARGET_NAME} PRIVATE -std=c++0x /W4 /WX)
else(MSVC)
  target_compile_options(${TARGET_NAME} PRIVATE -std=c++0x -Wall -Wextra -pedantic -Werror)
endif(MSVC)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ===================================
# ======== Project settings =========
# ===================================

link_dependency(OpenGL3)

# libigl
add_subdirectory("${PROJECT_EXTERNALS_PATH}/libigl/shared/cmake")
INCLUDE_DIRECTORIES("${LIBIGL_INCLUDE_DIRS}")
add_definitions("${LIBIGL_DEFINITIONS}")


#GLEW
#link_dependency(GLEW)
set(BUILD_UTILS false)
#add_subdirectory("${PROJECT_EXTERNALS_PATH}/glew-cmake")
file(GLOB_RECURSE GLEW
    "${PROJECT_EXTERNALS_PATH}/glew-cmake/src/glew.c")
include_directories("${PROJECT_EXTERNALS_PATH}/glew-cmake/include")


# GLFW
#link_dependency(GLFW3)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "GLFW build examples." )
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "GLFW build tests.")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "GLFW build docs.")
set(GLFW_INSTALL OFF CACHE INTERNAL "GLFW install.")
set(GLFW_DOCUMENT_INTERNALS OFF CACHE INTERNAL "GLFW document internals.")
set(GLFW_USE_EGL OFF CACHE INTERNAL "GLFW use EGL.")
set(GLFW_USE_HYBRID_HPG OFF CACHE INTERNAL "GLFW use hybrid HPG.")
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE INTERNAL "MSCV runtime library dll.")
set(LIB_SUFFIX "" CACHE INTERNAL "Suffix of lib.")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "GLFW build shared libs.")
add_subdirectory("${PROJECT_EXTERNALS_PATH}/glew-cmake")
set(GLFW3_LIBRARY glfw)

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

#DevIL
#link_dependency(DevIL)
set(BUILD_SHARED_LIBS OFF)
add_definitions("-fPIC")
add_subdirectory("${PROJECT_EXTERNALS_PATH}/DEVIL/DevIL")
#file(GLOB_RECURSE ASSIMP
#    "${PROJECT_EXTERNALS_PATH}/ASSIMP/src/*")
include_directories("${PROJECT_EXTERNALS_PATH}/DEVIL/include")
include_directories("${PROJECT_EXTERNALS_PATH}/DEVIL/DevIL/include")
LINK_DIRECTORIES("${CMAKE_BINARY_DIR}/externals/DEVIL/DevIL/src-IL")
LINK_DIRECTORIES("${CMAKE_BINARY_DIR}/externals/DEVIL/DevIL/src-ILU")
LINK_DIRECTORIES("${CMAKE_BINARY_DIR}/externals/DEVIL/DevIL/src-ILUT")
set(DevIL
    DevIL
    ILU
    ILUT)

# ASSIMP
#link_dependency(ASSIMP)
set(ASSIMP_BUILD_ASSIMP_TOOLS false)
add_subdirectory("${PROJECT_EXTERNALS_PATH}/ASSIMP")
#file(GLOB_RECURSE ASSIMP
#    "${PROJECT_EXTERNALS_PATH}/ASSIMP/code/*")
include_directories("${PROJECT_EXTERNALS_PATH}/ASSIMP/include")
LINK_DIRECTORIES("${CMAKE_BINARY_DIR}/externals/ASSIMP/code")

# ImGui
set(IM_GUI_DIR ${PROJECT_EXTERNALS_PATH}/IMGUI)
file(GLOB IM_GUI
    ${IM_GUI_DIR}/*.h
    ${IM_GUI_DIR}/*.cpp
    ${IM_GUI_DIR}/examples/opengl3_example/imgui_impl_glfw_gl3.cpp)
include_directories(${IM_GUI_DIR})

# Pipe path to ImGui fonts
add_definitions(-DIMGUI_FONTS_PATH="${PROJECT_EXTERNALS_PATH}/IMGUI/extra_fonts")

#native file dialog
set(NFD_DIR ${PROJECT_EXTERNALS_PATH}/NATIVEFILEDIALOG)

include_directories(${NFD_DIR}/src/include)

if("${CMAKE_SYSTEM}" MATCHES "Linux")
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
# Add other flags to the compiler
add_definitions(${GTK3_CFLAGS_OTHER})
endif()
file(GLOB NFD
    ${NFD_DIR}/src/*.h
    ${NFD_DIR}/src/*.c)

#link_dependency(ZLIB)

# Collect all code
set(EXTERNAL_CODE
        ${GLM}
        ${GLEW}
${IM_GUI}
${NFD}
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

if(EXISTS ${PROJECT_SHADERS_PATH})
	add_subdirectory(${PROJECT_SHADERS_PATH})
endif()



file (COPY "${CMAKE_MODULE_PATH}/gdb_prettyprinter.py" DESTINATION ${PROJECT_BINARY_DIR})
