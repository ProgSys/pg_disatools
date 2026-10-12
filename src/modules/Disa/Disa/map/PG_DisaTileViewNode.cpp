#include "PG_DisaTileViewNode.h"
#include <Disa/PG_DisaMapModel.h>
#include <Disa/fileformats/PG_MPP.h>
#include <Disa/PG_DisaMapGeometryHolder.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/PG_DisaAssetsModel.h>
#include <Resources/PG_MaterialBinding.h>
#include <Util/Math/PG_Compare.h>
#include <algorithm>
#include <memory>
#include <optional>
#include <span>

template<int SW, int SE, int NW, int NE, int Size>
inline void buildFace(const std::array<glm::vec3, Size>& corners,
	const mpdfileTexdata& tex, const mpdfileTexdata& decal, const std::optional<mpdfileTexdata>& shadow,
	const glm::vec3& normal, PG_DisaTileGeometry& geo) {

	//auto& surface = geo.surfaces[glm::ivec3(tex.textureIndex, decal.textureIndex, shadow? shadow.value().textureIndex: -1)];
	const glm::ivec3 texIds(tex.textureIndex, decal.textureIndex, shadow ? shadow.value().textureIndex : -1);
	//auto& surface = geo.surfaces[texIds];
	auto& surface = [&]()->PG_DisaTileGeometry::Surface& {
		auto findIt = std::ranges::find(geo.surfaces, texIds, &std::pair<glm::ivec3, PG_DisaTileGeometry::Surface>::first);
		if (findIt == geo.surfaces.end()) {
			geo.surfaces.push_back({ texIds, PG_DisaTileGeometry::Surface() });
			return geo.surfaces.back().second;
		}
		return findIt->second;
		}();

		const unsigned int iS = surface.vertices.size(); //index Start
		/*
		*  3-----2
		*  |     |
		*  |     |
		*  0-----1
		*/
		surface.vertices.push_back(corners[SW]);
		geo.aabb.add(surface.vertices.back());
		surface.vertices.push_back(corners[SE]);
		geo.aabb.add(surface.vertices.back());
		surface.vertices.push_back(corners[NE]);
		geo.aabb.add(surface.vertices.back());
		surface.vertices.push_back(corners[NW]);
		geo.aabb.add(surface.vertices.back());


		surface.normals.push_back(normal);
		surface.normals.push_back(normal);
		surface.normals.push_back(normal);
		surface.normals.push_back(normal);

		const auto addUVs = [](std::vector<glm::vec3>& uvs, const mpdfileTexdata& tex) {
			if (tex.textureIndex == 0) {
				uvs.insert(uvs.end(), 4, glm::vec3(0.0f, 0.0f, -1.0f));
			}
			else {
				const float decalMode = (tex.transform & 0b110000) >> 4;
				constexpr float offset = 0.3f;//this is ugly but the game does it as well :P
				/*
				*   1----0
				*   |    |
				*   2----3
				*/
				std::array<glm::vec3, 4> uv{
					glm::vec3(tex.u + tex.width - offset,	tex.vRight + tex.heightRight - offset,	decalMode),
					glm::vec3(tex.u + offset,				tex.v + tex.height - offset,			decalMode),
					glm::vec3(tex.u + offset,				tex.v + offset,							decalMode),
					glm::vec3(tex.u + tex.width - offset,	tex.vRight + offset,					decalMode)
				};
				//rotation
				std::rotate(uv.begin(), std::next(uv.begin(), tex.transform & 0b11), uv.end());
				//mirror X
				if (tex.transform & 0b100) {
					std::swap(uv[0], uv[1]);
					std::swap(uv[3], uv[2]);
				}
				//mirror Z
				if (tex.transform & 0b1000) {
					std::swap(uv[0], uv[3]);
					std::swap(uv[1], uv[2]);
				}

				uvs.insert(uvs.end(), uv.begin(), uv.end());
			}
			};

		addUVs(surface.uvs, tex);
		addUVs(surface.uvsDecal, decal);
		if (shadow)
			addUVs(surface.uvsShadowMap, shadow.value());
		else
			surface.uvsShadowMap.insert(surface.uvsShadowMap.end(), 4, glm::vec3(0.0f, 0.0f, -1.0f));

		surface.indices.insert(surface.indices.end(), {
			iS + 0, iS + 3, iS + 1,
			iS + 1, iS + 3, iS + 2,
			});
}

