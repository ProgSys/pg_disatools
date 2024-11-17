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
 
cmake_minimum_required(VERSION 3.14.0)
project(DisaTools VERSION 1.0.0 LANGUAGES CXX HOMEPAGE_URL "https://github.com/ProgSys/pg_disatools" DESCRIPTION "Modding tools for Disgaea PC")

set(PROJECT_GLOBAL_NUMBER 1.0.0 CACHE PATH "Project version number.")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake CACHE PATH "Project specific path. Set manually if it was not found.")

set (TITLE_FileManager "Disa PC File Manager v0.6.2 alpha" CACHE PATH "Tilte")
set (TITLE_SpriteSheetEditor "Sprite Sheet Editor v0.4.8 alpha" CACHE PATH "Tilte")
set (TITLE_DATAEditor "Data Editor v0.0.8 alpha" CACHE PATH "Tilte")
set (TITLE_MapEditor "Data Map Editor v0.2.9 alpha" CACHE PATH "Tilte")
set (TITLE_TX2Editor "TX2 Editor v0.0.3 alpha" CACHE PATH "Tilte")

include(${CMAKE_MODULE_PATH}/generators/QtProject.cmake)
