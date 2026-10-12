#pragma once

#include <unordered_map>
#include <utility>
#include <vector>

#include <Base/PG_Lifeline.h>
#include <Disa/map/PG_DisaActorModel.h>

using PG_DisaActorLifeline = PG_Lifeline_T<PG_DisaActorModel>;
class PG_DisaActorScriptViewModel;
/**
* @brief Everything standing on the map, addressed the way a script addresses it.
* @details The rows are an ordinary list in load order. The slot numbers are a map over the top of
* them, because a removed row moves every row behind it up and a slot number must not move with it.
*
* The game splits the range: 0 to 63 are the units the map file placed, 64 to 95 belong to a script
* and are filled while a cutscene runs, and a few numbers above 200 are not items at all but the
* origin a coordinate counts from. See reverse/Doku.md for the whole table.
*/
class PG_DisaActorListModel : public PG_ListModel<PG_DisaActorModel*> {
	PG_OBJECT(PG_DisaActorListModel);
	PG_PARENT(PG_ListModel<PG_DisaActorModel*>);
signals:
	void visibleChanged(bool value);
public:
	using PG_ListModel<PG_DisaActorModel*>::PG_ListModel;

	/// Whether the view shows the actors at all, the slots keep working either way
	bool isVisible() const { return m_visible; }
	void setVisible(bool value);

	static constexpr int firstMapSlot = 0;
	static constexpr int lastMapSlot = 63;
	static constexpr int firstScriptSlot = 64;
	static constexpr int lastScriptSlot = 95;
	/// Not an item, the teleporter panel, which coordinates are counted from. @see getSlotOrigin
	static constexpr int slotTeleporter = 203;

	/**
	* @brief Where a coordinate with no parent counts from, in tile units.
	* @details The map runs along the negative x axis, so a coordinate the game counts the other way
	* is mirrored on the way in. Mirroring a tile index lands one tile short, which is what the whole
	* tile on x puts back. The z axis is not mirrored and needs nothing.
	* @see PG_DisaContext::toTileUnits
	*/
	static constexpr glm::vec3 mapOrigin = glm::vec3(1.f, 0.f, 0.f);

	static bool isMapSlot(int slot) { return slot >= firstMapSlot && slot <= lastMapSlot; }
	static bool isScriptSlot(int slot) { return slot >= firstScriptSlot && slot <= lastScriptSlot; }
	static bool isItemSlot(int slot) { return isMapSlot(slot) || isScriptSlot(slot); }

	/// The item of that slot, null when the slot is empty.
	PG_DisaActorModel* getSlotItem(int slot) const;
	/// The row of that slot, an invalid location when the slot is empty.
	PG_ListModelLocation getSlot(int slot) const;
	/// The slot an item sits in, -1 when it sits in none.
	int getSlotOf(const PG_DisaActorModel* item) const;

	bool isSlotFree(int slot) const { return getSlotItem(slot) == nullptr; }
	/**
	* @brief Puts an item into a slot, taking it over.
	* @details Whatever the slot held is removed with it, so a slot never holds two. The item is
	* added to the rows when it is not one of them yet.
	*/
	void setSlot(int slot, PG_DisaActorModel* item);
	/// The row of that slot, with a sprite made for it when the slot was free.
	PG_ListModelLocation takeActorSlot(int slot);
	/// Removes the item of that slot, does nothing when the slot is empty.
	void clearSlot(int slot);
	/// Empties every slot a script filled, what the end of a cutscene does.
	void clearScriptSlots();

	/**
	* @brief Where a coordinate parented to that slot counts from, in tile units.
	* @details An item slot counts from the item, height included. A slot that is not an item is a
	* named origin, the teleporter panel being one, and those carry no height: the game adds only
	* x and z for them. Anything else counts from the map origin.
	*/
	glm::vec3 getSlotOrigin(int slot) const;

	/**
	* @brief The smallest slot of the map range that holds nothing.
	* @details Where the game would put a new unit: its own allocator takes the first free entry of
	* the actor array, and that array is the slot table.
	* @returns -1 when all 64 are taken, which is as many as the file can hold.
	*/
	int findFreeMapSlot() const;

