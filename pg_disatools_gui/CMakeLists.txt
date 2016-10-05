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

set (FileManagerTITLE "Disa PC File Manager v0.5.2 alpha")
set (SpriteSheetEditorTITLE "Sprite Sheet Editor v0.3.2 alpha")
set (DATAEditorTITLE "Data Editor v0.0.6 alpha")
set (MapEditorTITLE "Map Viewer v0.0.1 development")
set (TX2EditorTITLE "TX2 Editor v0.0.2 alpha")


#add_definitions(-DSpriteSheetEditorTITLE=${SpriteSheetEditorTITLE})
#add_definitions(-DDATAEditorTITLE=${DATAEditorTITLE})
#add_definitions(-DMapEditorTITLE=${MapEditorTITLE})

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

find_package(Qt5Quick REQUIRED)
find_package(Qt5QuickWidgets REQUIRED)

find_package(PG_DISATOOLS REQUIRED)

# OpenGL & GLEW library
find_package(OpenGL3 REQUIRED)
find_package(GLEW REQUIRED)

QT5_WRAP_UI(UIMainWindowSrcs ui/mainwindow.ui)
QT5_WRAP_UI(UIFileInfoBoxSrcs ui/fileinfobox.ui)
QT5_WRAP_UI(UISpriteSheetEditorSrcs ui/spritesheeteditor.ui)
QT5_WRAP_UI(UIEnterValueSrcs ui/entervaluedialog.ui)
QT5_WRAP_UI(UICreateNewSpriteSrcs ui/createNewSpriteSheet.ui)
QT5_WRAP_UI(UISreateNewAnimationSrcs ui/createNewAnimation.ui)
QT5_WRAP_UI(UIDataEditor ui/dataEditor.ui)
QT5_WRAP_UI(UIMapEditor ui/mapEditor.ui)
QT5_WRAP_UI(UItx2Editor ui/tx2Editor.ui)
QT5_WRAP_UI(UItx2ConvertTo ui/tx2ConvertTo.ui)

#windres disa_tools_icon.rc -O coff -o disa_tools_icon.res
SET(sim_EXE_SRCS "")
if(MINGW)
SET(sim_EXE_SRCS ${sim_EXE_SRCS} "${PROJECT_SOURCE_DIR}/../icons/disa_tools_icon.res")
message (STATUS "Icon File: ${sim_EXE_SRCS} ")
endif(MINGW)

#windres sprite_sheet_editor_icon.rc -O coff -o sprite_sheet_editor_icon.res
SET(SPRITE_EDITOR_EXE_SRCS "")
if(MINGW)
SET(SPRITE_EDITOR_EXE_SRCS ${SPRITE_EDITOR_EXE_SRCS} "${PROJECT_SOURCE_DIR}/../icons/sprite_sheet_editor_icon.res")
message (STATUS "Sprite editor icon File: ${SPRITE_EDITOR_EXE_SRCS} ")
endif(MINGW)

#windres data_editor_icon.rc -O coff -o data_editor_icon.res
SET(DATA_EDITOR_EXE_SRCS "")
if(MINGW)
SET(DATA_EDITOR_EXE_SRCS ${DATA_EDITOR_EXE_SRCS} "${PROJECT_SOURCE_DIR}/../icons/data_editor_icon.res")
message (STATUS "Data editor icon File: ${DATA_EDITOR_EXE_SRCS} ")
endif(MINGW)

#windres map_editor_icon.rc -O coff -o map_editor_icon.res
SET(MAP_EDITOR_EXE_SRCS "")
if(MINGW)
SET(MAP_EDITOR_EXE_SRCS ${MAP_EDITOR_EXE_SRCS} "${PROJECT_SOURCE_DIR}/../icons/map_editor_icon.res")
message (STATUS "Map editor icon File: ${MAP_EDITOR_EXE_SRCS} ")
endif(MINGW)

#windres tx2_editor_icon.rc -O coff -o tx2_editor_icon.res
SET(TX2_EDITOR_EXE_SRCS "")
if(MINGW)
SET(TX2_EDITOR_EXE_SRCS ${TX2_EDITOR_EXE_SRCS} "${PROJECT_SOURCE_DIR}/../icons/tx2_editor_icon.res")
message (STATUS "TX2 editor icon File: ${TX2_EDITOR_EXE_SRCS} ")
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

message (STATUS "PG_DISATOOLS_INCLUDE_PATH: ${PG_DISATOOLS_INCLUDE_PATH}")
message (STATUS "Project binary dir: ${PROJECT_BINARY_DIR}")
message (STATUS "Project source dir: ${PROJECT_SOURCE_DIR}")

