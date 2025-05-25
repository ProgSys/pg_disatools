#include "PG_DisaGeoEffectsModel.h"
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_TableDAT.h>
#include <Disa/fileformats/PG_TX2.h>
#include <Resources/PG_ResourceManager.h>
#include <Util/PG_StringUtil.h>

static PG_DisaGeoEffectsModel* g_global = nullptr;
static PG_DisaGeoEffectModel g_emptyItemModel = {
	static_cast<unsigned short>(-1),
	"Missingno",
	"Missingno",
	"",
	"",
};

PG_DisaGeoEffectModel::operator std::string() const { 
	if ((nameNormelized.size() + descriptionNormelized.size()) < 16) return nameNormelized + ": " + descriptionNormelized;
	return nameNormelized + ": " + PG_StringUtil::mid(descriptionNormelized, 0, 15 - nameNormelized.size()) + "...";
}

static inline glm::vec4 getIconLocation(unsigned short iconIndex, unsigned short iconType) {
	glm::vec4 out;
	out.x = (iconIndex % 16) * 16.f;
	out.y = (iconIndex / 16) * 16.f;
	if (iconIndex == 78 && iconType == 2) {
		out.z = 32.f;
		out.x -= out.z;
		out.w = 16.f;
	}else if (iconIndex == 76 && iconType == 3) {
		out.z = 32.f;
		out.w = 32.f;
		out.x += out.z;
	}
	else{
		out.z = 16.f;
		out.w = 16.f;
	}

	return out;
}

PG_Variety PG_DisaGeoEffectModel::getData(Identity identity) const {
	switch (identity)
	{
	case Identity_Id: return id;
	case Identity_Name: return nameNormelized;
	case Identity_Description: return descriptionNormelized;
	}
	return PG_Variety();
}


void PG_DisaGeoEffectsModel::init() {
	assert(!g_global);
	g_global = this;
}

void PG_DisaGeoEffectsModel::open(std::shared_ptr<PG_OLA> startDat) {
	m_startDat = startDat;

	if (!startDat) {
		clear();
		return;
	}

	auto entry = startDat->find(PG_File("GE.DAT"), false);
	if (!entry) {
		OUT_ERROR("Failed to find GE.DAT!");
		clear();
		return;
	}

	PG_TableDAT tableDat;
	tableDat.setDefFile(PG_File(PG_ResourceManager::resourceLocation() + "/script/disa/defFiles/GE.DEF"));
	auto read = entry.createReader();
	if (!tableDat.open(read)) {
		OUT_ERROR("Failed to read GE.DEF!");
		clear();
		return;
	}

	const int idIndex = tableDat.getHeaderIndex("ID");
	const int nameIndex = tableDat.getHeaderIndex("Title");
	const int descriptionIndex = tableDat.getHeaderIndex("Description");

	const int max = std::max(std::max(idIndex, nameIndex), descriptionIndex);

	if (idIndex < 0 || nameIndex < 0) {
		OUT_ERROR("Missing needed header data! (Needs: " << "ID" << ", " << "Title" << ")");
		clear();
		return;
	}

	std::vector<PG_DisaGeoEffectModel> models;
	models.reserve(tableDat.size());
	for (const PG_TebleDATRow& row : tableDat) {
		if (max >= row.columns.size()) {
			OUT_ERROR("Invalid column size!");
			break;
		}

		PG_DisaGeoEffectModel model;
		model.id = row.columns[idIndex].data.convert<decltype(model.id)>();
		model.name = row.columns[nameIndex].data.convert<decltype(model.name)>();
		model.nameNormelized = PG_StringUtil::fullwidthAndHalfwidthUtf8ToNormal(model.name);
		model.description = row.columns[descriptionIndex].data.convert<decltype(model.description)>();
		model.descriptionNormelized = PG_StringUtil::fullwidthAndHalfwidthUtf8ToNormal(model.description);
		models.push_back(std::move(model));
	}

	set(std::move(models));
}

const PG_DisaGeoEffectModel& PG_DisaGeoEffectsModel::getItemModel(unsigned short id) {
	auto findIt = std::ranges::find(*this, id, &PG_DisaGeoEffectModel::id);
	if (findIt == end()) return g_emptyItemModel;
	return *findIt;
}

PG_DisaGeoEffectsModel* PG_DisaGeoEffectsModel::get() {
	return g_global;
}