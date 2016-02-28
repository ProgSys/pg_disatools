/*
 * PG_JavaBridge.cpp
 *
 *  Created on: 27.02.2016
 *      Author: ProgSys
 */

#include <pg/util/PG_JavaBridge.h>
#include <stdexcept>

namespace PG {
namespace UTIL {

void simpleCall(){
	int x = 3;
	return;
}

int multiply(float a, float b, float & result){
	result = a * b;
	return 0;
}

char* uncompressTX2Image(const char* bytes, int lengh, int& width, int& height){

	char* rgbabytes = new char[4*4*4]();
	rgbabytes[0] = bytes[0];
	rgbabytes[2] = 5;
	rgbabytes[5] = 5;

	width = 4;
	height = 4;

	return rgbabytes;
}

} /* namespace UTIL */
} /* namespace PG */