#===============================================
#   pg_disatools_gui Files
#===============================================
file(GLOB PG_FILE_MENEGER_SRC
	#"*.cpp"
	"src/*.cpp"
	"src/util/*.cpp"
	"src/fileMenager/*.cpp"
)

file(GLOB PG_FILE_MENEGER_HEADERS
	#"*.h"
	"include/*.h"
	"include/util/*.h"
	"include/fileMenager/*.h"
)

#===============================================
# pg_file_manager
#===============================================
if(DEBUG_COMPILE)
add_executable( 
	pg_file_manager
		"fileManager.cpp"
		${PG_FILE_MENEGER_HEADERS}
		${PG_FILE_MENEGER_SRC}
		${MOCSrcs} ${UIMainWindowSrcs} ${UIFileInfoBoxSrcs}  ${UIEnterValueSrcs}
		
)
else()
add_executable( 
	pg_file_manager
		WIN32
		"fileManager.cpp"
		${PG_FILE_MENEGER_HEADERS}
		${PG_FILE_MENEGER_SRC}
		${MOCSrcs} ${UIMainWindowSrcs} ${UIFileInfoBoxSrcs}  ${UIEnterValueSrcs}
		
)
set_target_properties(pg_file_manager PROPERTIES OUTPUT_NAME ${FileManagerTITLE})
endif()

target_link_libraries(
	pg_file_manager
	${OpenGL3_LIBRARY}
	${GLEW_LIBRARY}
	
	Qt5::Widgets
	#Qt5::OpenGL
	${PG_DISATOOLS_LIBRARY}
	${sim_EXE_SRCS}
)


#===============================================
#   pg_sprite_editor Files
#===============================================
file(GLOB PG_SPRITE_EDITOR_SRC
	#"*.cpp"
	"src/*.cpp"
	"src/util/*.cpp"
	"src/openGL/*.cpp"
	"src/spriteSheetEditor/*.cpp"
)

file(GLOB PG_SPRITE_EDITOR_HEADERS
	#"*.h"
	"include/*.h"
	"include/util/*.h"
	"include/openGL/*.h"
	"include/spriteSheetEditor/*.h"
)

#===============================================
# pg_sprite_editor
#===============================================

if(DEBUG_COMPILE)
add_executable( 
	pg_sprite_editor
		"spriteEditor.cpp"
		${PG_SPRITE_EDITOR_HEADERS}
		${PG_SPRITE_EDITOR_SRC}
		${MOCSrcs} ${UISpriteSheetEditorSrcs} ${UICreateNewSpriteSrcs} ${UISreateNewAnimationSrcs}
		
)
else()
add_executable( 
	pg_sprite_editor
		WIN32
		"spriteEditor.cpp"
		${PG_SPRITE_EDITOR_HEADERS}
		${PG_SPRITE_EDITOR_SRC}
		${MOCSrcs} ${UISpriteSheetEditorSrcs} ${UICreateNewSpriteSrcs} ${UISreateNewAnimationSrcs} 
		
)
set_target_properties(pg_sprite_editor PROPERTIES OUTPUT_NAME ${SpriteSheetEditorTITLE})
endif()

target_link_libraries(
	pg_sprite_editor
	${OpenGL3_LIBRARY}
	${GLEW_LIBRARY}
	
	Qt5::Widgets
	Qt5::OpenGL
	Qt5::Qml
	Qt5::Quick
	Qt5::QuickWidgets
	${PG_DISATOOLS_LIBRARY}
	${SPRITE_EDITOR_EXE_SRCS}
)

#===============================================
#   pg_data_editor Files
#===============================================
file(GLOB PG_DATA_EDITOR_SRC
	#"*.cpp"
	"src/*.cpp"
	"src/util/*.cpp"
	"src/dataEditor/*.cpp"
)

file(GLOB PG_DATA_EDITOR_HEADERS
	#"*.h"
	"include/*.h"
	"include/util/*.h"
	"include/dataEditor/*.h"
)

#===============================================
# pg_data_editor
#===============================================

if(DEBUG_COMPILE)
add_executable( 
	pg_data_editor
		"dataEditor.cpp"
		${PG_DATA_EDITOR_HEADERS}
		${PG_DATA_EDITOR_SRC}
		${MOCSrcs} ${UIDataEditor} 
		
)
else()
add_executable( 
	pg_data_editor
		WIN32
		"dataEditor.cpp"
		${PG_DATA_EDITOR_HEADERS}
		${PG_DATA_EDITOR_SRC}
		${MOCSrcs} ${UIDataEditor}  
		
)
set_target_properties(pg_data_editor PROPERTIES OUTPUT_NAME ${DATAEditorTITLE})
endif()

