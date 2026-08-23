#pragma once

#include <Scene/Model/PG_ListSelectionInteractableModel.h>
#include <Disa/fileformats/PG_MPD.h>
#include <Disa/map/PG_DisaMapChunkModel.h>


class PG_DisaEventTileModel : public PG_Object {
	PG_OBJECT(PG_DisaEventTileModel);
	PG_PARENT(PG_Object);

	PG_PROPERTY(int value GET getValue SET setValue CHANGED valueChanged)
signals:
	void valueChanged();
	void tilePositionChanged();
public:
	using PG_Object::PG_Object;

	enum ECustomIdentity : Identity {
		Identity_Value = EIdentity::PG_Identity_Custom,
		Identity_TilePosition,
	};

	int getLocation() const { return m_location; }
	void setLocation(int index) { m_location = index; }

	PG_DisaMapChunkModel* getChunk() const { return m_chunk.get(); }
	void setChunk(PG_DisaMapChunkModel* model);

	glm::ivec2 getTilePosition() const { return glm::ivec2(-m_data.x, m_data.z); }
	void setTilePosition(glm::ivec2 pos);

	unsigned short getValue() const { return m_data.index; }
	void setValue(unsigned short value)  { 
		if (m_data.index == value) return;
		m_data.index = value;
		emit valueChanged();
	}

	const mpdfileChunkEventTile& getData() const { return m_data; }
	void setData(const mpdfileChunkEventTile& data) { m_data = data; }
	void setData(mpdfileChunkEventTile&& data) { m_data = std::move(data); }


	PG_Variety getData(Identity identity) const;
	bool setData(const PG_Variety& value, Identity identity);
private:
	int m_location = 0;
	PG_Lifeline_T<PG_DisaMapChunkModel> m_chunk;
	mpdfileChunkEventTile m_data;
};

class PG_DisaEventTileListModel : public PG_ListModel<PG_DisaEventTileModel*> {
	PG_OBJECT(PG_DisaEventTileListModel);
	PG_PARENT(PG_ListModel<PG_DisaEventTileModel*>);
public:
	using PG_ListModel<PG_DisaEventTileModel*>::PG_ListModel;

	void addEvent(glm::ivec2 at);
};