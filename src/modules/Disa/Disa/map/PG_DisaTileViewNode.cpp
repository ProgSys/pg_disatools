#include "PG_DisaTileViewNode.h"
#include <Disa/PG_DisaMapModel.h>
#include <Disa/fileformats/PG_MPP.h>
#include <Disa/PG_DisaMapGeometryHolder.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/PG_DisaAssetsModel.h>
#include <Resources/PG_MaterialBinding.h>
#include <memory>

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

bool PG_DisaTileViewNode::isPresent(glm::ivec2 pos, PG_DisaMapModel* model) {
	if (!model) return false;
	return model->getTiles()->getTileLocation(pos);
}

void PG_DisaTileViewNode::updateFloorSelectionGeometry(glm::ivec2 pos, PG_DisaMapModel* model, PG_ModelNode* modelNode) {
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
	modelNode->getModel().load(PG_UNIQUE_RES("M_TileHover"), [glGeo = std::move(glGeo)]() -> std::vector<PG_Model::Surface> {
		return { { PG_Material("M_DisaTileEditSelection"), PG_Geometry(PG_UNIQUE_RES("TileHoverGeometry"), std::move(glGeo)) } };
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
	if (!modelNode || !model) return;

	if (eventTiles.empty()) {
		modelNode->getModel().reset();
		return;
	}

	PG_DisaGeoPanelGeometry geo;

	for (glm::ivec2 position : eventTiles) {
		std::array<glm::vec3, 4> top;
		auto tile = model->getTileLocation(position);
		if (tile) {
			auto* item = tile.getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);
			top = item->getTop(item->getTranslation());
		}
		else {
			top = PG_DisaMapTileModel::getTopCustom(glm::vec3(position.x, 0.f, position.y), { 0,0,0,0 }); //place at floor
		}
		buildFace<0, 1, 2, 3>(top, 3, geo); //Top
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

	modelNode->getModel().load(PG_UNIQUE_RES("M_TileEventPanels"), { { mat, PG_Geometry(PG_UNIQUE_RES("TileEventPanelsGeometry"), std::move(glGeo)) } });
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