#include "PG_DisaEditTerrainOperation.h"
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/map/PG_DisaTileEditSelectionNode.h>
#include <Scene/Editor/PG_EditorModule.h>
#include <Scene/PG_FootlooseViewNode.h>

static inline PG_DisaEditTerrainOperation* getOperation(PG_Object* parent) {
	return parent->findFirstChild<PG_DisaEditTerrainOperation>();
}

static inline PG_DisaMapModel* getModel(const PG_Scene* scene) {
	auto* mapView = scene->getRoot()->findFirstChild<PG_DisaMapViewNode>();
	if (!mapView) return nullptr;
	return mapView->getMapModel();
}


PG_DisaEditTerrainOperation* PG_DisaEditTerrainOperation::start(PG_Object* parent, const PG_Scene* scene, const PG_Ray& ray, float distance) {
	assert(scene);
	PG_DisaEditTerrainOperation* op = getOperation(parent);
	if (!op) op = PG_new<PG_DisaEditTerrainOperation>("PG_DisaEditTerrainOperation", parent);
	auto* model = getModel(scene);
	if (!model) return op;

	op->dragStartPos = ray.getPointOnRay(distance);
	op->originalData.clear();
	const glm::ivec2 originTilePosition(std::floor(op->dragStartPos.x), std::floor(op->dragStartPos.z));

	//calc offset

	if (auto location = model->getTiles()->getTileLocation(originTilePosition)) {
		auto tileModel = location.getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);
		op->originalTopOffsets = tileModel->getTopValues();
		for (float& y : op->originalTopOffsets) y -= op->dragStartPos.y;
	}
	else {
		std::ranges::fill(op->originalTopOffsets, 0.f);
	}

	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			glm::ivec2 tilePosition(originTilePosition.x + x, originTilePosition.y + y);

			if (PG_ListModelLocation loc = model->getTiles()->getTileLocation(tilePosition)) {
				op->originalData[tilePosition] = loc.getData(PG_DisaMapTileModel::Identity_Geometry);
			}
			else {
				op->originalData[tilePosition] = PG_Variety();
			}
		}
	}


	return op;
}