template<int SW, int SE, int NW, int NE, int Size, typename T>
inline void buildFace(const std::array<glm::vec3, Size>& corners, char geoColor, T& geo) {
	if (geo.surfaces.empty()) geo.surfaces.push_back(T::Surface());
	auto& surface = geo.surfaces.back();

	const unsigned int iS = surface.vertices.size(); //index Start
	/*
	*  3-----2
	*  |     |
	*  |     |
	*  0-----1
	*/
	float w = 1.0f;
	if constexpr (std::is_same_v<T, PG_DisaGeoPanelGeometry>) {
		w = float(geoColor);
	}

	surface.vertices.push_back(glm::vec4(corners[SW], w));
	surface.vertices.push_back(glm::vec4(corners[SE], w));
	surface.vertices.push_back(glm::vec4(corners[NE], w));
	surface.vertices.push_back(glm::vec4(corners[NW], w));

	if constexpr (std::is_same_v<T, PG_DisaTileTopGeometry>) {
		surface.uv.push_back(glm::vec2(0.f));
		surface.uv.push_back(glm::vec2(1.f, 0.f));
		surface.uv.push_back(glm::vec2(1.f));
		surface.uv.push_back(glm::vec2(0.f, 1.f));
	}

	surface.indices.insert(surface.indices.end(), {
		iS + 0, iS + 3, iS + 1,
		iS + 1, iS + 3, iS + 2,
		});
}

namespace {

/// One tile of a path, a flat plane at flightHeight when the path can not walk on it
struct PathTile {
	glm::vec2 center{ 0.f };
	std::optional<std::array<glm::vec3, 4>> top; /// corners in the order of PG_DisaMapTileModel::getTopData
	float flightHeight = 0.f;

	float heightAt(glm::vec2 position) const {
		if (!top) return flightHeight;
		const std::array<glm::vec3, 4>& corners = *top;
		const float u = position.x - corners[0].x;
		const float v = position.y - corners[0].z;
		//split along the same diagonal as the rendered tile, from corner 1 to 2
		if (u + v <= 1.f) return corners[0].y + (corners[1].y - corners[0].y) * u + (corners[2].y - corners[0].y) * v;
		return corners[3].y + (corners[2].y - corners[3].y) * (1.f - u) + (corners[1].y - corners[3].y) * (1.f - v);
	}
};

/// A cross section of the path ribbon
struct PathKnot {
	glm::vec2 position{ 0.f }; /// center line, x and z
	glm::vec2 side{ 0.f }; /// from the center line to the left border
	float height = 0.f;
	float heightLeft = 0.f;
	float heightRight = 0.f;
	float lengthLeft = 0.f; /// added to the path length at the left border
	float lengthRight = 0.f;

	static PathKnot on(const PathTile& tile, glm::vec2 position, glm::vec2 side) {
		return { position, side, tile.heightAt(position), tile.heightAt(position + side), tile.heightAt(position - side) };
	}
	glm::vec3 getCenter() const { return glm::vec3(position.x, height, position.y); }
};

glm::vec2 leftOf(glm::vec2 direction) { return glm::vec2(-direction.y, direction.x); }

} //namespace

bool PG_DisaTileViewNode::isPresent(glm::ivec2 pos, PG_DisaMapModel* model) {
	if (!model) return false;
	return model->getTiles()->getTileLocation(pos);
}