	/// Hands the rows the map file placed their slots, in file order. @see isMapSlot
	void resetMapSlots();

	/**
	* @brief Starts keeping slot 203 on the teleporter panel.
	* @details Called once by the map model, after every one of its lists exists. The rule belongs
	* to the slot rather than to what stands in it, so a sprite dropped into 203 lands on the panel
	* the same way the mesh does.
	*/
	void setupTeleporter();

	/**
	* @brief A view of this list for one script chain, which owns slots 64 to 95.
	* @details The caller owns what comes back, and every actor the chain spawns goes when it does.
	*/
	PG_DisaActorScriptViewModel* createScriptViewModel();

private:
	/// Puts whatever is in slot 203 onto the panel, next time round the event loop.
	void updateTeleporter();
	void doUpdateTeleporter();

	/* A lifeline rather than a pointer: an item removed anywhere else leaves the slot empty instead
	 * of leaving it pointing at something that is gone. */
	std::unordered_map<int, PG_DisaActorLifeline> m_slots;
	bool m_visible = true;
};


using SlotActorPair = std::pair<int, PG_DisaActorLifeline>;
/**
* @brief A script chain's view of PG_DisaActorListModel, holding slots 64 to 95.
* @details The game gives every script its own table of these and clears it when the script
* returns, so a script cannot see what the script that started it spawned. The retail scripts are
* not written that way: 54 of the 398 that touch these slots address one they never spawned into,
* which in the real game finds nothing. One view per chain rather than per script makes those do
* what they plainly meant.
*
* The actors themselves are rows of the list this views, next to the units the map placed, so the
* one view node draws the lot and only the numbering is private to the chain. The actors live
* exactly as long as this does: dropping it takes them out of the list.
*
* Every slot outside 64 to 95 is passed straight through to the list, so a caller that holds one of
* these never needs to reach past it.
*/
class PG_DisaActorScriptViewModel : public PG_ListModel<SlotActorPair> {
	PG_OBJECT(PG_DisaActorScriptViewModel);
	PG_PARENT(PG_ListModel<SlotActorPair>);
	friend PG_DisaActorListModel;
public:
	using PG_ListModel<SlotActorPair>::PG_ListModel;
	~PG_DisaActorScriptViewModel() override;

	/// The list the actors really live in, null once that list is gone.
	PG_DisaActorListModel* getOriginModel() const { return m_originModel.get(); }

	/// The item of that slot, null when the slot is empty.
	PG_DisaActorModel* getSlotItem(int slot) const;
	/// The row of that slot **in the list**, an invalid location when the slot is empty.
	PG_ListModelLocation getSlot(int slot) const;
	/// The slot an item sits in, -1 when it sits in none.
	int getSlotOf(const PG_DisaActorModel* item) const;

	bool isSlotFree(int slot) const { return getSlotItem(slot) == nullptr; }
	/**
	* @brief Puts an item into a slot, taking it over.
	* @details Whatever the slot held is removed with it, so a slot never holds two. The item is
	* added to the list when it is not one of its rows yet.
	*/
	void setSlot(int slot, PG_DisaActorModel* item);
	/// The row of that slot, with a sprite made for it when the slot was free.
	PG_ListModelLocation takeActorSlot(int slot);
	/// Removes the item of that slot, does nothing when the slot is empty.
	void clearSlot(int slot);
	/// Removes every actor the chain spawned, which is what dropping this does anyway.
	void resetScriptSlots();

	/// Where a coordinate parented to that slot counts from. @see PG_DisaActorListModel::getSlotOrigin
	glm::vec3 getSlotOrigin(int slot) const;

	/// Every actor the chain holds, which is what slot 202 stands in for while no battle runs.
	std::vector<PG_DisaActorModel*> getActors() const;

private:
	/// Row of this view that holds the slot, -1 when it holds none.
	int findRow(int slot) const;

	/* A lifeline because the destructor reaches into the list: a map closed before the chain ends
	 * would otherwise leave this pointing at something that is gone. */
	PG_Lifeline_T<PG_DisaActorListModel> m_originModel;
};
