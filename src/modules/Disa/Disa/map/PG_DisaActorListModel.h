#pragma once

#include <unordered_map>

#include <Base/PG_Lifeline.h>
#include <Disa/map/PG_DisaActorModel.h>

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
public:
	using PG_ListModel<PG_DisaActorModel*>::PG_ListModel;

	static constexpr int firstMapSlot = 0;
	static constexpr int lastMapSlot = 63;
	static constexpr int firstScriptSlot = 64;
	static constexpr int lastScriptSlot = 95;
	/// Not an item, the teleporter panel, which coordinates are counted from. @see getSlotOrigin
	static constexpr int slotTeleporter = 203;

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

	/// Hands the rows the map file placed their slots, in file order. @see isMapSlot
	void resetMapSlots();

	/**
	* @brief Starts keeping slot 203 on the teleporter panel.
	* @details Called once by the map model, after every one of its lists exists. The rule belongs
	* to the slot rather than to what stands in it, so a sprite dropped into 203 lands on the panel
	* the same way the mesh does.
	*/
	void setupTeleporter();

private:
	/// Puts whatever is in slot 203 onto the panel, next time round the event loop.
	void updateTeleporter();
	void doUpdateTeleporter();

	/* A lifeline rather than a pointer: an item removed anywhere else leaves the slot empty instead
	 * of leaving it pointing at something that is gone. */
	std::unordered_map<int, PG_Lifeline_T<PG_DisaActorModel>> m_slots;
};
