#include "PG_DisaMoveRange.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <Util/PG_iVec2Hasher.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaCharactersModel.h>
#include <Disa/map/PG_DisaActorListModel.h>
#include <Disa/map/PG_DisaSpriteModel.h>

PG_DisaMoveRange::Stats PG_DisaMoveRange::statsOf(const PG_DisaSpriteModel& unit) {
	auto* characters = PG_DisaCharactersModel::get();
	if (!characters) return {};
	auto findIt = std::ranges::find(*characters, unit.getCharacterId(), &PG_DisaCharacterModel::id);
	if (findIt == characters->end()) return {};
	return { findIt->movement, findIt->jumpPower };
}

std::vector<glm::ivec2> PG_DisaMoveRange::find(const PG_DisaMapModel& map, const PG_DisaActorListModel& actors, const PG_DisaSpriteModel& unit) {
	return find(map, actors, unit, statsOf(unit));
}

std::vector<glm::ivec2> PG_DisaMoveRange::find(const PG_DisaMapModel& map, const PG_DisaActorListModel& actors, const PG_DisaSpriteModel& unit,
	Stats stats) {
	//like the height grid of FUN_004d2cf0: the middle of the top, negative is up
	std::unordered_map<glm::ivec2, int, PG_iVec2Hasher> heights;
	for (const PG_DisaMapTileModel* tile : *map.getTiles()) {
		const mpdfileTile& data = tile->getData();
		if (data.mobility != 0) continue;
		constexpr float heightUnitsPerTile = 12.f;
		const int chunkHeight = static_cast<int>(std::lround(-tile->getTranslation().y * heightUnitsPerTile));
		heights[tile->getTilePosition()] = (data.cornersTop[1] + data.cornersTop[2]) / 2 + chunkHeight;
	}

	//allies only keep a unit from stopping on their tile, but every tile in range is highlighted anyway
	std::unordered_set<glm::ivec2, PG_iVec2Hasher> blocked;
	for (const PG_DisaActorModel* actor : actors) {
		auto* other = dynamic_cast<const PG_DisaSpriteModel*>(actor);
		if (!other || other == &unit || other->getCharacterAlliance() == unit.getCharacterAlliance()) continue;
		blocked.insert(other->getTilePosition());
	}

	const glm::ivec2 start = unit.getTilePosition();
	std::unordered_set<glm::ivec2, PG_iVec2Hasher> visited{ start };
	std::vector<glm::ivec2> front{ start };
	std::vector<glm::ivec2> out{ start };

	for (int step = 1; step <= stats.movement && !front.empty(); ++step) {
		std::vector<glm::ivec2> next;
		for (glm::ivec2 from : front) {
			auto fromHeight = heights.find(from);
			const int heightFrom = fromHeight != heights.end() ? fromHeight->second : 0;

			for (glm::ivec2 offset : { glm::ivec2(0, -1), glm::ivec2(1, 0), glm::ivec2(0, 1), glm::ivec2(-1, 0) }) {
				const glm::ivec2 to = from + offset;
				auto toHeight = heights.find(to);
				if (toHeight == heights.end() || visited.contains(to) || blocked.contains(to)) continue;
				if (heightFrom - toHeight->second > stats.jumpPower) continue;

				visited.insert(to);
				next.push_back(to);
				out.push_back(to);
			}
		}
		front = std::move(next);
	}
	return out;
}