void PG_DisaTileViewNode::updateFloorSelectionGeometry(glm::ivec2 pos, PG_DisaMapModel* model, PG_ModelNode* modelNode, const std::string& material) {
	if (!modelNode) return;
	auto location = model->getTiles()->getTileLocation(pos);
	PG_DisaTileTopGeometry geo;
	if (location) {
		auto* tileModel = location.getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);
		const auto top = tileModel->getTop(glm::vec3(0.f));
		buildFace<0, 1, 2, 3>(top, 0, geo); //Top
	}
	else {
		buildFace<0, 1, 2, 3>(PG_DisaMapTileModel::getTopCustom(glm::vec3(0.f), { 0,0,0,0 }), 0, geo); //Top
	}

	if (geo.surfaces.empty()) {
		modelNode->getModel().clear();
		return;
	}

	PG_Mesh glGeo = PG_Mesh::basic(std::move(geo.surfaces.front().indices), std::move(geo.surfaces.front().vertices), std::move(geo.surfaces.front().uv));
	modelNode->getModel().load(PG_UNIQUE_RES("M_TileHover"), [glGeo = std::move(glGeo), material]() -> std::vector<PG_Model::Surface> {
		return { { PG_Material(material), PG_Geometry(PG_UNIQUE_RES("TileHoverGeometry"), std::move(glGeo)) } };
		});
}

void PG_DisaTileViewNode::updateFaceLineSelectionGeometry(glm::ivec2 pos, PG_DisaFaceHit::Face face, PG_DisaMapModel* model, PG_ModelNode* modelNode) {
	if (!modelNode) return;

	std::array<glm::vec3, 4> points;
	auto location = model->getTiles()->getTileLocation(pos);
	if (!location) {
		points = PG_DisaMapTileModel::getTopCustom(glm::vec3(0.f), { 0,0,0,0 });
	}else{
		const auto* tileModel = location.getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);

		switch (face)
		{
		default:
		case PG_DisaFaceHit::Face_Top: points = tileModel->getTop(glm::vec3(0.f)); break;
		case PG_DisaFaceHit::Face_West: points = tileModel->getSideW(glm::vec3(0.f)); break;
		case PG_DisaFaceHit::Face_East: points = tileModel->getSideE(glm::vec3(0.f)); break;
		case PG_DisaFaceHit::Face_South: points = tileModel->getSideS(glm::vec3(0.f)); break;
		case PG_DisaFaceHit::Face_North: points = tileModel->getSideN(glm::vec3(0.f)); break;
		}
	}

	PG_Material mat("M_DisaTileMaterialEditSelection");
	PG_Mesh glGeo(
		{
			PG_Mesh::DataWrapper("position", std::vector<glm::vec3>(points.begin(), points.end())),
			PG_Mesh::DataWrapper("indices", std::vector<unsigned int>{0, 1, 3, 2, 0}, PG_Mesh::Index),
		}
	);
	//, GL_LINE_STRIP
	modelNode->getModel().load(PG_UNIQUE_RES("M_TileMaterialHover"), { { mat, PG_Geometry(PG_UNIQUE_RES("TileMaterialHoverGeometry"), std::move(glGeo), GL::LINE_STRIP) } });
}

void PG_DisaTileViewNode::updateFloorGeoGeometry(PG_DisaMapTileListModel* model, PG_ModelNode* modelNode) {
	if (!modelNode) return;


	PG_DisaGeoPanelGeometry geo;

	for (const PG_DisaMapTileModel* item : *model) {
		if (!static_cast<bool>(item->getData().geo_mark)) continue;
		const auto top = item->getTop(item->getTranslation());
		buildFace<0, 1, 2, 3>(top, item->getData().geo_color, geo); //Top
	}

	if (geo.surfaces.empty()) {
		modelNode->clearModel();
		return;
	}

	PG_Material mat("M_MapGeoPanelTile");
	PG_Mesh glGeo(
		{
			PG_Mesh::DataWrapper("position", std::move(geo.surfaces.front().vertices)),
			PG_Mesh::DataWrapper("indices",std::move(geo.surfaces.front().indices), PG_Mesh::Index),
		}
	);

	modelNode->getModel().load(PG_UNIQUE_RES("M_TileGeoPanels"), { { mat, PG_Geometry(PG_UNIQUE_RES("TileGeoPanelsGeometry"), std::move(glGeo)) } });
}

void PG_DisaTileViewNode::updateFloorEventGeometry(PG_DisaMapTileListModel* model, const std::vector<glm::ivec2>& eventTiles, PG_ModelNode* modelNode) {
	updateFloorColorGeometry(model, eventTiles, 3, modelNode);
}

