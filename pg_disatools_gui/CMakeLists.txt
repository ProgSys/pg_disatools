# run: mingw32-make -DCMAKE_INSTALL_PREFIX=C:/Users/ProgSys/Desktop/install install
cmake_minimum_required(VERSION 2.8.11)
project(pg_disatools_gui)


SET(DEPENDENCIES_PATH "${PROJECT_SOURCE_DIR}/../dependencies" CACHE FILEPATH "Path to the dependencies folder, which contains the include and lib files." )
SET(CMAKE_CONFIGURATION_TYPES Debug;Release)
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake")

option(DEBUG_COMPILE "Compile in debug mode" true)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set (CMAKE_PREFIX_PATH "D:/Qt/5.5/mingw492_32/lib/cmake") 


if(DEBUG_COMPILE)
	SET(CMAKE_BUILD_TYPE Debug CACHE FILEPATH "" FORCE)
	message (STATUS "DEBUG is ON")
	add_definitions(-DDEBUG)
else()
	SET(CMAKE_BUILD_TYPE Release CACHE FILEPATH "" FORCE)
	message (STATUS "Release is ON")
endif()

include(macros)
#Set default path
Setup_defaults(${CMAKE_PROJECT_NAME})


#find_package(Qt5 COMPONENTS Widgets Declarative OpenGL REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(PG_DISATOOLS REQUIRED)

# OpenGL & GLEW library
find_package(OpenGL3 REQUIRED)
find_package(GLEW REQUIRED)

QT5_WRAP_UI(UIMainWindowSrcs ui/mainwindow.ui)
QT5_WRAP_UI(UIFileInfoBoxSrcs ui/fileinfobox.ui)
QT5_WRAP_UI(UISpriteSheetEditorSrcs ui/spritesheeteditor.ui)
QT5_WRAP_UI(UIEnterValueSrcs ui/entervaluedialog.ui)

#windres disa_tools_icon.rc -O coff -o disa_tools_icon.res
SET(sim_EXE_SRCS "")
if(MINGW)
SET(sim_EXE_SRCS ${sim_EXE_SRCS} "${PROJECT_SOURCE_DIR}/../icons/disa_tools_icon.res")
message (STATUS "Icon File: ${sim_EXE_SRCS} ")
endif(MINGW)

SET(SPRITE_EDITOR_EXE_SRCS "")
if(MINGW)
SET(SPRITE_EDITOR_EXE_SRCS ${SPRITE_EDITOR_EXE_SRCS} "${PROJECT_SOURCE_DIR}/../icons/sprite_sheet_editor_icon.res")
message (STATUS "Sprite editor icon File: ${SPRITE_EDITOR_EXE_SRCS} ")
endif(MINGW)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	message (STATUS "DEBUG is ON")
endif()

message (STATUS "Modul path: ${CMAKE_MODULE_PATH}")

if(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions(-std=gnu++11)
endif()


# where the include files are located
include_directories(
	"${PROJECT_SOURCE_DIR}/include"
	${OpenGL3_INCLUDE_PATH}
    ${GLEW_INCLUDE_PATH}
	${Qt5Widgets_INCLUDES}
	${QT_QTOPENGL_INCLUDE_DIR} 
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
#   PG_QT
#===============================================
file(GLOB PG_QT_SRC
	#"*.cpp"
	"src/*.cpp"
	"src/openGL/*.cpp"
)

file(GLOB PG_QT_HEADERS
	#"*.h"
	"include/*.h"
	"include/openGL/*.h"
)

GroupSources("include/pg")
GroupSources("src/pg")

message (${PG_QT_SRC})

#===============================================
# pg_disatools_gui
#===============================================
if(DEBUG_COMPILE)
add_executable( 
	pg_disatools_gui
		"main.cpp"
		${PG_QT_HEADERS}
		${PG_QT_SRC}
		${MOCSrcs} ${UIMainWindowSrcs} ${UIFileInfoBoxSrcs} ${UISpriteSheetEditorSrcs} ${UIEnterValueSrcs}
		
)
else()
add_executable( 
	pg_disatools_gui
		WIN32
		"main.cpp"
		${PG_QT_HEADERS}
		${PG_QT_SRC}
		${MOCSrcs} ${UIMainWindowSrcs} ${UIFileInfoBoxSrcs} ${UISpriteSheetEditorSrcs} ${UIEnterValueSrcs}
		
)
endif()

target_link_libraries(
	pg_disatools_gui
	${OpenGL3_LIBRARY}
	${GLEW_LIBRARY}
	
	Qt5::Widgets
	Qt5::OpenGL
	${PG_DISATOOLS_LIBRARY}
	${sim_EXE_SRCS}
)


#===============================================
# pg_sprite_editor
#===============================================

if(DEBUG_COMPILE)
add_executable( 
	pg_sprite_editor
		"spriteEditor.cpp"
		${PG_QT_HEADERS}
		${PG_QT_SRC}
		${MOCSrcs} ${UISpriteSheetEditorSrcs}
		
)
else()
add_executable( 
	pg_sprite_editor
		WIN32
		"spriteEditor.cpp"
		${PG_QT_HEADERS}
		${PG_QT_SRC}
		${MOCSrcs} ${UISpriteSheetEditorSrcs}
		
)
endif()

target_link_libraries(
	pg_sprite_editor
	${OpenGL3_LIBRARY}
	${GLEW_LIBRARY}
	
	Qt5::Widgets
	Qt5::OpenGL
	${PG_DISATOOLS_LIBRARY}
	${SPRITE_EDITOR_EXE_SRCS}
)

#===============================================
# install pg_disatools_gui
#===============================================
install(
	TARGETS pg_disatools_gui pg_sprite_editor
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
# install debug/release
#===============================================
#[[
ADD_CUSTOM_TARGET(debug
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
  COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
  )

ADD_CUSTOM_TARGET(release
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
  COMMENT "Switch CMAKE_BUILD_TYPE to Release"
  )
]]
#===============================================
# clean
#===============================================
set_directory_properties(PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
	"${CMAKE_INSTALL_PREFIX}"
)