target_link_libraries(
	pg_data_editor
	${OpenGL3_LIBRARY}
	${GLEW_LIBRARY}
	
	Qt5::Widgets
	${PG_DISATOOLS_LIBRARY}
	${DATA_EDITOR_EXE_SRCS}
)

#===============================================
#   pg_map_editor Files
#===============================================
file(GLOB PG_MAP_EDITOR_SRC
	#"*.cpp"
	"src/*.cpp"
	"src/util/*.cpp"
	"src/openGL/*.cpp"
	"src/mapEditor/*.cpp"
)

file(GLOB PG_MAP_EDITOR_HEADERS
	#"*.h"
	"include/*.h"
	"include/util/*.h"
	"include/openGL/*.h"
	"include/mapEditor/*.h"
)

#===============================================
# pg_map_editor
#===============================================

if(DEBUG_COMPILE)
add_executable( 
	pg_map_editor
		"mapEditor.cpp"
		${PG_MAP_EDITOR_HEADERS}
		${PG_MAP_EDITOR_SRC}
		${MOCSrcs} ${UIMapEditor}
		
)
else()
add_executable( 
	pg_map_editor
		WIN32
		"mapEditor.cpp"
		${PG_MAP_EDITOR_HEADERS}
		${PG_MAP_EDITOR_SRC}
		${MOCSrcs} ${UIMapEditor} 
		
)
set_target_properties(pg_map_editor PROPERTIES OUTPUT_NAME ${MapEditorTITLE})
endif()

target_link_libraries(
	pg_map_editor
	${OpenGL3_LIBRARY}
	${GLEW_LIBRARY}
	Qt5::Widgets
	Qt5::OpenGL
	${PG_DISATOOLS_LIBRARY}
	${MAP_EDITOR_EXE_SRCS}
)

#===============================================
#   pg_tx2_editor Files
#===============================================
file(GLOB PG_TX2_EDITOR_SRC
	#"*.cpp"
	"src/*.cpp"
	"src/util/*.cpp"
	"src/tx2Editor/*.cpp"
)

file(GLOB PG_TX2_EDITOR_HEADERS
	#"*.h"
	"include/*.h"
	"include/util/*.h"
	"include/tx2Editor/*.h"
)

#===============================================
# pg_tx2_editor
#===============================================

if(DEBUG_COMPILE)
add_executable( 
	pg_tx2_editor
		"tx2Editor.cpp"
		${PG_TX2_EDITOR_HEADERS}
		${PG_TX2_EDITOR_SRC}
		${MOCSrcs} ${UItx2Editor} ${UItx2ConvertTo} 
		
)
else()
add_executable( 
	pg_tx2_editor
		WIN32
		"tx2Editor.cpp"
		${PG_TX2_EDITOR_HEADERS}
		${PG_TX2_EDITOR_SRC}
		${MOCSrcs} ${UItx2Editor} ${UItx2ConvertTo} 
		
)
set_target_properties(pg_tx2_editor PROPERTIES OUTPUT_NAME ${TX2EditorTITLE})
endif()

target_link_libraries(
	pg_tx2_editor
	
	Qt5::Widgets
	Qt5::Qml
	Qt5::Quick
	Qt5::QuickWidgets
	${PG_DISATOOLS_LIBRARY}
	${TX2_EDITOR_EXE_SRCS}
)

target_compile_definitions(pg_file_manager PRIVATE FileManagerTITLE="${FileManagerTITLE}" SpriteSheetEditorTITLE="${SpriteSheetEditorTITLE}" DATAEditorTITLE="${DATAEditorTITLE}")
target_compile_definitions(pg_sprite_editor PRIVATE SpriteSheetEditorTITLE="${SpriteSheetEditorTITLE}")
target_compile_definitions(pg_data_editor PRIVATE DATAEditorTITLE="${DATAEditorTITLE}")
target_compile_definitions(pg_map_editor PRIVATE MapEditorTITLE="${MapEditorTITLE}")
target_compile_definitions(pg_tx2_editor PRIVATE TX2EditorTITLE="${TX2EditorTITLE}")

GroupSources("include/pg")
GroupSources("src/pg")
#[[]]
#===============================================
# install 
#===============================================
install(
	TARGETS pg_file_manager pg_sprite_editor pg_data_editor pg_map_editor pg_tx2_editor
		RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
		LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
		ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/static
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
