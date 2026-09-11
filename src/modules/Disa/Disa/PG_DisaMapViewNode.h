#pragma once

#include <Scene/PG_ActorNode.h>
#include <Resources/PG_Material.h>
#include <Disa/PG_DisaMapModel.h>


class PG_FootlooseViewNode;
class PG_DisaTeleporterViewNode;

/**
* @brief Visual representation of disa map
* Needs a PG_DisaMapModel.
* @see PG_DisaMapModel
*/
class PG_DisaMapViewNode : public PG_ActorNode {
	PG_OBJECT(PG_DisaMapViewNode);
	PG_PARENT(PG_ActorNode);
signals:
	void modelChanged();
	void modeChanged();
	void hqModeChanged();
	void isActorsVisibleChanged();
	void isGeoVisibleChanged();
	void loaded();
public:
	enum EMode {
		Mode_View,
		Mode_Terrain,
		Mode_Material,
		Mode_Object,
		Mode_Event,
		Mode_Actor,
	};

	using PG_ActorNode::PG_ActorNode;
	void init() override;

	const std::shared_ptr<PG_DisaMPDContainer>& getModel() const { return m_mpdContainer; }
	/**
	* @brief Shows the map inside the container and keeps the container alive
	*/
	void setModel(const std::shared_ptr<PG_DisaMPDContainer>& model);

	PG_DisaMapModel* getMapModel() const { return m_mapModel.get(); }
	/**
	* @brief Shows the given map, which somebody else owns
	* @details The model may keep file containers alive, so whoever owns it has to outlive the view.
	*/
	void setMapModel(PG_DisaMapModel* model);

	EMode getMode() const { return m_mode; }
	void setMode(EMode mode);
	/**
	* @returns true when HD textures should be displayed
	*/
	bool isHDMode() const { return m_HDMode; }
	void setHDMode(bool state);
	/**
	* @returns true then actors (monsters) be visible
	*/
	bool isActorsVisible() const { return m_isActorsVisible; }
	void setActorsVisible(bool value);
	/**
	* @returns true when geo colors should be displayed
	*/
	bool isGeoVisible() const { return m_isGeoVisible; }
	void setGeoVisible(bool value);

	PG_FootlooseViewNode* getTileView() const { return m_tilesView; }
	PG_FootlooseViewNode* getObjectsView() const { return m_objectsView; }
	PG_FootlooseViewNode* getActorView() const { return m_actorView; }
	PG_DisaTeleporterViewNode* getTeleporterView() const { return m_teleporterView; }

private:
	void reload();
	void createActorView();
	void createGeoPanels();
	void contentChanged();

	std::shared_ptr<PG_DisaMPDContainer> m_mpdContainer; //only set when the map came out of a container
	PG_Lifeline_T<PG_DisaMapModel> m_mapModel;
	EMode m_mode = Mode_View;
	bool m_HDMode = false; //if true then HD textures are used
	bool m_isActorsVisible = true; //if true then actors (monsters) be visible
	bool m_isGeoVisible = true; //if true then geo color tiles will be visible

	PG_FootlooseViewNode* m_tilesView = nullptr;
	PG_FootlooseViewNode* m_objectsView = nullptr;
	PG_FootlooseViewNode* m_actorView = nullptr;
	PG_DisaTeleporterViewNode* m_teleporterView = nullptr;
	std::vector<PG_Material> m_matBuffer;
};