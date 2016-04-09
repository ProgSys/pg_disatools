/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2016  ProgSys
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, version 3 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with this program.  If not, see http://doc.qt.io/qt-5/lgpl.html
 *	or http://www.gnu.org/licenses/
 */
#ifndef INCLUDE_PG_GENERIC_PG_GLERROR_H_
#define INCLUDE_PG_GENERIC_PG_GLERROR_H_

#include <pg/util/PG_Base.h>

bool check_gl_error(const char *file, int line);

#define checkGLError() check_gl_error(FUNCTION_NAME,__LINE__)


#endif /* INCLUDE_PG_GENERIC_PG_GLERROR_H_ */
