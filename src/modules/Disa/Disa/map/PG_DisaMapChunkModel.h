#pragma once

#include <Scene/Model/PG_ListSelectionInteractableModel.h>
#include <Disa/fileformats/PG_MPD.h>

class PG_DisaMapChunkModel : public PG_Object {
	PG_OBJECT(PG_DisaMapChunkModel);
	PG_PARENT(PG_Object);
public:
	using PG_Object::PG_Object;

	int getLocation() const { return m_data.index; }
	void setLocation(int index) { m_data.index = index; }

	const glm::vec3& getTranslation() const { return getTranslationPtr(); }
	void setTranslation(const glm::vec3& offset) { getTranslationPtr() = offset; }

	glm::vec3 getTileTranslation() const;

	const mpdfileChunkHeader& getData() const { return m_data; }
	void setData(const mpdfileChunkHeader& data) { m_data = data; }
	void setData(mpdfileChunkHeader&& data) { m_data = std::move(data); }
private:
	inline glm::vec3& getTranslationPtr() { return *((glm::vec3*)&m_data.map_offset_x); }
	inline const glm::vec3& getTranslationPtr() const { return *((glm::vec3*)&m_data.map_offset_x); }

	mpdfileChunkHeader m_data;
};

class PG_DisaMapChunksListModel : public PG_ListModel<PG_DisaMapChunkModel*> {
	PG_OBJECT(PG_DisaMapChunksListModel);
	PG_PARENT(PG_ListModel<PG_DisaMapChunkModel*>);
public:
	using PG_ListModel<PG_DisaMapChunkModel*>::PG_ListModel;
};