/**
* @param offset Look to offset
* @param targets Corners to observe. For example if you observe bottom edge (0-1) then targets should be (0,2), (1,3).
* @param values The height values of tile
* @param neighborValues The height values of tile at offset. 0 is equal to not present.
*/
static inline void setSidesVisible(glm::ivec2 offset, const std::vector<glm::ivec2>& targets, 
	PG_DisaMapTileModel* tileModel, const std::array<float, 4>& values, 
	PG_DisaMapTileModel* neighborTileModel = nullptr, const std::array<float, 4>& neighborValues = {0.f, 0.f, 0.f, 0.f}) {
	if (targets.size() == 1 && (offset.x == 0 || offset.y == 0)) {//is corner?
		/**
		*   2-----3 +z
		*   |     |
		*   |     |
		*   0-----1 -z
		*   -x   +x
		*/
		switch (targets.front().x)
		{
		case 0: {
			if (offset.x != 0)	setSidesVisible(offset, { glm::ivec2(0,1), glm::ivec2(2,3)}, tileModel, values, neighborTileModel, neighborValues);
			else				setSidesVisible(offset, { glm::ivec2(0,2), glm::ivec2(1,3)}, tileModel, values, neighborTileModel, neighborValues);
		}break;				
		case 1: {			
			if (offset.x != 0)	setSidesVisible(offset, { glm::ivec2(1,0), glm::ivec2(3,2) }, tileModel, values, neighborTileModel, neighborValues);
			else				setSidesVisible(offset, { glm::ivec2(1,3), glm::ivec2(0,2) }, tileModel, values, neighborTileModel, neighborValues);
		}break;				
		case 2: {			
			if (offset.x != 0)	setSidesVisible(offset, { glm::ivec2(2,3), glm::ivec2(0,1) }, tileModel, values, neighborTileModel, neighborValues);
			else				setSidesVisible(offset, { glm::ivec2(2,0), glm::ivec2(3,1) }, tileModel, values, neighborTileModel, neighborValues);
		}break;				
		case 3: {			
			if (offset.x != 0)	setSidesVisible(offset, { glm::ivec2(3,2), glm::ivec2(1,0) }, tileModel, values, neighborTileModel, neighborValues);
			else				setSidesVisible(offset, { glm::ivec2(3,1), glm::ivec2(2,0) }, tileModel, values, neighborTileModel, neighborValues);
		}break;
		default:
			break;
		}

		return;
	}
	if (targets.size() != 2) return; //is side?
	assert(tileModel);

	//N, E, S, W
	constexpr std::array<glm::ivec2, 4> sideIndexMapping = { glm::ivec2(0,1),  glm::ivec2(2,0), glm::ivec2(2,3),  glm::ivec2(3,1) };
	constexpr std::array<int, 4> oppositeSideIndexMapping = { 2, 3, 0, 1 };

	const char sideA = PG_DisaMapTileModel::roundHeight(values[targets.front().x]);
	const char sideB = PG_DisaMapTileModel::roundHeight(values[targets.back().x]);
	const char oppositeSideA = PG_DisaMapTileModel::roundHeight(neighborValues[targets.front().y]);
	const char oppositeSideB = PG_DisaMapTileModel::roundHeight(neighborValues[targets.back().y]);
	const bool isSideVisible = (sideA < oppositeSideA || sideB < oppositeSideB);
	const bool isOppositeSideVisible = (oppositeSideA < sideA || oppositeSideB < sideB);

	const int sideIndex = std::distance(sideIndexMapping.begin(), std::ranges::find_if(sideIndexMapping, [&targets](glm::ivec2 v) {
		return (v.x == targets.front().x && v.y == targets.back().x) || (v.y == targets.front().x && v.x == targets.back().x);
		}));

	tileModel->setSideIVisible(sideIndex, isSideVisible);
	if(neighborTileModel){
		neighborTileModel->setSideIVisible(oppositeSideIndexMapping[sideIndex], isOppositeSideVisible);
	}
}

static inline int iDot(const glm::ivec2 a, const glm::ivec2 b) {
	return a.x * b.x + a.y * b.y;
}

static inline void calcUVOffset(mpdfileTexdata& tex, glm::ivec2 projectionDir, glm::ivec2 offset, int faceHeight, bool isTop) {
	if (tex.textureIndex == 0 && (tex.width == 0 && tex.height == 0)) {
		tex.u = 0;
		tex.v = 0;
		tex.vRight = 0;

		tex.width = 32;
		tex.height = 32;
		tex.heightRight = 32;
		return;
	}

	if (isTop) {
		int u = tex.u;
		int v = tex.v;
		int vRight = tex.vRight;
		u += tex.width * (offset.x);
		v += tex.height * (offset.y);
		vRight += tex.heightRight * (offset.y);
		if (u < 0) u += 255;
		if (v < 0) v += 255;
		if (vRight < 0) vRight += 255;
		u %= 255;
		v %= 255;
		vRight %= 255;
		tex.u = static_cast<unsigned char>(u);
		tex.v = static_cast<unsigned char>(v);
		tex.vRight = static_cast<unsigned char>(vRight);
		return;
	}

	std::swap(projectionDir.x, projectionDir.y);
	if (projectionDir.y != 0) projectionDir.y *= -1;
	auto projection = iDot(projectionDir, offset);
	if (projection != 0) {
		int u = tex.u;
		u += tex.width * (projection > 0 ? -1 : 1);
		if (u < 0) u += 255;
		u %= 255;
		tex.u = static_cast<unsigned char>(u);
	}

	float unitHeight = float(tex.height) / -faceHeight;

	tex.height = static_cast<unsigned char>(std::round(unitHeight * 8.f));
	tex.heightRight = tex.height;
}

