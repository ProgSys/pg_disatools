/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2016 ProgSys
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
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
 */

#ifndef INCLUDE_PG_UTIL_PG_APIUTIL_H_
#define INCLUDE_PG_UTIL_PG_APIUTIL_H_

//#define EXPORT __attribute__ ((dllexport))
//#define EXPORT __declspec(dllexport)

#define EXPORT

/*
#if defined PG_UTIL_DLL_CONFIG && defined _MSC_VER && !(defined __INTEL_COMPILER)
#ifdef EXPORTS
	#define EXPORT __declspec(dllexport)
#else
	#define EXPORT __declspec(dllimport)
#endif
#else
	#define EXPORT
#endif
*/
#ifdef _MSC_VER
#define PACK( ... ) __pragma( pack(push, 1) ) __VA_ARGS__ __pragma( pack(pop) )
#else
//__attribute__((packed, aligned(1)));
#define PACK( ... ) __VA_ARGS__ __attribute__((packed, aligned(1)))
#endif


inline constexpr unsigned char operator "" _c(unsigned long long arg) noexcept {
	return static_cast<char>(arg);
}


#endif /* INCLUDE_PG_UTIL_PG_APIUTIL_H_ */