void PG_DisaTileViewNode::updateFloorColorGeometry(PG_DisaMapTileListModel* model, const std::vector<glm::ivec2>& tiles, char color, PG_ModelNode* modelNode) {
	if (!modelNode || !model) return;

	if (tiles.empty()) {
		modelNode->getModel().reset();
		return;
	}

	PG_DisaGeoPanelGeometry geo;

	for (glm::ivec2 position : tiles) {
		std::array<glm::vec3, 4> top;
		auto tile = model->getTileLocation(position);
		if (tile) {
			auto* item = tile.getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);
			top = item->getTop(item->getTranslation());
		}
		else {
			top = PG_DisaMapTileModel::getTopCustom(glm::vec3(position.x, 0.f, position.y), { 0,0,0,0 }); //place at floor
		}
		buildFace<0, 1, 2, 3>(top, color, geo); //Top
	}

	if (geo.surfaces.empty()) {
		modelNode->clearModel();
		return;
	}

	PG_Material mat("M_MapGeoPanelTile");
	PG_Mesh glGeo(
		{
			PG_Mesh::DataWrapper("position", std::move(geo.surfaces.front().vertices)),
			PG_Mesh::DataWrapper("indices",std::move(geo.surfaces.front().indices), PG_Mesh::Index),
		}
	);

	modelNode->getModel().load(PG_UNIQUE_RES("M_TileColorPanels"), { { mat, PG_Geometry(PG_UNIQUE_RES("TileColorPanelsGeometry"), std::move(glGeo)) } });
}