static inline std::array<mpdfileTexdata, 12> computeTextures(PG_DisaMapTileListModel* tiles, const glm::ivec2 tilePos, const std::unordered_map<glm::ivec2, PG_Variety, PG_iVec2Hasher>& originalData) {
	std::array<mpdfileTexdata, 12> out; // Base color: N, E, S, W, Top; Decal layer: N, E, S, W, Top; Shadowmap: Top, (is always zero?) 
	//In 2D
	//N == -Z
	//E == -X
	/**                   S
	*               2  |  3  | 4
	*       Z       ---2-----3---
	*       ^     W 1  |     |  5 E
	*       |          |     |
	*       |       ---0-----1---
	* X<----o       0  |  7  |  6
	*                     N
	*                     
	*/

	struct TextureFace {
		glm::ivec2 projectionDir;
		bool isTop;
	};
	constexpr static std::array<TextureFace, 12> faces{
		TextureFace{glm::ivec2( 0, -1), false},
		TextureFace{glm::ivec2(-1,  0), false},
		TextureFace{glm::ivec2( 0,  1), false},
		TextureFace{glm::ivec2( 1,  0), false},
		TextureFace{glm::ivec2( 0,  0), true},

		TextureFace{glm::ivec2( 0, -1), false},
		TextureFace{glm::ivec2(-1,  0), false},
		TextureFace{glm::ivec2( 0,  1), false},
		TextureFace{glm::ivec2( 1,  0), false},
		TextureFace{glm::ivec2( 0,  0), true},

		TextureFace{glm::ivec2(0, 0), true},
	};

	auto it = out.begin();
	for(auto& [projectionDir, isTop]: faces){

		glm::ivec2 offset = projectionDir;
		auto findIt = originalData.end();
		if(!isTop){
			std::swap(offset.x, offset.y);
			findIt = originalData.find(tilePos + offset);
			if (findIt == originalData.end() || findIt->second.isEmpty()) {
				offset *= -1;
				findIt = originalData.find(tilePos + offset);
			}
		}

		if (findIt == originalData.end() || findIt->second.isEmpty()) {
			//try find any side
			for (glm::ivec2 offset2 : {
				glm::ivec2(-1, 0), glm::ivec2(1, 0),
					glm::ivec2(0, -1), glm::ivec2(0, 1),
					glm::ivec2(-1, -1), glm::ivec2(1, -1), glm::ivec2(-1, 1), glm::ivec2(1, 1)
			}) {
				if (findIt = originalData.find(tilePos + offset2); findIt != originalData.end() && !findIt->second.isEmpty()) {
					offset = offset2;
					break;
				}
			}
		}


		if (findIt == originalData.end() || findIt->second.isEmpty()) { 
			//we found nothing
			
			constexpr int tb = 32;
			unsigned char u = tb * 7 - (tilePos.x * tb); //but why???

			unsigned char v = 128 - (tilePos.y * tb);
			if (!isTop) {
				if (iDot(projectionDir, glm::ivec2(1, 0)) != 0) {
					u = v;
				}
				v = 0;
			}
			if (projectionDir == glm::ivec2(1, 0) || projectionDir == glm::ivec2(0, -1)) {
				u = static_cast<unsigned char>(256 - u); //flip
			}

			*it = mpdfileTexdata{ u, v, v, tb,tb,tb, 0,0 };
		}
		else {
			//found a side to the left or right
			const auto& tileData = findIt->second.get<mpdfileTile>();
			
			*it = tileData.textures[std::distance(out.begin(), it)];
			calcUVOffset(*it, projectionDir, offset, tileData.cornersTop[0], isTop);
		}
		++it;
	}

	//set all missing textures indeces
	{
		unsigned char minTextureSideIndex = 255;
		for (int i = 0; i < 4; ++i) {
			if(out[i].textureIndex > 0) minTextureSideIndex = std::min(out[i].textureIndex, minTextureSideIndex);
		}
		if (minTextureSideIndex == 255) {
			minTextureSideIndex = 1;
			[&]{ //try to find something
				for (const auto& item : originalData) {
					if (!item.second) continue;
					const auto& tileData = item.second.get<mpdfileTile>();
					for (int i = 0; i < 4; ++i) {
						if (tileData.textures[i].textureIndex > 0) {
							minTextureSideIndex = tileData.textures[i].textureIndex;
							return;
						}
					}
				}

				for (auto tile : *tiles) {
					for (int i = 0; i < 4; ++i) {
						if (tile->getData().textures[i].textureIndex > 0) {
							minTextureSideIndex = tile->getData().textures[i].textureIndex;
							return;
						}
					}
				}
			}();
		}

		for (int i = 0; i < 4; ++i) {
			if (out[i].textureIndex == 0) 
				out[i].textureIndex = minTextureSideIndex;
		}
	}

	if (out[4].textureIndex == 0) {
		out[4].textureIndex = 1;
		[&] { //try to find something
			for (const auto& item : originalData) {
				if (!item.second) continue;
				const auto& tileData = item.second.get<mpdfileTile>();
				if (tileData.textures[4].textureIndex > 0) {
					out[4].textureIndex = tileData.textures[4].textureIndex;
					return;
				}
			}

			for (auto tile : *tiles) {
				if (tile->getData().textures[4].textureIndex > 0) {
					out[4].textureIndex = tile->getData().textures[4].textureIndex;
					return;
				}
			}
			}();
	}

	return out;
}

