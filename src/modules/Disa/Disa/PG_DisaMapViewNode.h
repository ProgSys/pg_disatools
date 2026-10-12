#pragma once

#include <Scene/PG_ActorNode.h>
#include <Resources/PG_Material.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/map/PG_DisaActorListModel.h> //the lifeline needs the whole type


class PG_FootlooseViewNode;
class PG_DisaActorsViewNode;

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
	* @returns true when the actors of the map are shown, the flag lives on its actor list
	* @see PG_DisaActorListModel::isVisible
	*/
	bool isActorsVisible() const;
	void setActorsVisible(bool value);

	/// The list the actor view shows, the one of the map unless another was swapped in
	PG_DisaActorListModel* getActorList() const;
	/**
	* @brief Shows another actor list instead of the one of the map, like a copy to play on
	* @param actors Null goes back to the list of the map
	*/
	void setActorList(PG_DisaActorListModel* actors);
	/// Without the view no actor is drawn at all, which spares a preview the work
	void setActorViewEnabled(bool enabled);
	/**
	* @returns true when geo colors should be displayed
	*/
	bool isGeoVisible() const { return m_isGeoVisible; }
	void setGeoVisible(bool value);

	PG_FootlooseViewNode* getTileView() const { return m_tilesView; }
	PG_FootlooseViewNode* getObjectsView() const { return m_objectsView; }
	PG_DisaActorsViewNode* getActorView() const { return m_actorView; }

private:
	void reload();
	/// Gives the view the list through a delay, so a big map does not load in one frame
	void setDelayedModel(PG_FootlooseViewNode* view, PG_ListModelBase* model);
	void createActorView();
	void createGeoPanels();
	void contentChanged();

	std::shared_ptr<PG_DisaMPDContainer> m_mpdContainer; //only set when the map came out of a container
	PG_Lifeline_T<PG_DisaMapModel> m_mapModel;
	EMode m_mode = Mode_View;
	bool m_HDMode = false; //if true then HD textures are used
	bool m_isActorViewEnabled = true;
	PG_Lifeline_T<PG_DisaActorListModel> m_actorList; /// swapped in instead of the list of the map
	bool m_isGeoVisible = true; //if true then geo color tiles will be visible

	PG_FootlooseViewNode* m_tilesView = nullptr;
	PG_FootlooseViewNode* m_objectsView = nullptr;
	PG_DisaActorsViewNode* m_actorView = nullptr;
	std::vector<PG_Material> m_matBuffer;
};