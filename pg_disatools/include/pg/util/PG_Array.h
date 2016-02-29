/*
 * Array wrapper.
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
