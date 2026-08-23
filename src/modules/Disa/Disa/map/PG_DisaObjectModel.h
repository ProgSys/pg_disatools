#pragma once

#include <glm/gtx/quaternion.hpp>
#include <Scene/Model/PG_ListSelectionInteractableModel.h>
#include <Disa/fileformats/PG_MPD.h>
#include <Disa/map/PG_DisaMapChunkModel.h>

class PG_DisaMapChunkModel;

class PG_DisaObjectModel : public PG_Object {
	PG_OBJECT(PG_DisaObjectModel);
	PG_PARENT(PG_Object);


	PG_PROPERTY(int location GET getLocation SET setLocation);
	
	PG_PROPERTY(glm::ivec3 objectDisaTranslation GET getObjectDisaTranslation SET setObjectDisaTranslation CHANGED objectTranslationChanged);
	PG_PROPERTY(glm::ivec3 objectDisaRotation GET getObjectDisaRotation SET setObjectDisaRotation CHANGED objectRotationChanged);
	PG_PROPERTY(glm::ivec3 objectDisaScale GET getObjectDisaScale SET setObjectDisaScale CHANGED objectScaleChanged);

	PG_PROPERTY(glm::vec3 translationUser GET getObjectTranslation SET setObjectTranslation CHANGED objectTranslationChanged);
	PG_PROPERTY(glm::quat rotationUser GET getObjectRotation SET setObjectRotation CHANGED objectRotationChanged);
	PG_PROPERTY(glm::vec3 scaleUser GET getObjectScale SET setObjectScale CHANGED objectScaleChanged);

	PG_PROPERTY(glm::ivec2 uvScroll GET getUvScroll SET setUvScroll CHANGED objectUVScrollChanged);

signals:
	void objectTranslationChanged();
	void objectRotationChanged();
	void objectScaleChanged();
	void objectUVScrollChanged();
public:
	enum ECustomIdentity : Identity {
		Identity_Translation = PG_Identity_Custom,
		Identity_Rotation,
		Identity_Scale,
		Identity_UVScroll,
	};

	using PG_Object::PG_Object;

	int getLocation() const { return m_location; }
	void setLocation(int index) { m_location = index; }

	PG_DisaMapChunkModel* getChunk() const { return m_chunk.get(); }
	void setChunk(PG_DisaMapChunkModel* model);
	glm::vec3 getChunkTranslation() const { return m_chunk->getTileTranslation(); }

	static float getBaseScale();
	static glm::ivec3 toObjectTranslation(glm::vec3 worldPos);
	glm::vec3 getObjectTranslation() const;
	void setObjectTranslation(glm::vec3 t);

	glm::quat getObjectRotation() const;
	void setObjectRotation(glm::quat r);

	glm::vec3 getObjectScale() const;
	void setObjectScale(glm::vec3 v);

	glm::ivec2 getUvScroll() const { return { m_data.textureScrollX , m_data.textureScrollY }; }
	void setUvScroll(glm::ivec2 v);

	glm::ivec3 getObjectDisaTranslation() const { return { m_data.x , m_data.y , m_data.z }; }
	void setObjectDisaTranslation(glm::ivec3 t);

	glm::ivec3 getObjectDisaRotation() const { return { m_data.xRot , m_data.yRot , m_data.zRot }; }
	void setObjectDisaRotation(glm::ivec3 t);

	glm::ivec3 getObjectDisaScale() const { return { m_data.xScale , m_data.yScale , m_data.zScale }; }
	void setObjectDisaScale(glm::ivec3 t);

	void remove();

	const mpdfileChunkObject& getData() const { return m_data; }
	void setData(const mpdfileChunkObject& data) { m_data = data; }
	void setData(mpdfileChunkObject&& data) { m_data = std::move(data); }

	PG_Variety getData(Identity identity) const;
	bool setData(const PG_Variety& value, Identity identity);
private:
	int m_location = 0;
	PG_Lifeline_T<PG_DisaMapChunkModel> m_chunk;
	mpdfileChunkObject m_data;
};

class PG_DisaObjectListModel : public PG_ListModel<PG_DisaObjectModel*> {
	PG_OBJECT(PG_DisaObjectListModel);
	PG_PARENT(PG_ListModel<PG_DisaObjectModel*>);
public:
	using PG_ListModel<PG_DisaObjectModel*>::PG_ListModel;
};