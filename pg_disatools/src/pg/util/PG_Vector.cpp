/*
 * Simple vector classes.
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

#include <pg/util/PG_Vector.h>

namespace PG {
namespace UTIL {

template<>
void tVector2<char>::dump(std::ostream& o) const{
	o<<"("<<(int)x<<", "<<(int)y<<")";
}

template<>
void tVector2<unsigned char>::dump(std::ostream& o) const{
	o<<"("<<(int)x<<", "<<(int)y<<")";
}


template<>
void tVector3<char>::dump(std::ostream& o) const{
	o<<"("<<(int)x<<", "<<(int)y<<", "<<(int)z<<")";
}

template<>
void tVector3<unsigned char>::dump(std::ostream& o) const{
	o<<"("<<(int)x<<", "<<(int)y<<", "<<(int)z<<")";
}


template<>
void tVector4<char>::dump(std::ostream& o) const{
	o<<"("<<(int)x<<", "<<(int)y<<", "<<(int)z<<", "<<(int)w<<")";
}

template<>
void tVector4<unsigned char>::dump(std::ostream& o) const{
	o<<"("<<(int)x<<", "<<(int)y<<", "<<(int)z<<", "<<(int)w<<")";
}

} /* namespace UTIL */
} /* namespace PG */
