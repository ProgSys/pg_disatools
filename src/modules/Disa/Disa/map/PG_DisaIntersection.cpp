#include "PG_DisaIntersection.h"
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Util/PG_MathGeometry.h>

PG_DisaFaceHit::Intersection PG_DisaFaceHit::intersection(PG_DisaTileViewNode* tile, const PG_Ray& ray, bool includeSides, bool includeEdges) {
	//find the exact tile hit
	PG_DisaMapTileModel* model = tile->getLocation().getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);
	assert(model);


	using PointsFunc = std::array<glm::vec3, 4>(PG_DisaMapTileModel::*)(const glm::vec3&) const;
	std::vector<std::pair<PG_DisaFaceHit::Face, PointsFunc>> pointsFunc =
		{ { Face_Top, &PG_DisaMapTileModel::getTop} };
	if (includeSides) {
		if (model->hasSideN()) pointsFunc.emplace_back(Face_North, &PG_DisaMapTileModel::getSideN);
		if (model->hasSideE()) pointsFunc.emplace_back(Face_East, &PG_DisaMapTileModel::getSideE);
		if (model->hasSideS()) pointsFunc.emplace_back(Face_South, &PG_DisaMapTileModel::getSideS);
		if (model->hasSideW()) pointsFunc.emplace_back(Face_West, &PG_DisaMapTileModel::getSideW);
	}
	else {
		pointsFunc = { { Face_Top, &PG_DisaMapTileModel::getTop} };
	}

	PG_Ray textRay(ray.orig - model->getTranslation(), ray.dir);
	float foundDistance = std::numeric_limits<float>::max();
	PG_DisaFaceHit::Face foundFace = Face_Top;
	float height, heightRange; //used to normalize projection to 0-1

	for (auto [face, pointFunc] : pointsFunc) {
		const auto facePoints = std::invoke(pointFunc, model, glm::vec3(0.f));
		std::optional<float> distance;
		if (distance = PG_MathGeometry::intersectionTriangle(textRay.orig, textRay.dir, facePoints[0], facePoints[1], facePoints[2])) {}
		else if (distance = PG_MathGeometry::intersectionTriangle(textRay.orig, textRay.dir, facePoints[1], facePoints[2], facePoints[3])) {}

		if (distance && *distance < foundDistance) {
			foundDistance = *distance;
			foundFace = face;
			height = facePoints[0].y;
			heightRange = facePoints[2].y - height;
		}
	}

	//hit was behind or too big?
	if (foundDistance < 0.f || foundDistance > std::numeric_limits<float>::max()/2.0F) {
		return PG_DisaFaceHit::Intersection{ -1.f, nullptr };
	}

	const glm::vec3 p = textRay.getPointOnRay(foundDistance);
	glm::vec2 projection;
	bool flip = false;
	switch (foundFace){
	default: 
	case PG_DisaFaceHit::Face_Top: projection = { p.x, p.z }; break;
	case PG_DisaFaceHit::Face_East: flip = true;
	case PG_DisaFaceHit::Face_West: projection = { p.z, (p.y - height) / heightRange };  break;
	case PG_DisaFaceHit::Face_South: flip = true;
	case PG_DisaFaceHit::Face_North: projection = { p.x, (p.y - height) / heightRange }; break;
	};

	unsigned char bitChar = 0;
	if (includeEdges) {
		constexpr float edgeThreshold = 0.2f;
		if (projection.x < edgeThreshold) bitChar |= flip? PG_DisaFaceHit::Right: PG_DisaFaceHit::Left;
		if (projection.x > 1.f - edgeThreshold) bitChar |= flip ? PG_DisaFaceHit::Left: PG_DisaFaceHit::Right;
		if (projection.y < edgeThreshold) bitChar |= flip ? PG_DisaFaceHit::Top : PG_DisaFaceHit::Bottom;
		if (projection.y > 1.f - edgeThreshold) bitChar |= flip ? PG_DisaFaceHit::Bottom : PG_DisaFaceHit::Top;
	}

	return PG_DisaFaceHit::Intersection{ foundDistance, PG_EditorActorInfo{tile, PG_Variety{PG_DisaFaceHit{foundFace, static_cast<PG_DisaFaceHit::HitBits>(bitChar)}}} };
}