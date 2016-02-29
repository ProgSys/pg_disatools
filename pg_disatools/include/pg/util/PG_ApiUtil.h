/*
 * Defines for dllexport.
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


#ifndef INCLUDE_PG_UTIL_PG_APIUTIL_H_
#define INCLUDE_PG_UTIL_PG_APIUTIL_H_

#if defined PG_UTIL_DLL_CONFIG && defined _MSC_VER && !(defined __INTEL_COMPILER)
#ifdef PG_UTIL_EXPORTS
	#define PG_UTIL_API __declspec(dllexport)
#else
	#define PG_UTIL_API __declspec(dllimport)
#endif
#else
	#define PG_UTIL_API __declspec(dllexport)
#endif

#endif /* INCLUDE_PG_UTIL_PG_APIUTIL_H_ */
