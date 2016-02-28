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

char* uncompressTX2Image(const char* bytes, int length, int& width, int& height){



	return nullptr;
}

} /* namespace UTIL */
} /* namespace PG */
