#include "PG_DisaMagicModel.h"
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_TableDAT.h>
#include <Disa/PG_DisaUiTextursModel.h>
#include <Resources/PG_ResourceManager.h>
#include <Util/PG_StringUtil.h>

static PG_DisaMagicsModel* g_global = nullptr;
static PG_DisaMagicModel g_emptyItemModel = {
	static_cast<unsigned short>(-1),
	"Missingno",
	"Missingno",
	"",
	"",
};

PG_Variety PG_DisaMagicModel::getData(Identity identity) const {
	switch (identity)
	{
	case Identity_Id: return id;
	case Identity_Name: return nameNormelized;
	case PG_Identity_Tooltip:
	case Identity_Description: return descriptionNormelized;
	case PG_Identity_PreviewImage: return PG_DisaUiTextursModel::getIconLocation(iconIndex, 0);
	}
	return PG_Variety();
}


void PG_DisaMagicsModel::init() {
	assert(!g_global);
	g_global = this;
}

void PG_DisaMagicsModel::open(std::shared_ptr<PG_OLA> startDat) {
	m_startDat = startDat;

	if (!startDat) {
		clear();
		return;
	}

	auto entry = startDat->find(PG_File("MAGIC.DAT"), false);
	if (!entry) {
		OUT_ERROR("Failed to find MAGIC.DAT!");
		clear();
		return;
	}

	PG_TableDAT tableDat;
	tableDat.setDefFile(PG_File(PG_ResourceManager::resourceLocation() + "/script/disa/defFiles/MAGIC.DEF"));
	auto read = entry.createReader();
	if (!tableDat.open(read)) {
		OUT_ERROR("Failed to read MAGIC.DEF!");
		clear();
		return;
	}

	const int idIndex = tableDat.getHeaderIndex("ID");
	const int nameIndex = tableDat.getHeaderIndex("Name");
	const int descriptionIndex = tableDat.getHeaderIndex("Description");

	const int max = std::max(std::max(idIndex, nameIndex), descriptionIndex);

	if (idIndex < 0 || nameIndex < 0) {
		OUT_ERROR("Missing needed header data! (Needs: " << "ID" << ", " << "Name" << ")");
		clear();
		return;
	}

	std::vector<PG_DisaMagicModel> models;
	models.reserve(tableDat.size());
	for (const PG_TebleDATRow& row : tableDat) {
		if (max >= row.columns.size()) {
			OUT_ERROR("Invalid column size!");
			break;
		}

		PG_DisaMagicModel model;
		model.id = row.columns[idIndex].data.convert<decltype(model.id)>();
		model.name = row.columns[nameIndex].data.convert<decltype(model.name)>();
		model.nameNormelized = PG_StringUtil::fullwidthAndHalfwidthUtf8ToNormal(model.name);
		model.description = row.columns[descriptionIndex].data.convert<decltype(model.description)>();
		model.descriptionNormelized = PG_StringUtil::fullwidthAndHalfwidthUtf8ToNormal(model.description);

		if (model.descriptionNormelized.starts_with('(')) {
			auto closeCaption = model.descriptionNormelized.find_first_of(')');
			if (closeCaption != std::string::npos) {
				model.iconIndex = PG_StringUtil::toInt({ model.descriptionNormelized.begin() + 1,  model.descriptionNormelized.begin() + closeCaption });
				if (model.iconIndex > 0) --model.iconIndex;
			}
		}
		else {
			model.iconIndex = 13;
		}


		models.push_back(std::move(model));
	}
	std::ranges::stable_sort(models, {}, [](const PG_DisaMagicModel& m) { return !(m.iconIndex == 6); });

	set(std::move(models));

}

const PG_DisaMagicModel& PG_DisaMagicsModel::getMagicModel(unsigned short id) {
	auto findIt = std::ranges::find(*this, id, &PG_DisaMagicModel::id);
	if (findIt == end()) return g_emptyItemModel;
	return *findIt;
}

PG_DisaMagicsModel* PG_DisaMagicsModel::get() {
	return g_global;
}