void PG_DisaTileViewNode::updateFloorPathGeometry(PG_DisaMapTileListModel* model, const std::vector<glm::ivec2>& path, float pathWidth, PG_ModelNode* modelNode) {
	if (!modelNode || !model) return;

	if (path.size() < 2 || PG_Math::leOrNaN(pathWidth, 0.f)) {
		modelNode->getModel().reset();
		return;
	}

	constexpr float jumpHeight = 6.f / 12.f; //6 game height units, a higher step becomes a jump arc
	constexpr float jumpClearance = 0.25f; //how far the arc passes over the higher edge
	constexpr float levelTolerance = 0.5f / 12.f;
	constexpr float groundOffset = 0.03f; //lifts the ribbon over the ground, so bent tiles do not clip it
	constexpr int surfaceDivisions = 4; //per half tile, so the ribbon follows bent tiles
	constexpr int jumpDivisions = 12;

	const float halfWidth = std::min(pathWidth, 1.f) * 0.5f;
	std::vector<glm::vec4> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<unsigned int> indices;
	float length = 0.f;

	const auto addRun = [&](std::span<const glm::ivec2> run) {
		std::vector<PathTile> tiles(run.size());
		for (std::size_t i = 0; i < run.size(); ++i) {
			tiles[i].center = glm::vec2(run[i]) + 0.5f;
			auto location = model->getTileLocation(run[i]);
			if (!location) continue;
			auto* item = location.getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);
			if (item->getMobility() == PG_DisaMapTileModel::Unlimited) tiles[i].top = item->getTop(item->getTranslation());
		}
		const auto edgeOf = [&tiles](std::size_t i) { return (tiles[i].center + tiles[i + 1].center) * 0.5f; }; //between i and i + 1

		//before the first ground the path flies at the height it reaches that ground, without any ground at the floor
		float flightHeight = 0.f;
		auto ground = std::ranges::find_if(tiles, [](const PathTile& tile) { return tile.top.has_value(); });
		if (ground != tiles.end() && ground != tiles.begin()) flightHeight = ground->heightAt(edgeOf(static_cast<std::size_t>(ground - tiles.begin()) - 1));
		for (std::size_t i = 0; i < tiles.size(); ++i) {
			if (!tiles[i].top) tiles[i].flightHeight = flightHeight;
			else if (i + 1 < tiles.size()) flightHeight = tiles[i].heightAt(edgeOf(i));
		}

		struct Step {
			glm::vec2 direction{ 0.f };
			glm::vec2 edge{ 0.f };
			float heightFrom = 0.f; /// top of the tile the step leaves, at the edge
			float heightTo = 0.f;
			bool isJump = false;
		};
		std::vector<Step> steps;
		for (std::size_t i = 0; i + 1 < tiles.size(); ++i) {
			const glm::vec2 edge = edgeOf(i);
			const float heightFrom = tiles[i].heightAt(edge);
			const float heightTo = tiles[i + 1].heightAt(edge);
			steps.push_back({ tiles[i + 1].center - tiles[i].center, edge, heightFrom, heightTo, std::abs(heightFrom - heightTo) > jumpHeight });
		}

		//the ribbon starts and ends at a square around the tile center, so do a bend and both ends of a jump
		const auto hasSquare = [&steps](std::size_t tile) {
			if (tile == 0 || tile == steps.size()) return true;
			return steps[tile - 1].isJump || steps[tile].isJump || steps[tile - 1].direction != steps[tile].direction;
		};
		/// the directions the path enters and leaves a tile
		const auto directionsAt = [&steps](std::size_t tile) {
			return std::pair(steps[tile > 0 ? tile - 1 : 0].direction, steps[std::min(tile, steps.size() - 1)].direction);
		};
		/* Where the ribbon meets the square of a tile. In a bend the inner corner belongs to the entry and the exit
		 * side, so there the ribbon takes the length of the center, like the square does. */
		const auto squareKnot = [&](std::size_t i, bool isExit) {
			const PathTile& tile = tiles[i];
			const auto [in, out] = directionsAt(i);
			const glm::vec2 direction = isExit ? out : in;
			PathKnot knot = PathKnot::on(tile, tile.center + direction * (isExit ? halfWidth : -halfWidth), leftOf(direction) * halfWidth);
			if (in != out) {
				const float toCenter = glm::distance(knot.getCenter(), glm::vec3(tile.center.x, tile.heightAt(tile.center), tile.center.y));
				(glm::dot(out, leftOf(in)) > 0.f ? knot.lengthLeft : knot.lengthRight) = isExit ? -toCenter : toCenter;
			}
			return knot;
		};

		PathKnot last;
		std::optional<float> lastColor;
		const auto pushPair = [&](const PathKnot& knot, float color) {
			vertices.emplace_back(knot.position.x + knot.side.x, knot.heightLeft + groundOffset, knot.position.y + knot.side.y, color);
			vertices.emplace_back(knot.position.x - knot.side.x, knot.heightRight + groundOffset, knot.position.y - knot.side.y, color);
			uvs.emplace_back(length + knot.lengthLeft, 0.f);
			uvs.emplace_back(length + knot.lengthRight, 1.f);
		};
		const auto addKnot = [&](const PathKnot& knot, float color) {
			if (lastColor != color) pushPair(last, color); //a color change gets its own vertices, so it stays sharp
			length += glm::distance(last.getCenter(), knot.getCenter());
			const auto i = static_cast<unsigned int>(vertices.size());
			pushPair(knot, color);
			indices.insert(indices.end(), { i - 2, i, i - 1, i - 1, i, i + 1 });
			last = knot;
			lastColor = color;
		};
		const auto addPiece = [&](const PathKnot& end, int divisions, float color, const auto& knotAt) {
			const PathKnot start = last;
			for (int division = 1; division < divisions; ++division) addKnot(knotAt(start, end, static_cast<float>(division) / divisions), color);
			addKnot(end, color);
		};
		/* A fan around the center, which lies exactly on the top whichever diagonal splits the tile. The ribbon
		 * ends at the entry side and starts again at the exit side. */
		const auto addSquare = [&](std::size_t i) {
			const PathTile& tile = tiles[i];
			const auto [in, out] = directionsAt(i);
			const float color = tile.top ? 0.f : 1.f;
			const glm::vec3 center(tile.center.x, tile.heightAt(tile.center), tile.center.y);
			const PathKnot entry = squareKnot(i, false);
			const PathKnot exit = squareKnot(i, true);
			const float lengthIn = length;
			const float lengthCenter = lengthIn + glm::distance(entry.getCenter(), center);
			length = lengthCenter + glm::distance(center, exit.getCenter());

			const auto first = static_cast<unsigned int>(vertices.size());
			vertices.emplace_back(center + glm::vec3(0.f, groundOffset, 0.f), color);
			uvs.emplace_back(lengthCenter, 0.5f);
			const bool isLeftTurn = glm::dot(out, leftOf(in)) > 0.f;
			for (glm::vec2 corner : { glm::vec2(-1.f, -1.f), glm::vec2(-1.f, 1.f), glm::vec2(1.f, 1.f), glm::vec2(1.f, -1.f) }) {
				const glm::vec2 position = tile.center + corner * halfWidth;
				vertices.emplace_back(position.x, tile.heightAt(position) + groundOffset, position.y, color);
				//-1 at the entry side, 1 at the exit side, a bend has two corners on both or neither
				const float along = glm::dot(corner, in + out) * 0.5f;
				const float acrossIn = glm::dot(corner, leftOf(in));
				const float acrossOut = glm::dot(corner, leftOf(out));
				const float across = isLeftTurn ? std::min(acrossIn, acrossOut) : std::max(acrossIn, acrossOut);
				uvs.emplace_back(along < 0.f ? glm::mix(lengthCenter, lengthIn, -along) : glm::mix(lengthCenter, length, along), 0.5f - 0.5f * across);
			}
			for (unsigned int k = 0; k < 4; ++k) indices.insert(indices.end(), { first, first + 1 + k, first + 1 + (k + 1) % 4 });
			last = exit;
			lastColor.reset();
		};
		//between a square and the edge the top is a plane, so like a slope that half needs only one quad
		const auto addOnTile = [&](const PathTile& tile, const PathKnot& end, glm::vec2 side, bool isSingleQuad) {
			addPiece(end, isSingleQuad || !tile.top ? 1 : surfaceDivisions, tile.top ? 0.f : 1.f, [&tile, side](const PathKnot& a, const PathKnot& b, float t) {
				return PathKnot::on(tile, glm::mix(a.position, b.position, t), side);
				});
		};

		addSquare(0);
		for (std::size_t i = 0; i < steps.size(); ++i) {
			const Step& step = steps[i];
			const PathTile& from = tiles[i];
			const PathTile& to = tiles[i + 1];
			const glm::vec2 side = leftOf(step.direction) * halfWidth;
			const bool hasEndSquare = hasSquare(i + 1);
			const PathKnot end = hasEndSquare ? squareKnot(i + 1, false) : PathKnot::on(to, to.center, side);

			if (step.isJump) {
				const float arcHeight = std::max(std::max(step.heightFrom, step.heightTo) + jumpClearance - (last.height + end.height) * 0.5f, jumpClearance);
				addPiece(end, jumpDivisions, 1.f, [side, arcHeight](const PathKnot& a, const PathKnot& b, float t) {
					const float arc = 4.f * arcHeight * t * (1.f - t); //parabola, arcHeight at the edge
					return PathKnot{ glm::mix(a.position, b.position, t), side, glm::mix(a.height, b.height, t) + arc,
						glm::mix(a.heightLeft, b.heightLeft, t) + arc, glm::mix(a.heightRight, b.heightRight, t) + arc };
					});
			}
			else {
				//the lower tile slopes straight from the edge of the higher one to its center or square
				const bool isFromLower = step.heightFrom + levelTolerance < step.heightTo;
				const bool isToLower = step.heightTo + levelTolerance < step.heightFrom;
				addOnTile(from, PathKnot::on(isFromLower ? to : from, step.edge, side), side, isFromLower || hasSquare(i));
				addOnTile(to, end, side, isToLower || hasEndSquare);
			}
			if (hasEndSquare) addSquare(i + 1);
		}
	};

	//a step that is no neighbour ends a run, a step straight back ends it too but the next run starts on the same tile
	std::size_t runStart = 0;
	for (std::size_t i = 1; i <= path.size(); ++i) {
		std::size_t nextStart = i;
		if (i < path.size()) {
			const glm::ivec2 step = path[i] - path[i - 1];
			const bool isReversal = i - 1 > runStart && step == path[i - 2] - path[i - 1];
			if (std::abs(step.x) + std::abs(step.y) == 1 && !isReversal) continue;
			if (isReversal) nextStart = i - 1;
		}
		if (i - runStart >= 2) addRun(std::span(path).subspan(runStart, i - runStart));
		runStart = nextStart;
	}

	if (indices.empty()) {
		modelNode->clearModel();
		return;
	}

	//a stand in until the path gets its own shader, it shows w = 0 red and w = 1 green
	PG_Material mat("M_DisaTilePath");
	PG_Mesh glGeo = PG_Mesh::basic(std::move(indices), std::move(vertices), std::move(uvs));
	modelNode->getModel().load(PG_UNIQUE_RES("M_TilePath"), { { mat, PG_Geometry(PG_UNIQUE_RES("TilePathGeometry"), std::move(glGeo)) } });
}

