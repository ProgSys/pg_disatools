#include "PG_DisaMaterialEditSelectionNode.h"
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/map/PG_DisaTileViewNode.h>

void PG_DisaMaterialEditSelectionHandlerNode::setHovered(glm::ivec2 pos, bool hovered, PG_DisaFaceHit hitInfo) {
	PG_DisaMaterialEditSelectionNode* node = nullptr;

	auto findIt = m_selectionNodes.find(pos);
	if (findIt != m_selectionNodes.end()) {
		auto findFaceIt = findIt->second.find(hitInfo.face);
		if (findFaceIt != findIt->second.end()) {
			node = findFaceIt->second;
		}
	}
	
	if (!node && hovered) {
		node = PG_new<PG_DisaMaterialEditSelectionNode>("N_TileMaterialHoverModel", this);
		node->setTranslation(glm::vec3(pos.x, 0.f, pos.y));
		auto* map = getParent<PG_DisaMapViewNode>();
		assert(map);
		PG_DisaTileViewNode::updateFaceLineSelectionGeometry(pos, hitInfo.face, map->getMapModel(), node);
		m_selectionNodes[pos][hitInfo.face] = node;
	}

	if (node) {
		node->setHovered(hovered);
		node->setDisaFaceHit(hitInfo);
		if (!node->isValid()) {
			delete node;
			if (findIt != m_selectionNodes.end()) {
				findIt->second.erase(hitInfo.face);
				if (findIt->second.empty()) {
					m_selectionNodes.erase(findIt);
				}
			}
		}
	}
}

void PG_DisaMaterialEditSelectionHandlerNode::setSelected(glm::ivec2 pos, bool selected, PG_DisaFaceHit hitInfo) {
	PG_DisaMaterialEditSelectionNode* node = nullptr;

	auto findIt = m_selectionNodes.find(pos);
	if (findIt != m_selectionNodes.end()) {
		auto findFaceIt = findIt->second.find(hitInfo.face);
		if (findFaceIt != findIt->second.end()) {
			node = findFaceIt->second;
		}
	}

	if (!node && selected) {
		node = PG_new<PG_DisaMaterialEditSelectionNode>("N_TileMaterialHoverModel", this);
		node->setTranslation(glm::vec3(pos.x, 0.f, pos.y));
		auto* map = getParent<PG_DisaMapViewNode>();
		assert(map);
		PG_DisaTileViewNode::updateFaceLineSelectionGeometry(pos, hitInfo.face, map->getMapModel(), node);
		m_selectionNodes[pos][hitInfo.face] = node;
	}

	if (node) {
		node->setSelected(selected);
		node->setDisaFaceHit(hitInfo);
		if (!node->isValid()) {
			delete node;
			if (findIt != m_selectionNodes.end()) {
				findIt->second.erase(hitInfo.face);
				if (findIt->second.empty()) {
					m_selectionNodes.erase(findIt);
				}
			}
		}
	}
}

void PG_DisaMaterialEditSelectionHandlerNode::update(glm::ivec2 pos) {
	auto findIt = m_selectionNodes.find(pos);
	if (findIt != m_selectionNodes.end()) {
		auto* map = getParent<PG_DisaMapViewNode>();
		assert(map);
		for (auto& [face, item] : findIt->second) {
			PG_DisaTileViewNode::updateFaceLineSelectionGeometry(pos, face, map->getMapModel(), item);
		}
	}
}