#include "PG_DisaMoveRange.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaCharactersModel.h>
#include <Disa/map/PG_DisaSpriteModel.h>

namespace {

/// The neighbour order of the game, with x mirrored like the editor mirrors the map
constexpr std::array<glm::ivec2, 4> g_neighbors = { glm::ivec2(0, -1), glm::ivec2(-1, 0), glm::ivec2(0, 1), glm::ivec2(1, 0) };

} //namespace

PG_DisaMoveRange::Stats PG_DisaMoveRange::statsOf(const PG_DisaSpriteModel& unit) {
	auto* characters = PG_DisaCharactersModel::get();
	if (!characters) return {};
	auto findIt = std::ranges::find(*characters, unit.getCharacterId(), &PG_DisaCharacterModel::id);
	if (findIt == characters->end()) return {};
	return { findIt->movement, findIt->jumpPower };
}

PG_DisaMoveRange::PG_DisaMoveRange(const PG_DisaMapModel& map, glm::ivec2 start, unsigned char alliance, Stats stats,
	std::span<const Occupant> occupants) : m_start(start), m_jumpPower(stats.jumpPower) {
	//like the height grid of FUN_004d2cf0: the middle of the top
	for (const PG_DisaMapTileModel* tile : *map.getTiles()) {
		const mpdfileTile& data = tile->getData();
		if (data.mobility != 0) continue;
		constexpr float heightUnitsPerTile = 12.f;
		const int chunkHeight = static_cast<int>(std::lround(-tile->getTranslation().y * heightUnitsPerTile));
		m_heights[tile->getTilePosition()] = (data.cornersTop[1] + data.cornersTop[2]) / 2 + chunkHeight;
	}

	std::unordered_set<glm::ivec2, PG_iVec2Hasher> blocked;
	for (const Occupant& occupant : occupants) {
		if (occupant.alliance == alliance) m_allies.insert(occupant.tile);
		else blocked.insert(occupant.tile);
	}

	m_distances[start] = 0;
	m_tiles.push_back(start);
	std::vector<glm::ivec2> front{ start };
	for (int step = 1; step <= stats.movement && !front.empty(); ++step) {
		std::vector<glm::ivec2> next;
		for (glm::ivec2 from : front) {
			for (glm::ivec2 offset : g_neighbors) {
				const glm::ivec2 to = from + offset;
				if (!m_heights.contains(to) || m_distances.contains(to) || blocked.contains(to)) continue;
				if (heightOf(from) - heightOf(to) > m_jumpPower) continue;

				m_distances[to] = step;
				next.push_back(to);
				m_tiles.push_back(to);
			}
		}
		front = std::move(next);
	}
}

int PG_DisaMoveRange::getDistance(glm::ivec2 tile) const {
	auto findIt = m_distances.find(tile);
	return findIt != m_distances.end() ? findIt->second : -1;
}

bool PG_DisaMoveRange::isDestination(glm::ivec2 tile) const {
	return tile != m_start && m_distances.contains(tile) && !m_allies.contains(tile);
}

glm::ivec2 PG_DisaMoveRange::nearestDestination(glm::ivec2 tile) const {
	glm::ivec2 nearest = m_start;
	int nearestSquared = std::numeric_limits<int>::max();
	int nearestDistance = std::numeric_limits<int>::max();
	for (glm::ivec2 candidate : m_tiles) {
		if (!isDestination(candidate)) continue;
		const glm::ivec2 delta = candidate - tile;
		const int squared = delta.x * delta.x + delta.y * delta.y;
		const int distance = getDistance(candidate);
		if (squared < nearestSquared || (squared == nearestSquared && distance < nearestDistance)) {
			nearest = candidate;
			nearestSquared = squared;
			nearestDistance = distance;
		}
	}
	return nearest;
}

std::vector<glm::ivec2> PG_DisaMoveRange::buildPath(glm::ivec2 target) const {
	const int distance = getDistance(target);
	if (distance < 0) return {};

	std::vector<glm::ivec2> path(static_cast<std::size_t>(distance) + 1);
	path[distance] = target;
	for (int step = distance - 1; step >= 0; --step) {
		const glm::ivec2 current = path[step + 1];
		//the first neighbour one step closer that the unit can climb up from, like FUN_004d1e50
		auto previous = std::ranges::find_if(g_neighbors, [&](glm::ivec2 offset) {
			const glm::ivec2 tile = current + offset;
			return getDistance(tile) == step && heightOf(tile) - heightOf(current) <= m_jumpPower;
			});
		if (previous == g_neighbors.end()) return {};
		path[step] = current + *previous;
	}
	return path;
}

int PG_DisaMoveRange::heightOf(glm::ivec2 tile) const {
	auto findIt = m_heights.find(tile);
	return findIt != m_heights.end() ? findIt->second : 0;
}
