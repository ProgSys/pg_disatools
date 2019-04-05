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
#ifndef INCLUDE_PG_UTIL_PG_FILE_H_
#define INCLUDE_PG_UTIL_PG_FILE_H_

#include <string>
#include <ostream>
#include <Util/PG_ApiUtil.h>

namespace PG {
namespace UTIL {

/*!
 * @brief A simple file wrapper class, containing many useful operations.
 */
class File {
	std::string m_path;
public:
	EXPORT File(const std::string& path = "");

	/*!
	 * @brief Get the full path to file.
	 */
	EXPORT std::string const& getPath() const;
	/*!
	 * @brief Get the file extension.
	 */
	EXPORT std::string getFileExtension() const;
	/*!
	 * @brief Get the name of the file with the file file extension.
	 */
	EXPORT std::string getFile() const;
	/*!
	 * @brief Get the name of the file without the file file extension.
	 */
	EXPORT std::string getName() const;

	EXPORT File toUpper() const;
	EXPORT File toLower() const;

	EXPORT void operator=(const std::string& path);
	EXPORT void operator=(const File& file);

	EXPORT bool operator==(const std::string& path) const;
	EXPORT bool operator==(const File& file) const;

	EXPORT bool operator<(const File& file) const;

	EXPORT void clear();
	EXPORT bool isEmpty() const;

	EXPORT void set(const std::string& path);

	EXPORT unsigned int size() const;
	EXPORT bool exists() const;
	EXPORT bool create(const std::string& text = "") const;
	EXPORT bool rename(const std::string& name) const;
	EXPORT bool remove() const;


	friend std::ostream& operator<<(std::ostream& o,const File& file){
		return o<<file.getPath();
	}

	EXPORT virtual ~File();
};

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_FILE_H_ */
