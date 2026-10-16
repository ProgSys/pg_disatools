#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <ranges>
#include <Util/PG_Any.h>
#include <Util/PG_DeltaTimer.h>
#include <Util/PG_Hash.h>
#include <Resources/PG_Model.h>
#include <Resources/PG_Texture.h>
#include <Scene/PG_ActorNode.h>

class PG_Scene;

/**
* @brief Draws pg_engine content into the game.
* @details The content lives as nodes in a PG_Scene of its own, so they animate and bind their
* materials like in any pg_engine scene. The game's render queue still decides when and in which pass a
* node is drawn: submit queues it there, and FUN_00426770 draws it when the queue reaches it.
*
* A node is asked for by an id. It is kept while its id is used, and deleted once the id was used in
* neither the current frame nor the last.
*/
class DisaCustomRenderManager {
	friend struct DisaRenderObjectManager;
public:
	using NodeId = PG_Hash64;

	enum class EGroup {
		World,  /// perspective, seen through the game camera
		Screen, /// 2D, in screen space
	};

	bool isEnabled() const{ return m_enabled; }
	void setEnabled(bool value) { m_enabled = value; }

	void init();
	/**
	* @brief Start of a frame. Deletes the nodes nobody uses any more, ticks the scene and runs the event loop.
	* Hooked to FUN_00424AE0, which the game calls once at the top of its loop.
	*/
	void onFrameBegin();
	/**
	* @brief Injection of the very first glViewport command
	*/
	void glViewportReplace(int x, int y, int width, int height);
	/**
	* @brief Injection of DrawArrays
	*/
	void glDrawArraysReplace(unsigned int mode, int first, int count);

	/// An id for a node that is drawn at most once per frame, from a key that names what it shows.
	static NodeId makeNodeId(std::string_view key);
	/**
	* @brief An id for a node that may be drawn several times per frame, like the same text at two places.
	* @details Every call with the same key in one frame gives another id, so each use gets a node of its
	* own. The n-th use of a key gets the same id in every frame, so the nodes are reused.
	*/
	NodeId makeFrameNodeId(std::string_view key);
	/// An id nothing else hands out, for content that owns its node, like a custom map model.
	NodeId createUniqueNodeId() { return g_uniqueIdBit | ++m_uniqueIdCounter; }

	/**
	* @brief The node of that id, created on first use and counted as used for this frame.
	* @details Asking for an id whose node is of another type replaces the node.
	*/
	template<typename T>
	T* getNode(NodeId id, EGroup group) {
		static_assert(std::is_base_of_v<PG_ActorNode, T>, "T must derive from PG_ActorNode!");
		m_usedIds.insert(id);
		PG_ActorNode*& node = m_nodes[id];
		if (auto* typed = dynamic_cast<T*>(node)) return typed;

		if (node) node->deleteLater();
		auto* created = PG_new<T>("DisaCustomNode", getGroup(group));
		node = created;
		return created;
	}

	template<typename T, typename F>
	T* getNode(NodeId id, EGroup group, F creator) {
		static_assert(std::is_base_of_v<PG_ActorNode, T>, "T must derive from PG_ActorNode!");
		m_usedIds.insert(id);
		PG_ActorNode*& node = m_nodes[id];
		if (auto* typed = dynamic_cast<T*>(node)) return typed;

		if (node) node->deleteLater();
		auto* created = PG_new<T>("DisaCustomNode", getGroup(group));
		creator(created);
		node = created;
		return created;
	}

	/**
	* @brief Queues the node in the game's render queue for this frame.
	* Original function: FUN_00425df0
	* @param depth Used for layering
	* @param depthMaskEnabled Stored inverted by submitCustomModel, false keeps the depth writes on
	*/
	void submit(float depth, PG_ActorNode* node, int blendMode, bool isPerspective, bool depthMaskEnabled, float polygonOffset, int stencilMode, unsigned char stencilRef, bool useColorTint);
	/// The node submitted at that position this frame, null when there is none
	PG_ActorNode* getFrameNode(int index) const;

	const PG_Texture& getErrorTexture() const { return m_errorTexure; }
	const PG_Texture& getLoadTexture() const { return m_loadTexture; }
	const PG_Model& getFullImageScreenModel() const { return m_fullImageScreenModel; }
	const PG_Model& getImageScreenModel() const { return m_imageScreenModel; }

	const PG_DeltaTimer& getDeltaTimer() const { return m_deltaTimer; }
	/**
	* @returns Singelton
	*/
	static DisaCustomRenderManager* get();

	static void resetDisaOpenGLStates();
private:
	/// Set on every id of createUniqueNodeId and never on one of makeNodeId, so the two can't collide
	static constexpr NodeId g_uniqueIdBit = NodeId(1) << 63;

	PG_ActorNode* getGroup(EGroup group) const { return group == EGroup::World ? m_worldGroup : m_screenGroup; }
	/// Deletes the nodes whose id was used in neither this frame nor the last
	void releaseUnusedNodes();

	bool m_isInit = false;
	bool m_enabled = true;
	glm::ivec2 m_viewportPos;
	glm::ivec2 m_viewportSize;
	PG_DeltaTimer m_deltaTimer;

	PG_Texture m_errorTexure;
	PG_Texture m_loadTexture;
	PG_Model m_fullImageScreenModel;
	PG_Model m_imageScreenModel;
	std::vector<PG_Any> m_cachedResources;

	/// Lives until the game ends, like this singleton
	PG_Scene* m_scene = nullptr;
	PG_ActorNode* m_worldGroup = nullptr;
	PG_ActorNode* m_screenGroup = nullptr;

	std::unordered_map<NodeId, PG_ActorNode*> m_nodes;
	std::unordered_set<NodeId> m_usedIds;     /// this frame
	std::unordered_set<NodeId> m_usedIdsLast; /// the frame before, its draw may still be in flight
	std::vector<PG_ActorNode*> m_frameNodes;  /// this frame's submits, the render object carries index + 1
	std::unordered_map<NodeId, int> m_frameKeyUses; /// how often a key of makeFrameNodeId was used this frame
	NodeId m_uniqueIdCounter = 0;
};
