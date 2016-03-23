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

#ifndef INCLUDE_PG_UTIL_PG_ARRAY_H_
#define INCLUDE_PG_UTIL_PG_ARRAY_H_

#include <iterator>
#include <initializer_list>
#include <ostream>
#include <pg/util/PG_Exception.h>

namespace PG {
namespace UTIL {

/*!
 * @brief A wrapper struct around a array, so it is easier to use. Similar to std::array.
 */
template <typename T, unsigned int arraySize>
struct Array {
	T array[arraySize];

	Array(){}
	Array(const Array<T,arraySize>& arr){
		std::copy(arr.begin(), arr.end(), array);
	}
	Array(const T (&arr)[arraySize]){
		std::copy(&arr[0], &arr[arraySize], array);
	}
	Array( std::initializer_list<T> l){
		auto end = l.size() <= arraySize ? l.end() : l.begin() + arraySize;
		std::copy(l.begin(), end, array);
	}

	void operator=(const Array<T,arraySize>& arr){
		std::copy(arr.begin(), arr.end(), array);
	}

	void operator=(const T (&arr)[arraySize]){
		std::copy(&arr[0], &arr[arraySize], array);
	}

	void operator=(std::initializer_list<T> l){
		auto end = l.size() <= arraySize ? l.end() : l.begin() + arraySize;
		std::copy(l.begin(), end, array);
	}

	T& get(unsigned int index){
		assert_Test("Out of bound!", index >= arraySize);
		return array[index];
	}

	const T& get(unsigned int index) const{
		assert_Test("Out of bound!", index >= arraySize);
		return array[index];
	}

	T& operator[](unsigned int index){
		assert_Test("Out of bound!", index >= arraySize);
		return array[index];
	}

	const T& operator[](unsigned int index) const{
		assert_Test("Out of bound!", index >= arraySize);
		return array[index];
	}

	virtual unsigned int size() const{
		return arraySize;
	}

	void moveRight(unsigned int steps = 1){
		unsigned int i = arraySize-1;
		for( ; i >= steps; --i)
			array[i] = array[i-steps];

		for( ; i > 0; --i)
			array[i] = 0;
		array[0] = 0;
	}

	void moveLeft(unsigned int steps = 1){
		unsigned int i = 0;
		for( ; i < (arraySize - steps); ++i)
			array[i] = array[i+steps];

		for( ; i < arraySize; ++i)
			array[i] = 0;
	}

	virtual T* begin(){
		return &array[0];
	}

	virtual const T* begin() const{
		return &array[0];
	}

	virtual T* end(){
		return &array[arraySize];
	}

	virtual const T* end() const{
		return &array[arraySize];
	}

	virtual T& front(){
		return array[0];
	}

	virtual const T& front() const{
		return array[0];
	}

	virtual T& back(){
		return array[arraySize-1];
	}

	virtual const T& back() const{
		return array[arraySize-1];
	}

	friend std::ostream& operator<<(std::ostream& o,const Array<T,arraySize>& array){
		if(array.size() == 0) return o<<"{}";
		o<<"{"<<array.front();
		for(auto  i = array.begin()+1; i != array.end(); i++)
			o<<", "<<(*i);
		o<<"}";
		return o;
	}

	virtual ~Array(){}

};


} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_ARRAY_H_ */