static inline void editTile(float height, const glm::ivec2 tilePos, PG_DisaMapTileListModel* tiles, 
	std::array<float, 4> offsets, const std::unordered_map<glm::ivec2, PG_Variety, PG_iVec2Hasher>& originalData,
	PG_ListModelLocation& location, PG_DisaEditTerrainOperation::EMergeMode merge, PG_DisaFaceHit faceHit = PG_DisaFaceHit()) {
	constexpr float heightStep = 1.f / 12.f;
	//height = std::max(heightStep, height);


	auto model = (PG_DisaMapTileListModel*)location.getModel();
	assert(model);

	if (!location) {
		std::array<mpdfileTexdata, 12> textures = computeTextures(tiles, tilePos, originalData);
		location = model->addNewTile(tilePos, textures);
		assert(location);
	}

	auto tileModel = location.getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);
	auto values = tileModel->getTopValues();

	std::vector<int> moveTargets;
	struct MergeTarget {
		glm::ivec2 offset;
		std::vector<glm::ivec2> targets;
	};

	/**
	*  2  |  3  | 4
	*  ---2-----3---
	*  1  |     |  5
	*     |     | 
	*  ---0-----1---
	*  0  |  7  |  6
	*/

	static std::vector<MergeTarget> possibleMergeTargets{
	/*0*/	{glm::ivec2(-1, -1), std::vector<glm::ivec2>{ glm::ivec2(0, 3)  }}, // 0
	/*1*/	{glm::ivec2(-1, 0), std::vector<glm::ivec2>{ glm::ivec2(0, 1), glm::ivec2(2, 3)  }}, //Left
	/*2*/	{glm::ivec2(-1, 1), std::vector<glm::ivec2>{ glm::ivec2(2, 1)  }}, // 2
	/*3*/	{glm::ivec2(0, 1), std::vector<glm::ivec2>{ glm::ivec2(2, 0), glm::ivec2(3, 1)  }}, //Top
	/*4*/	{glm::ivec2(1, 1), std::vector<glm::ivec2>{ glm::ivec2(3, 0) }},//3
	/*5*/	{glm::ivec2(1, 0), std::vector<glm::ivec2>{ glm::ivec2(3, 2), glm::ivec2(1, 0)}}, //Right
	/*6*/	{glm::ivec2(1, -1), std::vector<glm::ivec2>{ glm::ivec2(1, 2) } }, //1
	/*7*/	{glm::ivec2(0, -1), std::vector<glm::ivec2>{ glm::ivec2(0, 2), glm::ivec2(1, 3)  }}, //Bottom
		};
	std::vector<MergeTarget> mergeTargets;

	switch (faceHit.faceSection)
	{
	case PG_DisaFaceHit::Right: {
		moveTargets = { 1, 3 };
		mergeTargets.insert(mergeTargets.end(), std::next(possibleMergeTargets.begin(), 4), std::next(possibleMergeTargets.begin(), 6 + 1));
		mergeTargets.emplace_back(glm::ivec2(0, 1), std::vector<glm::ivec2>{ glm::ivec2(3, 1)  });
		mergeTargets.emplace_back(glm::ivec2(0, -1), std::vector<glm::ivec2>{ glm::ivec2(1, 3)  });
	}break;
	case PG_DisaFaceHit::Left: {
		moveTargets = { 2, 0 };
		mergeTargets.insert(mergeTargets.end(), std::next(possibleMergeTargets.begin(), 0), std::next(possibleMergeTargets.begin(), 2 + 1));
		mergeTargets.emplace_back(glm::ivec2(0, 1), std::vector<glm::ivec2>{ glm::ivec2(2, 0) });
		mergeTargets.emplace_back(glm::ivec2(0, -1), std::vector<glm::ivec2>{ glm::ivec2(0, 2)  });
	}break;
	case PG_DisaFaceHit::Top: {
		moveTargets = { 2, 3 };
		mergeTargets.insert(mergeTargets.end(), std::next(possibleMergeTargets.begin(), 2), std::next(possibleMergeTargets.begin(), 4 + 1));
		mergeTargets.emplace_back(glm::ivec2(-1, 0), std::vector<glm::ivec2>{ glm::ivec2(2, 3)  });
		mergeTargets.emplace_back(glm::ivec2(1, 0), std::vector<glm::ivec2>{ glm::ivec2(3, 2)});
	}break;
	case PG_DisaFaceHit::Bottom: {
		moveTargets = { 0, 1 };
		mergeTargets.insert(mergeTargets.end(), std::next(possibleMergeTargets.begin(), 6), std::next(possibleMergeTargets.begin(), 7 + 1));
		mergeTargets.push_back(possibleMergeTargets[0]);
		mergeTargets.emplace_back(glm::ivec2(1, 0), std::vector<glm::ivec2>{ glm::ivec2(1, 0)});
		mergeTargets.emplace_back(glm::ivec2(-1, 0), std::vector<glm::ivec2>{ glm::ivec2(0, 1) });
	}break;
	case PG_DisaFaceHit::Right | PG_DisaFaceHit::Top: {
		moveTargets = { 3 };
		mergeTargets.push_back(possibleMergeTargets[4]);
		mergeTargets.emplace_back( glm::ivec2(1, 0), std::vector<glm::ivec2>{ glm::ivec2(3, 2)} );
		mergeTargets.emplace_back(glm::ivec2(0, 1), std::vector<glm::ivec2>{  glm::ivec2(3, 1)  });
	}break;
	case PG_DisaFaceHit::Right | PG_DisaFaceHit::Bottom: {
		moveTargets = { 1 };
		mergeTargets.push_back(possibleMergeTargets[6]);
		mergeTargets.emplace_back(glm::ivec2(1, 0), std::vector<glm::ivec2>{ glm::ivec2(1, 0)});
		mergeTargets.emplace_back(glm::ivec2(0, -1), std::vector<glm::ivec2>{ glm::ivec2(1, 3)  });
	}break;
	case PG_DisaFaceHit::Left | PG_DisaFaceHit::Top: {
		moveTargets = { 2 };
		mergeTargets.push_back(possibleMergeTargets[2]);
		mergeTargets.emplace_back(glm::ivec2(-1, 0), std::vector<glm::ivec2>{ glm::ivec2(2, 3)  });
		mergeTargets.emplace_back(glm::ivec2(0, 1), std::vector<glm::ivec2>{ glm::ivec2(2, 0) });
	}break;
	case PG_DisaFaceHit::Left | PG_DisaFaceHit::Bottom: {
		moveTargets = { 0 };
		mergeTargets.push_back(possibleMergeTargets[0]);
		mergeTargets.emplace_back(glm::ivec2(0, -1), std::vector<glm::ivec2>{ glm::ivec2(0, 2)  });
		mergeTargets.emplace_back(glm::ivec2(-1, 0), std::vector<glm::ivec2>{ glm::ivec2(0, 1) });
	}break;
	default:
		moveTargets = { 0,1,2,3 };

		/**
		*   2-----3 +z
		*   |     |
		*   |     |
		*   0-----1 -z
		*   -x   +x
		*/
		mergeTargets = possibleMergeTargets;
		break;
	}

	//find max
	auto minMaxIt = std::ranges::minmax_element(moveTargets, {}, [&values](int i)->float { return values[i]; });
	const float minValue = values[*minMaxIt.min];
	const float maxValue = values[*minMaxIt.max] - offsets[*minMaxIt.max];
	float diff = height - maxValue;

	//clamp so lowest edge is not below heightStep
	if (minValue + diff < heightStep) {
		diff = heightStep - minValue;
	}
	if (std::abs(diff) < heightStep) return;

	for (const int i : moveTargets) {
		values[i] += diff;
	}

	tileModel->setTopValues(values);
	
	for (const auto& [offset, targets] : mergeTargets) {
		if (auto neighborTileLocation = model->getTileLocation(tilePos + offset)) {
			auto neighborTileModel = neighborTileLocation.getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);
			auto neighborValues = neighborTileModel->getTopValues();

			switch (merge)
			{
			case PG_DisaEditTerrainOperation::MergeMode_Merge:
				for (glm::ivec2 target : targets) {
					neighborValues[target.y] = std::max(heightStep, values[target.x]);
				}
				break;
			case PG_DisaEditTerrainOperation::MergeMode_Move:{
				const float neighborValuesMaxValue = neighborValues[std::ranges::max_element(targets, {}, [&neighborValues](glm::ivec2 target)->float { return neighborValues[target.y]; })->y];
				diff = height - (maxValue - neighborValuesMaxValue) - neighborValuesMaxValue;
				for (glm::ivec2 target : targets) {
					neighborValues[target.y] = std::max(heightStep, neighborValues[target.y] + diff);
				}
				
			}break;
			default:break;
			}

			//hide/show sides
			setSidesVisible(offset, targets, tileModel, values, neighborTileModel, neighborValues);
			if (!merge)  continue;

			neighborTileModel->setTopValues(neighborValues);
		}
		else {
			setSidesVisible(offset, targets, tileModel, values);
		}
	}
}


