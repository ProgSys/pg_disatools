/*
 * PG_GLError.h
 *
 *  Created on: 23.12.2015
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_GENERIC_PG_GLERROR_H_
#define INCLUDE_PG_GENERIC_PG_GLERROR_H_

#include <pg/util/PG_Base.h>

bool check_gl_error(const char *file, int line);

#define checkGLError() check_gl_error(FUNCTION_NAME,__LINE__)


#endif /* INCLUDE_PG_GENERIC_PG_GLERROR_H_ */
