/*
 * PG_JavaBridge.h
 *
 *  Created on: 27.02.2016
 *      Author: ProgSys
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
