/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2024  ProgSys
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
#pragma once

#include <glm/glm.hpp>

struct IVec2Hash {
	std::size_t operator () (const glm::ivec2& p) const {
		auto h1 = std::hash<int>{}(p.x);
		auto h2 = std::hash<int>{}(p.y);

		// Mainly for demonstration purposes, i.e. works but is overly simple
		// In the real world, use sth. like boost.hash_combine
		return h1 ^ h2;
	}
};