void PG_DisaTileViewNode::updateFloorShadowGeometry(const std::array<glm::vec3, 4>& points, PG_ModelNode* modelNode) {
	if (!modelNode) return;

	PG_DisaTileTopGeometry geo;
	buildFace<0, 1, 2, 3>(points, 0, geo); //Top

	PG_Material mat("M_MapShadowTile");
	PG_Mesh glGeo(
		{
			PG_Mesh::DataWrapper("position", std::move(geo.surfaces.front().vertices)),
			PG_Mesh::DataWrapper("uv", std::move(geo.surfaces.front().uv)),
			PG_Mesh::DataWrapper("indices",std::move(geo.surfaces.front().indices), PG_Mesh::Index),
		}
	);

	modelNode->getModel().load(PG_UNIQUE_RES("M_TileShadow"), { { mat, PG_Geometry(PG_UNIQUE_RES("TileShadowGeometry"), std::move(glGeo)) } });
}


PG_Object* PG_DisaTileViewNode::getTileModel() const {
	if (!m_location) return nullptr;
	return m_location.getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);
}

void PG_DisaTileViewNode::setLocation(PG_ListModelLocation loc) {
	m_location = loc;
	if (!m_location) return;
	updateGeometry();
	updateTranslation();
}

void PG_DisaTileViewNode::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.contains(PG_Identity_Item) || m_location != location;

	if (hasItem || identities.contains(PG_Identity_Location)) {
		m_location = location;
	}

	if (hasItem || contains<PG_DisaMapTileModel::Identity_Geometry>(identities)) {
		updateGeometry();
	}
	if (hasItem || contains<PG_DisaMapTileModel::Identity_Translation>(identities)) {
		updateTranslation();
	}
}

