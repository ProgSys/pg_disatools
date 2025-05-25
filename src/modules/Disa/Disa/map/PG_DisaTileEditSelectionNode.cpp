#include "PG_DisaTileEditSelectionNode.h"
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/map/PG_DisaTileViewNode.h>

void PG_DisaMapSelectionHandlerNode::setHovered(glm::ivec2 pos, bool hovered, PG_DisaFaceHit hitInfo) {
	PG_DisaTileEditSelectionNode* node = nullptr;

	auto findIt = m_selectionNodes.find(pos);
	if (findIt != m_selectionNodes.end()) {
		node = findIt->second;
	}
	else if (hovered) {
		node = PG_new<PG_DisaTileEditSelectionNode>("N_TileHoverModel", this);
		node->setTranslation(glm::vec3(pos.x, 0.f, pos.y));
		auto* map = getParent<PG_DisaMapViewNode>();
		assert(map);
		PG_DisaTileViewNode::updateFloorSelectionGeometry(pos, map->getMapModel(), node);
		m_selectionNodes[pos] = node;
	}

	if (node) {
		node->setDisaFaceHit(hovered ? hitInfo : PG_DisaFaceHit(PG_DisaFaceHit::Face_Top, PG_DisaFaceHit::None));
		if (!node->isValid()) {
			delete findIt->second;
			if (findIt != m_selectionNodes.end()) {
				m_selectionNodes.erase(findIt);
			}
		}
	}
}

void PG_DisaMapSelectionHandlerNode::setSelected(glm::ivec2 pos, bool selected) {
	PG_DisaTileEditSelectionNode* node = nullptr;

	auto findIt = m_selectionNodes.find(pos);
	if (findIt != m_selectionNodes.end()) {
		node = findIt->second;
	}
	else if (selected) {
		node = PG_new<PG_DisaTileEditSelectionNode>("N_TileHoverModel", this);
		node->setTranslation(glm::vec3(pos.x, 0.f, pos.y));
		auto* map = getParent<PG_DisaMapViewNode>();
		assert(map);
		PG_DisaTileViewNode::updateFloorSelectionGeometry(pos, map->getMapModel(), node);
		m_selectionNodes[pos] = node;
	}

	if (node) {
		node->setSelected(selected);
		if (!node->isValid()) {
			delete node;
			if (findIt != m_selectionNodes.end()) {
				m_selectionNodes.erase(findIt);
			}
		}
	}
}

void PG_DisaMapSelectionHandlerNode::update(glm::ivec2 pos) {
	auto findIt = m_selectionNodes.find(pos);
	if (findIt != m_selectionNodes.end()) {
		auto* map = getParent<PG_DisaMapViewNode>();
		assert(map);
		PG_DisaTileViewNode::updateFloorSelectionGeometry(pos, map->getMapModel(), findIt->second);
	}
}

void PG_DisaMapSelectionHandlerNode::cleanUp() {
	auto* map = getParent<PG_DisaMapViewNode>()->getMapModel();
	assert(map);
	for (auto it = m_selectionNodes.begin(); it != m_selectionNodes.end(); ) {
		if (PG_DisaTileViewNode::isPresent(it->first, map)) {
			++it;
			continue;
		}

		delete it->second;
		it = m_selectionNodes.erase(it);
	}
}