void PG_DisaEditTerrainOperation::drag(PG_Object* parent, PG_MouseMotionEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, const PG_Ray& ray) {
	PG_DisaEditTerrainOperation* op = getOperation(parent);
	if (!op) return;

	constexpr glm::vec3 upDir = glm::vec3(0.f, 1.f, 0.f);
	auto intersection = ray.intersectPlane(op->dragStartPos, glm::cross(glm::cross(ray.dir, upDir), upDir));
	if (!intersection) return;
	float newHeight = std::max(0.f, ray.getPointOnRay(*intersection).y);
	//std::cout << newHeight <<" start: " << op->dragStartPos << std::endl;
	EMergeMode mergeModeNew = EMergeMode::MergeMode_None;
	if (e->getKeyMod() & PG_KMod_LShift) {
		mergeModeNew = EMergeMode::MergeMode_Move;
	}
	else if (e->getKeyMod() & PG_KMod_LAlt) {
		mergeModeNew = EMergeMode::MergeMode_Merge;
	}

	auto* model = getModel(scene);
	if (mergeModeNew != op->mergeMode) {
		op->mergeMode = mergeModeNew;
		for (const auto& [pos, data] : op->originalData) {
			if (pos == glm::ivec2(0)) continue;
			model->getTiles()->setTileData(pos, data);
		}
	}

	std::vector<PG_EditorActorInfo> hovered;
	for (const auto& [actor, additionalData, index] : editorModule->getHovered()) {
		if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(actor.get())) {
			auto location = tile->getLocation();
			editTile(newHeight, tile->getTilePosition(), model->getTiles(), op->originalTopOffsets,  op->originalData, location, op->mergeMode, additionalData.get<PG_DisaFaceHit>());
			if (location) {
				hovered.emplace_back(actor.get(), additionalData);
			}
		}
		else if (auto map = dynamic_cast<PG_DisaMapViewNode*>(actor.get())) {
			const glm::ivec2 newTilePos = additionalData.get<PG_DisaEmptyTileHit>().tilePos;
			glm::ivec3 tileLocalPos = map->getMapModel()->getChunks()->front()->getTileTranslation();
			tileLocalPos.x = newTilePos.x - tileLocalPos.x;
			tileLocalPos.z = newTilePos.y - tileLocalPos.z;
			if (tileLocalPos.x > 0 || tileLocalPos.z < 0 || tileLocalPos.x < -254 || tileLocalPos.z > 254) continue;

			auto location = map->getMapModel()->getTiles()->getTileLocation(newTilePos);
			editTile(newHeight, newTilePos, model->getTiles(), op->originalTopOffsets, op->originalData, location, op->mergeMode);
			if (auto* tile = map->getTileView()->get(location)) {
				hovered.emplace_back(tile, PG_Variety(PG_DisaFaceHit(PG_DisaFaceHit::Face_Top, PG_DisaFaceHit::Center)));
			}
		}
	}
	editorModule->setHovered(hovered);
	editorModule->setSelection(hovered);
}