float PG_DisaTileViewNode::getTopHeight() const {
	if (!m_location) return 0.f;
	return m_location.getDataT<float>(PG_DisaMapTileModel::Identity_TopHeight);
}

void PG_DisaTileViewNode::updateGeometry() {
	if (!m_location) return;
	auto modelRoot = m_location.getModel()->findParent<PG_DisaMapModel>(3);
	auto parent = getParent()->getParent<PG_DisaMapViewNode>();
	if (!modelRoot || !parent) return;
	auto* tileModel = m_location.getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);
	const auto& tileData = tileModel->getData();

	PG_DisaTileGeometry geo;

	const glm::vec3 offset(0.f);

	if (const int i = PG_DisaMapTileModel::getTopTextureIndex(tileData); i >= 0) {
		const auto top = PG_DisaMapTileModel::getTopData(tileData, offset);
		buildFace<0, 1, 2, 3>(top, tileData.textures[4], tileData.textures[9], tileData.textures[10], glm::vec3(0.0f, 1.0f, 0.0f), geo); //Top
	}

	//other then top texture index must be bigger 0, for some reason...
	if (const int i = PG_DisaMapTileModel::getSideNTextureIndex(tileData); i > 0)
		buildFace<0, 1, 2, 3>(PG_DisaMapTileModel::getSideNData(tileData, offset), tileData.textures[0], tileData.textures[5], {}, glm::vec3(0.0f, 0.0f, -1.0f), geo); //Z-
	if (const int i = PG_DisaMapTileModel::getSideWTextureIndex(tileData); i > 0)
		buildFace<0, 1, 2, 3>(PG_DisaMapTileModel::getSideWData(tileData, offset), tileData.textures[3], tileData.textures[8], {}, glm::vec3(1.0f, 0.0f, 0.0f), geo); //X+
	if (const int i = PG_DisaMapTileModel::getSideSTextureIndex(tileData); i > 0)
		buildFace<0, 1, 2, 3>(PG_DisaMapTileModel::getSideSData(tileData, offset), tileData.textures[2], tileData.textures[7], {}, glm::vec3(0.0f, 0.0f, 1.0f), geo); //Z+
	if (const int i = PG_DisaMapTileModel::getSideETextureIndex(tileData); i > 0)
		buildFace<0, 1, 2, 3>(PG_DisaMapTileModel::getSideEData(tileData, offset), tileData.textures[1], tileData.textures[6], {}, glm::vec3(0.0f, 0.0f, -1.0f), geo); //X-

	glm::vec2 pos((int)tileModel->getData().x, (int)tileModel->getData().z);

	if (geo.surfaces.size() > 90000) return;

	const bool isHdMode = modelRoot->getAssets()->isHDViewEnabled();
	modelRoot->getAssets()->getTextures()->getItems();
	std::vector<PG_Texture> textures;
	if (isHdMode) {
		for (const auto& texAsset : modelRoot->getAssets()->getTextures()->getItems()) {
			textures.push_back(texAsset.getHDTexture());
		}
	}
	else {
		for (const auto& texAsset : modelRoot->getAssets()->getTextures()->getItems()) {
			textures.push_back(texAsset.getSDTexture());
		}
	}

	PG_Material tileMat(isHdMode ? "M_MapTileHD" : "M_MapTile");

	std::function<std::vector<PG_Model::Surface>()> mapModelLoadFunc =
		[geoSurfaces = std::move(geo.surfaces), textures = std::move(textures), tileMat]() mutable -> std::vector<PG_Model::Surface>  {
		//mpp data
		std::vector<PG_Model::Surface> surfaces;
		for (auto& [matIds, surface] : geoSurfaces) {
			if (surface.indices.empty()) continue;

			PG_Mesh geometry(
				{
					PG_Mesh::DataWrapper("position", std::move(surface.vertices)),
					PG_Mesh::DataWrapper("normal", std::move(surface.normals)),
					PG_Mesh::DataWrapper("uv", std::move(surface.uvs)),
					PG_Mesh::DataWrapper("uvDecal", std::move(surface.uvsDecal)),
					PG_Mesh::DataWrapper("uvShadowMap", std::move(surface.uvsShadowMap)),
					PG_Mesh::DataWrapper("indices",std::move(surface.indices), PG_Mesh::Index),
				}
				);



			PG_Material mat = tileMat;
			if (!textures.empty()) {
				mat.makeInstance();
				const auto addTexture = [&mat, &textures](int id, const std::string& name) {
					if (id <= 0) {
						mat.setBinding(name, PG_Texture("transparent"));
						return;
					}
					id -= 1;
					if (id < textures.size() && textures[id].isValid()) {
						mat.setBinding(name, textures[id]);
					}
					};

				addTexture(matIds.x, "textureSlot0");
				addTexture(matIds.y, "textureSlot1");
				addTexture(matIds.z, "textureSlot2");
			}

			if (!mat.isSet()) mat.load("M_MapDefaultTile");

			surfaces.emplace_back(
				std::move(mat),
				std::move(PG_Geometry(PG_UNIQUE_RES("G_DisaMapTile"), std::move(geometry)))
			);
		}
		return surfaces;
		};

	getModel().load(PG_UNIQUE_RES("M_DisaTile"), std::move(mapModelLoadFunc));

	auto gAABB = geo.aabb;
	if (!gAABB.isValid() && gAABB.max.y == gAABB.min.y) {
		gAABB.min.y -= 0.2;
	}
	if (!gAABB.isValid()) {
		OUT_WARNING("Tile at (" << pos.x << ", " << pos.y << ") has invalid aabb!");
		getVolumeCollision().reset();
		getVolumeVisibility().reset();
		return;
	}
	setVolumeCollision(PG_Volume::fromAABB(gAABB));
	getVolumeCollision().transform(getModelMatrix());
	setVolumeVisibility(PG_Volume::fromAABB(gAABB));
	getVolumeVisibility().transform(getModelMatrix());

}

void PG_DisaTileViewNode::updateTranslation() {
	if (!m_location) return;

	setTranslation(m_location.getDataT<glm::vec3>(PG_DisaMapTileModel::Identity_Translation));
}
