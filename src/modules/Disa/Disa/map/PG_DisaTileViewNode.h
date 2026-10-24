#pragma once
#include <functional>
#include <optional>
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
	/**
	* @brief Lays a ribbon on the ground along the given tiles, to show the way a unit walks
	* @details The ribbon runs from tile center to tile center and only bends at a center, by 90 degrees. Its
	* begin, end and bends are squares of its width around the center, which lie on the tile top. A step to
	* anything but a direct neighbour, or straight back, interrupts it, the next part starts at the following
	* tile. Over a tile without ground or one that can not be walked on, it stays flat at the height it left
	* the ground at. A small step slopes over the half of the lower tile, into its center or square. A high
	* one becomes an arc between the squares of both tiles, so it starts and ends like a new path. The whole
	* ribbon floats slightly above the ground.
	*
	* The position w is 0 where the unit walks and 1 where it flies or jumps. The uv x is the length of the
	* path up to the vertex and keeps counting over interruptions, the uv y goes from 0 to 1 across the width.
	* In a bend the inner corner has the length of the center, so the length sweeps around it like a miter.
	* @param path At least two connected tiles, otherwise the model is reset
	* @param pathWidth In tiles, clamped to 1
	*/
	static void updateFloorPathGeometry(PG_DisaMapTileListModel* model, const std::vector<glm::ivec2>& path, float pathWidth, PG_ModelNode* modelNode);
	/**
	* @brief The top of a tile in tile units, corners in the order of PG_DisaMapTileModel::getTop
	* @returns Nothing where the tile has no ground that can be walked on
	*/
	using FloorTopFunction = std::function<std::optional<std::array<glm::vec3, 4>>(glm::ivec2)>;
	/**
	* @brief Like the one above, for ground that is not a PG_DisaMapTileListModel, like the map of the running game
	* @param material Gets the uv and the walk or fly flag in the position w described above
	*/
	static void updateFloorPathGeometry(const FloorTopFunction& topOf, const std::vector<glm::ivec2>& path, float pathWidth, PG_ModelNode* modelNode,
		const std::string& material = "M_DisaTilePath");
	static void updateFloorShadowGeometry(const std::array<glm::vec3, 4>& points, PG_ModelNode* modelNode);
private:
	void updateGeometry();
	void updateTranslation();
	PG_ListModelLocation m_location;
};