void PG_DisaEditTerrainOperation::cancel(PG_Object* parent, const PG_Scene* scene) {
	PG_DisaEditTerrainOperation* op = getOperation(parent);
	if (!op) return;
	auto* model = getModel(scene);
	if (!model) return;

	for (const auto& [pos, data] : op->originalData) {
		model->getTiles()->setTileData(pos, data);
	}

	delete op;
}

void PG_DisaEditTerrainOperation::stop(PG_Object* parent) {
	PG_DisaEditTerrainOperation* op = getOperation(parent);
	if (op) delete op;
}

void PG_DisaEditTerrainOperation::updateTile(PG_DisaMapTileListModel* tiles, glm::ivec2 pos) {
	auto loc = tiles->getTileLocation(pos);
	if (!loc) return; //nothing to do
	/**
	*   2-----3 +z
	*   |     |
	*   |     |
	*   0-----1 -z
	*   -x   +x
	*/

	struct LoopUp {
		glm::ivec2 offset;
		glm::ivec2 targetA; //(self, target)
		glm::ivec2 targetB; //(self, target)
	};

	constexpr std::array<LoopUp, 4> surroundings{
		LoopUp{glm::ivec2(1, 0), glm::ivec2(1, 0) , glm::ivec2(3, 2)},
		LoopUp{glm::ivec2(-1, 0), glm::ivec2(0, 1) , glm::ivec2(2, 3)},
		LoopUp{glm::ivec2(0, 1), glm::ivec2(2, 0) , glm::ivec2(3, 1)},
		LoopUp{glm::ivec2(0, -1), glm::ivec2(0, 2) , glm::ivec2(1, 3)}
	};

	auto tileModel = tiles->get(loc.getIndex());
	auto values = tileModel->getTopValues();

	for (const LoopUp loopUp : surroundings) {
		auto neighborLoc = tiles->getTileLocation(pos + loopUp.offset);
		if (neighborLoc) {
			auto tileModelNeighbor = tiles->get(neighborLoc.getIndex());
			auto valuesNeighbor = tileModelNeighbor->getTopValues();
			setSidesVisible(loopUp.offset, { loopUp.targetA, loopUp.targetB }, tileModel, values, nullptr, valuesNeighbor);
		}
		else {
			setSidesVisible(loopUp.offset, { loopUp.targetA, loopUp.targetB}, tileModel, values);
		}
	}
}