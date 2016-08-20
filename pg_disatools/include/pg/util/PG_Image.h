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
#ifndef INCLUDE_PG_UTIL_PG_IMAGE_H_
#define INCLUDE_PG_UTIL_PG_IMAGE_H_

#include <vector>
#include <cstring>
#include <pg/util/PG_Vector.h>
#include <pg/util/PG_Exception.h>

namespace PG {
namespace UTIL {

/*!
 * @brief A simple image class.
 */
template<typename T>
class Image {
	unsigned int m_width;
	unsigned int m_height;
	std::vector<T> m_pixels;
public:
	Image(){
		m_width = 0;
		m_height = 0;
	}

	Image(unsigned int width, unsigned int height){
		resize(width, height);
	}

	Image(const uvec2& size){
		resize(size);
	}

	Image(const Image<T>& image){
		m_width = image.getWidth();
		m_height = image.getHeight();

		m_pixels.resize(m_width*m_height);
		memcpy(&m_pixels[0], &image[0], m_pixels.size()*sizeof(T) );
	}

	Image(const std::vector<T>& image, unsigned int width, unsigned int height){
		resize(width, height);

		memcpy(&m_pixels[0], &image[0], image.size()*sizeof(T) );
	}

	unsigned int getWidth() const{
		return m_width;
	}
	unsigned int getHeight() const{
		return m_height;
	}

	unsigned int size() const{
		return m_pixels.size();
	}

	void set(unsigned int x,unsigned int y, const T& pixel){
		m_pixels[y*m_width+x] = pixel;
	}

	void set(const uvec2& vec, const T& pixel){
		m_pixels[vec.y*m_width+vec.x] = pixel;
	}

	T& get(unsigned int x,unsigned int y){
		return m_pixels[y*m_width+x];
	}

	T& get(const uvec2& vec){
		return m_pixels[vec.y*m_width+vec.x];
	}

	T const& get(unsigned int x,unsigned int y) const{
		return m_pixels[y*m_width+x];
	}

	T const& get(const uvec2& vec) const{
		return m_pixels[vec.y*m_width+vec.x];
	}

	T& getRow(unsigned int y){
		return m_pixels[y*m_width];
	}

	T const& getRow(unsigned int y) const{
		return m_pixels[y*m_width];
	}


	void getWindow(const PG::UTIL::uvec2& start, const PG::UTIL::uvec2& size, Image<T>& windowOut) const{
		assert_Test("Window is out of bound!", (start.x >= m_width) || (start.y >= m_height) || ( (start.x+size.x) > m_width) || ((start.y+size.y)> m_height));

		windowOut.resize(size);
		for(unsigned int y = 0; y < size.y; ++y)
			memcpy(&windowOut.getRow(y), &get(start+uvec2(0,y)), size.x*sizeof(T) );

	}

	void getWindow(const PG::UTIL::uvec2& start, const PG::UTIL::uvec2& size, std::vector<T>& windowOut) const{
		assert_Test("Window is out of bound!", (start.x >= m_width) || (start.y >= m_height) || ( (start.x+size.x) > m_width) || ((start.y+size.y)> m_height));

		windowOut.resize(size.x*size.y);
		for(unsigned int y = 0; y < size.y; ++y)
			memcpy(&windowOut[y*size.x], &get(start+uvec2(0,y)), size.x*sizeof(T) );

	}

	void setWindow(const PG::UTIL::uvec2& start, const Image<T>& windowIn){
		assert_Test("Window is out of bound!", (start.x >= m_width) || (start.y >= m_height) || ( (start.x+windowIn.getWidth()) > m_width) || ((start.y+windowIn.getHeight())> m_height));

		for(unsigned int y = 0; y < windowIn.getHeight(); ++y){
			memcpy(&get(start+uvec2(0,y)), &windowIn.getRow(y) , windowIn.getWidth()*sizeof(T) );
		}
	}

	void setWindow(const PG::UTIL::uvec2& start, const PG::UTIL::uvec2& size, const std::vector<T>& windowIn){
		assert_Test("Window has wrong size!", windowIn.size() < (size.x*size.y));
		assert_Test("Window is out of bound!", (start.x >= m_width) || (start.y >= m_height) || ( (start.x+size.x) > m_width) || ((start.y+size.y)> m_height));

		for(unsigned int y = 0; y < size.y; ++y)
			memcpy(&get(start+uvec2(0,y)), &windowIn[y*size.x] , size.x*sizeof(T) );
	}

	void setWindow(const PG::UTIL::uvec2& start, const PG::UTIL::uvec2& size, const unsigned char* bits){
		//assert_Test("Window has wrong size!", windowIn.size() < (size.x*size.y));
		assert_Test("Window is out of bound!", (start.x >= m_width) || (start.y >= m_height) || ( (start.x+size.x) > m_width) || ((start.y+size.y)> m_height));

		for(unsigned int y = 0; y < size.y; ++y)
			memcpy(&get(start+uvec2(0,y)), bits+(y*size.x*sizeof(T)) , size.x*sizeof(T) );
	}

	T& operator[](unsigned int index){
		return m_pixels[index];
	}

	T const& operator[](unsigned int index) const{
		return m_pixels[index];
	}

	void resize(unsigned int width, unsigned int height){
		m_width = width;
		m_height = height;

		m_pixels.resize(m_width*m_height);
	}

	void resize(const uvec2& size){
		m_width = size.x;
		m_height = size.y;

		m_pixels.resize(m_width*m_height);
	}

	// for iterators
	T* begin(){
		return &m_pixels[0];
	}

	const T* begin() const{
		return &m_pixels[0];
	}

	T* end(){
		return &m_pixels[0] + m_pixels.size();
	}

	const T* end() const{
		return &m_pixels[0] + m_pixels.size();
	}

	T& front(){
		return m_pixels.front();
	}

	const T& front() const{
		return m_pixels.front();
	}

	T& back(){
		return m_pixels.back();
	}

	const T& back() const{
		return m_pixels.back();
	}

};

typedef PG::UTIL::Image<PG::UTIL::rgb> RGBImage;
typedef PG::UTIL::Image<PG::UTIL::rgba> RGBAImage;
typedef PG::UTIL::Image<unsigned char> IDImage;
typedef PG::UTIL::Image<unsigned int> IDuImage;

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_IMAGE_H_ */
