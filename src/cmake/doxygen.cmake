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


#-- Add an Option to toggle the generation of the API documentation

OPTION(BUILD_DOCUMENTATION "HTML API Dokumentation mit Doxygen erstellen" OFF)
IF(BUILD_DOCUMENTATION)

  FIND_PACKAGE(Doxygen)
  IF (NOT DOXYGEN_FOUND)
    MESSAGE("Doxygen wird für die Dokumentation benötigt. Bitte installiere es.")
  ELSE()

    #-- Configure the Template Doxyfile for our specific project
    CONFIGURE_FILE(${CMAKE_MODULE_PATH}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY IMMEDIATE)

    #-- Add a custom target to run Doxygen when ever the project is built
    ADD_CUSTOM_TARGET (Docs ALL
    	COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
    	SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
    
    # IF you do NOT want the documentation to be generated EVERY time you build the project
    # then leave out the 'ALL' keyword from the above command.
  ENDIF()
ENDIF()