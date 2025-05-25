#pragma once

#include <Scene/Editor/PG_EditorActorInfo.h>

class PG_DisaTileViewNode;

struct PG_DisaFaceHit {
	/**
	* @brief The 5 faces of a tile
	* (dont question why North is at the bottom)
	*       S
	*    2-----3
	* E  |  T  |  W
	* z  |     |
	* ^  0-----1
	* |     N
	*  -> x
	*/
	enum Face : unsigned char {
		Face_Top = 0, //+y
		Face_West, // +x
		Face_East, // -x
		Face_South, // +z
		Face_North//-z
	};

	enum HitBits : unsigned char {
		Center = 0,
		Right = 0b0000001, // +x
		Left = 0b0000010, // -x
		Top = 0b0000100, // +z
		Bottom = 0b0001000, //-z
		TopLeft = Top | Left,
		TopRight = Top | Right,
		BottomLeft = Bottom | Left,
		BottomRight = Bottom | Right,
		None = 0b1000000
	};

	Face face;
	HitBits faceSection;

	bool operator==(PG_DisaFaceHit other) const { return face == other.face && faceSection == other.faceSection; }
	bool operator!=(PG_DisaFaceHit other) const { return face != other.face || faceSection != other.faceSection; }
	bool operator<(PG_DisaFaceHit other) const { return face < other.face || (face == other.face && faceSection < other.faceSection); }
	bool operator>(PG_DisaFaceHit other) const { return face > other.face || (face == other.face && faceSection > other.faceSection); }

	using Intersection = std::pair<PG_RayIntersection, PG_EditorActorInfo>;
	static Intersection intersection(PG_DisaTileViewNode* tile, const PG_Ray& ray, bool includeSides = false, bool includeEdges = true);
};

struct PG_DisaEmptyTileHit {
	glm::ivec2 tilePos;

	bool operator==(PG_DisaEmptyTileHit other) const { return tilePos == other.tilePos; }
	bool operator!=(PG_DisaEmptyTileHit other) const { return tilePos != other.tilePos; }
	bool operator<(PG_DisaEmptyTileHit other) const { return tilePos.x < other.tilePos.x || (tilePos.x == other.tilePos.x && tilePos.y < other.tilePos.y); }
	bool operator>(PG_DisaEmptyTileHit other) const { return tilePos.x > other.tilePos.x || (tilePos.x == other.tilePos.x && tilePos.y > other.tilePos.y); }
};
