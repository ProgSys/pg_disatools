# Unofficial tools for Disgaea PC

## Last Build

Please make a backup of your files before using this tools!

You can download the latest version of the tools here:

* [Version v0.1 alpha](https://dl.dropboxusercontent.com/u/33065226/disgaea/Disgaea%20Tools%20v0.1%20alpha.zip)
* [Version v0.2 alpha](https://dl.dropboxusercontent.com/u/33065226/disgaea/Disgaea%20Tools%20v0.2%20alpha.zip)

###Content
#### PSPfsExtractor

Java application for extracting and packing files from a Disgaea PC DATA.DAT archive.

To run it you need to have Java installed: https://www.java.com/en/download/

![gui image](https://dl.dropboxusercontent.com/u/33065226/disgaea/java_gui_preview5.png)

#### TX2Converter.exe

A command line tool which allows you to convert Disgaea PC *.TX2 and *.MPP textures to *.TGA (BGRA8888) or *.PGM (P6 RBG888 No alpha).
Sadly the other way around is not supported yet.
Make a backup before using this tool!

#### Known problems/TODOS:

* The positioning of the DUMMY files inside the DATA.DAT is wrong.

# Disclaimer && License
This tools are unofficial and they are not perfect and may have errors. I do not work for Nippon Ichi Software, Inc. or NIS America, Inc. . 

## PSPfsExtractor
This project uses the JNI (Java Native Interface) Library, which requires a different license.

### GNU General Public License

	Copyright (C) 2016  ProgSys

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
## TX2Converter.exe && libpg_disatools.dll
My libraries and command line tools use the MIT License.

### The MIT License (MIT)

	Copyright (c) 2016 ProgSys

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.