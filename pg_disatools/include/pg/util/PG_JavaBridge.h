/*
 * Exports for java JNI.
 *
 *  Copyright (C) 2016  ProgSys
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_PG_UTIL_PG_JAVABRIDGE_H_
#define INCLUDE_PG_UTIL_PG_JAVABRIDGE_H_

namespace PG {
namespace UTIL {

//void  simpleCall();

extern "C" {

void __declspec(dllexport) simpleCall();
int __declspec(dllexport) multiply (float a, float b, float & result);
char* __declspec(dllexport) uncompressTX2Image (const char* bytes, int length, int& width, int& height);


}

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_JAVABRIDGE_H_ */
