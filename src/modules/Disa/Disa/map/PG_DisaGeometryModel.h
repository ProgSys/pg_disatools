#pragma once

#include <string>

#include <Disa/map/PG_DisaActorModel.h>

/**
* @brief A slot item drawn as a mesh instead of a sprite.
* @details The teleporter panel is one, and the props a cutscene loads are the next. It only names
* the resource and how big it is drawn, everything about where it stands and whether it is drawn at
* all belongs to PG_DisaActorModel, so one view can follow it exactly like it follows a sprite.
*/
class PG_DisaGeometryModel : public PG_DisaActorModel {
	PG_OBJECT(PG_DisaGeometryModel);
	PG_PARENT(PG_DisaActorModel);

	PG_PROPERTY(std::string geometryResource GET getResource SET setResource CHANGED resourceChanged);
	PG_PROPERTY(float geometryScale GET getScale SET setScale CHANGED scaleChanged);
signals:
	void resourceChanged();
	void scaleChanged();
public:
	using PG_DisaActorModel::PG_DisaActorModel;

	enum ECustomIdentity : Identity {
		Identity_Resource = Identity_ActorLast_,
		Identity_Scale,
	};

	/// The model a map resource is loaded under, the teleporter panel being "R_DisaTeleporterBase".
	const std::string& getResource() const { return m_resource; }
	void setResource(const std::string& value);

	/// Whole model scale. The map's own meshes are stored twelve times the size they are drawn at.
	float getScale() const { return m_scale; }
	void setScale(float value);

	PG_DisaActorModel* clone() const override;
	PG_Variety getData(Identity identity) const override;
	bool setData(const PG_Variety& value, Identity identity) override;
private:
	std::string m_resource;
	float m_scale = 1.f / 12.f;
};
