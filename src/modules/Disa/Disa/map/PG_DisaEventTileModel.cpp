#include "PG_DisaEventTileModel.h"
#include <Disa/PG_DisaMapModel.h>

void PG_DisaEventTileModel::setChunk(PG_DisaMapChunkModel* model) {
	assert(model);
	if (m_chunk == model) return;
	m_chunk = model;
}

void PG_DisaEventTileModel::setTilePosition(glm::ivec2 pos) {
	if (pos == getTilePosition() || m_location < 0) return;
	auto loc = getParent<PG_ListModel<PG_DisaEventTileModel*>>()->getLocation(m_location);
	loc.setData(pos, Identity_TilePosition);
}


PG_Variety PG_DisaEventTileModel::getData(Identity identity) const {
	switch (identity)
	{
	case PG_Identity_Text: return std::to_string(getValue());
	case Identity_Value: return getValue();
	case Identity_TilePosition: return getTilePosition();
	}
	return PG_Variety();
}

bool PG_DisaEventTileModel::setData(const PG_Variety& value, Identity identity) {
	switch (identity) {
	case Identity_Value: {
		if(value.is<unsigned short>()){
			if (auto v = value.get<unsigned short>(); v != getValue()) {
				setValue(v);
				return true;
			}
		}
		else if (value.is<unsigned int>()) {
			if (auto v = value.get<unsigned int>(); v != getValue()) {
				setValue(static_cast<unsigned short>(v));
				return true;
			}
		}
	}break;

	case Identity_TilePosition: {
		if (auto v = value.get<glm::ivec2>(); v != getTilePosition()) {
			m_data.x = -v.x;
			m_data.z = v.y;
			emit tilePositionChanged();
			return true;
		}
	}break;
	}


	return false;
}

void PG_DisaEventTileListModel::addEvent(glm::ivec2 at) {
	if (at.x > 128 || at.x < -128 || at.y > 128 || at.y < -128) {
		OUT_ERROR("Event tile " << at << " is outside map boundaries!");
		return;
	}
	auto* mapModel = getParent<PG_DisaMapModel>();
	if (!mapModel || mapModel->getChunks()->empty()) return;
	
	auto* eventModel = PG_new<PG_DisaEventTileModel>("PG_DisaEventTileModel", this);
	eventModel->setData(mpdfileChunkEventTile{static_cast<char>(-at.x), static_cast<char>(at.y), 0});
	eventModel->setChunk(mapModel->getChunks()->front());
	push_back(eventModel);
}