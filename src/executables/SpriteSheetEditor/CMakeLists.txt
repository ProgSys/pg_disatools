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
cmake_minimum_required(VERSION 3.14.0)
set(UI_FILES spritesheeteditor.ui compileErrorBox.ui createNewAnimation.ui createNewSpriteSheet.ui resizeDialog.ui)
set(RESOURCE_FILES "${PROJECT_ICONS_PATH}/sprite_sheet_editor_icon.res") #windres aminoVisIcon.rc -O coff -o aminoVisIcon.res
set(VERSION_NUMBER 0.1.7)
set(EXECUTABLE_TITLE ${TITLE_SpriteSheetEditor})
include(${CMAKE_MODULE_PATH}/generators/QtExecutable.cmake)
target_compile_definitions(${ProjectId} PRIVATE TITLE_SpriteSheetEditor="${TITLE_SpriteSheetEditor}" )