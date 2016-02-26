# run: mingw32-make -DCMAKE_INSTALL_PREFIX=C:/Users/ProgSys/Desktop/install install
cmake_minimum_required(VERSION 2.8.3)
project(pg_disatools)

SET(DEPENDENCIES_PATH "${PROJECT_SOURCE_DIR}/../dependencies" CACHE FILEPATH "Path to the dependencies folder, which contains the include and lib files." )
SET(CMAKE_CONFIGURATION_TYPES Debug;Release)
SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake")

include(macros)

#Set default path
Setup_defaults(${CMAKE_PROJECT_NAME})


if(UNIX)
	add_definitions(-fPIC)
endif()

SET(ADDITIONAL_CCP  "${LODEPNG_INCLUDE_PATH}/*.cpp")

message (STATUS "Modul path: ${CMAKE_MODULE_PATH}")
#find_package(GLM REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=gnu++0x) #-std=gnu++0x
	#add_definitions(-DGLEW_STATIC=TRUE) -std=gnu++0x
else()
	#add_definitions(-DPG_LOADERS_EXPORTS)
	#add_definitions(-DPG_LOADERS_DLL_CONFIG)
endif()

# where the include files are located
include_directories(
	"${PROJECT_SOURCE_DIR}/include"
	#${GLM_INCLUDE_PATH}
)


link_directories(
	"${PROJECT_SOURCE_DIR}/src"
)


message (STATUS "Project binary dir: ${PROJECT_BINARY_DIR}")
message (STATUS "Project source dir: ${PROJECT_SOURCE_DIR}")
message (STATUS ${ADDITIONAL_CCP})
#===============================================
#   PG_LOADERS
#===============================================
file(GLOB PG_LOADERS_SRC
	"${ADDITIONAL_CCP}"

    "src/*.cpp"
	
	"src/pg/test/*.cpp"
	
	"src/pg/loaders/*.cpp"
	
	"src/pg/savers/*.cpp"

	"src/pg/files/*.cpp"
	
    "src/pg/util/*.cpp"

)

file(GLOB PG_LOADERS_HEADERS
	"${DEBUG_LEAK_H}"
	"include/*.h"
	
	"include/pg/*.h"
	
	"include/pg/test/*.h"
	
	"include/pg/loaders/*.h"
	
	"include/pg/savers/*.h"

	"include/pg/files/*.h"
	
    "include/pg/util/*.h"

)

GroupSources("include/pg")
GroupSources("src/pg")
#source_group(source FILES ${PG_ENGINE_SRC})

message (${PG_LOADERS_SRC})
add_library(
	#static lib
	pg_disatools SHARED
		${PG_LOADERS_HEADERS}
		${PG_LOADERS_SRC}
)

target_link_libraries(
	pg_disatools
)
#===============================================
# disa_tools_test
#===============================================

add_executable(
	disa_tools_test
		"src/main.cpp"
)

target_link_libraries(
	disa_tools_test
		pg_disatools
)

#===============================================
# install
#===============================================
install(
	TARGETS pg_disatools disa_tools_test
		RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
		LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
		ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/static
)
install(
	TARGETS pg_disatools disa_tools_test
		RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
		LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
		ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/static
)
# install include files
install(
	DIRECTORY include
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		FILES_MATCHING PATTERN "*.h"
)

#===============================================
# clean
#===============================================
set_directory_properties(PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
	"${CMAKE_INSTALL_PREFIX}"
)
