# run: mingw32-make -DCMAKE_INSTALL_PREFIX=C:/Users/ProgSys/Desktop/install install
cmake_minimum_required(VERSION 2.8.11)
project(pg_disatools_gui)

SET(DEPENDENCIES_PATH "${PROJECT_SOURCE_DIR}/../dependencies" CACHE FILEPATH "Path to the dependencies folder, which contains the include and lib files." )
SET(CMAKE_CONFIGURATION_TYPES Debug;Release)
SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set (CMAKE_PREFIX_PATH "D:/Qt/5.5/mingw492_32/lib/cmake") 

find_package(Qt5 COMPONENTS Widgets Declarative)
find_package(PG_DISATOOLS REQUIRED)
#FIND_PACKAGE(Qt5Widgets REQUIRED)
#INCLUDE(${QT_USE_FILE})
 
QT5_WRAP_UI(UISrcs mainwindow.ui)
#QT5_WRAP_CPP(MOCSrcs ui_mainwindow.h)

include(macros)

#Set default path
Setup_defaults(${CMAKE_PROJECT_NAME})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	message (STATUS "DEBUG is ON")
	add_definitions(-DDEBUG)
endif()

#add_definitions(-fPIC)



message (STATUS "Modul path: ${CMAKE_MODULE_PATH}")

if(CMAKE_COMPILER_IS_GNUCXX)
    #add_definitions(-std=g++0x) #-std=gnu++0x
	add_definitions(-std=gnu++11)
	#add_definitions(-DGLEW_STATIC=TRUE) -std=gnu++0x
else()
	#add_definitions(-DPG_LOADERS_EXPORTS)
	#add_definitions(-DPG_LOADERS_DLL_CONFIG)
endif()

# where the include files are located
include_directories(
	"${PROJECT_SOURCE_DIR}"
	${Qt5Widgets_INCLUDES}
	${PG_DISATOOLS_INCLUDE_PATH}
)

add_definitions(${Qt5Widgets_DEFINITIONS})

set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

link_directories(
	"${PROJECT_SOURCE_DIR}"
)


message (STATUS "Project binary dir: ${PROJECT_BINARY_DIR}")
message (STATUS "Project source dir: ${PROJECT_SOURCE_DIR}")

#===============================================
#   PG_LOADERS
#===============================================
file(GLOB PG_LOADERS_SRC
	"*.cpp"
	
	"src/pg/test/*.cpp"
	
	"src/pg/loaders/*.cpp"
	
	"src/pg/savers/*.cpp"

	"src/pg/files/*.cpp"
	
    "src/pg/util/*.cpp"

)

file(GLOB PG_LOADERS_HEADERS
	"*.h"
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

#===============================================
# pg_disatools_gui
#===============================================

add_executable(
	pg_disatools_gui
		${PG_LOADERS_HEADERS}
		${PG_LOADERS_SRC}
		${MOCSrcs} ${UISrcs}
		
)

target_link_libraries(
	pg_disatools_gui
	${Qt5Widgets_LIBRARIES}
	${PG_DISATOOLS_LIBRARY}
	
)

#===============================================
# install
#===============================================
install(
	TARGETS pg_disatools_gui
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
