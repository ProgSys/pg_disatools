#pragma once
#include <Scene/PG_ModelNode.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <Scene/Model/PG_ListModelIdentity.h>
#include <Disa/map/PG_DisaIntersection.h>

class PG_DisaMapModel;
class PG_DisaMapTileListModel;
/**
* @brief Visual representation of disa map tile
* Needs a PG_DisaMapModel.
* @see PG_DisaMapModel
*/
class PG_DisaTileViewNode : public PG_ModelNode {
	PG_OBJECT(PG_DisaTileViewNode);
	PG_PARENT(PG_ModelNode);
	PG_PROPERTY(PG_Object* model GET getTileModel);
public:
	using PG_ModelNode::PG_ModelNode;

	PG_Object* getTileModel() const;
	PG_ListModelLocation getLocation() const { return m_location; }
	void setLocation(PG_ListModelLocation loc);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);

	glm::ivec2 getTilePosition() const { return glm::ivec2(std::floor(getTranslation().x), std::floor(getTranslation().z)); }
	float getTopHeight() const;

	static bool isPresent(glm::ivec2 pos, PG_DisaMapModel* model); /// returns true when tile is present at given position
	static void updateFloorSelectionGeometry(glm::ivec2 pos, PG_DisaMapModel* model, PG_ModelNode* modelNode, const std::string& material = "M_DisaTileEditSelection");
	static void updateFaceLineSelectionGeometry(glm::ivec2 pos, PG_DisaFaceHit::Face face, PG_DisaMapModel* model, PG_ModelNode* modelNode);
	static void updateFloorGeoGeometry(PG_DisaMapTileListModel* model, PG_ModelNode* modelNode);
	static void updateFloorEventGeometry(PG_DisaMapTileListModel* model, const std::vector<glm::ivec2>& eventTiles, PG_ModelNode* modelNode);
	/**
	* @brief Covers the tops of the given tiles in one geo panel color, a position without a tile is covered at the floor
	* @param color Index into the geo panel shader colors, 0 is red
	*/
	static void updateFloorColorGeometry(PG_DisaMapTileListModel* model, const std::vector<glm::ivec2>& tiles, char color, PG_ModelNode* modelNode);
	static void updateFloorShadowGeometry(const std::array<glm::vec3, 4>& points, PG_ModelNode* modelNode);
private:
	void updateGeometry();
	void updateTranslation();
	PG_ListModelLocation m_location;
};

