#include "PG_DisaItemModel.h"
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_TableDAT.h>
#include <Disa/fileformats/PG_TX2.h>
#include <Resources/PG_ResourceManager.h>
#include <Util/PG_StringUtil.h>

static PG_DisaItemsModel* g_global = nullptr;
static PG_DisaItemModel g_emptyItemModel = {
	static_cast<unsigned short>(-1),
	"Missingno",
	"Missingno",
	"",
	"",
	0,
	0
};

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

PG_Variety PG_DisaItemModel::getData(Identity identity) const {
	switch (identity)
	{
	case Identity_Id: return id;
	case Identity_Name: return nameNormelized;
	case Identity_Description: return descriptionNormelized;
	case Identity_PreviewImage: return getIconLocation(iconIndex, iconType);
	}
	return PG_Variety();
}


void PG_DisaItemsModel::init() {
	assert(!g_global);
	g_global = this;
}

void PG_DisaItemsModel::open(std::shared_ptr<PG_OLA> startDat) {
	m_startDat = startDat;

	if (!startDat) {
		clear();
		return;
	}

	auto entry = startDat->find(PG_File("MITEM.DAT"), false);
	if (!entry) {
		OUT_ERROR("Failed to find MITEM.DAT!");
		clear();
		return;
	}

	PG_TableDAT tableDat;
	tableDat.setDefFile(PG_File(PG_ResourceManager::resourceLocation() + "/script/disa/defFiles/MITEM.DEF"));
	auto read = entry.createReader();
	if (!tableDat.open(read)) {
		OUT_ERROR("Failed to read MITEM.DEF!");
		clear();
		return;
	}

	const int idIndex = tableDat.getHeaderIndex("ID");
	const int nameIndex = tableDat.getHeaderIndex("Name");
	const int descriptionIndex = tableDat.getHeaderIndex("Description");
	const int iconIndexIndex = tableDat.getHeaderIndex("IconIndex");
	const int iconTypeIndex = tableDat.getHeaderIndex("IconType");

	const int max = std::max(std::max(idIndex, nameIndex), descriptionIndex);

	if (idIndex < 0 || nameIndex < 0) {
		OUT_ERROR("Missing needed header data! (Needs: " << "ID" << ", " << "Name" << ")");
		clear();
		return;
	}

	std::vector<PG_DisaItemModel> models;
	models.reserve(tableDat.size());
	for (const PG_TebleDATRow& row : tableDat) {
		if (max >= row.columns.size()) {
			OUT_ERROR("Invalid column size!");
			break;
		}

		PG_DisaItemModel model;
		model.id = row.columns[idIndex].data.convert<decltype(model.id)>();
		model.name = row.columns[nameIndex].data.convert<decltype(model.name)>();
		model.nameNormelized = PG_StringUtil::fullwidthAndHalfwidthUtf8ToNormal(model.name);
		model.description = row.columns[descriptionIndex].data.convert<decltype(model.description)>();
		model.descriptionNormelized = PG_StringUtil::fullwidthAndHalfwidthUtf8ToNormal(model.description);
		if (iconIndexIndex < row.columns.size() && iconTypeIndex < row.columns.size()) {
			model.iconIndex = row.columns[iconIndexIndex].data.convert<decltype(model.iconIndex)>();
			model.iconType = row.columns[iconTypeIndex].data.convert<decltype(model.iconType)>();
		}
		models.push_back(std::move(model));
	}

	set(std::move(models));

	if (iconIndexIndex >= 0) {
		entry = startDat->find(PG_File("ITEM.TX2"), false);

		PG_TX2 tx2;
		auto reader = entry.createReader();
		if (tx2.open(reader.get())) {
			m_iconsTexture.load("DisaIconsAtlas", [tex = std::move(tx2)]() -> PG_Texture::ImageFormats {
				PG_RGBAImage out;
				decompressTX2(tex, out);
				return out;
				});

		}
	}
}

const PG_DisaItemModel& PG_DisaItemsModel::getItemModel(unsigned short id) {
	auto findIt = std::ranges::find(*this, id, &PG_DisaItemModel::id);
	if (findIt == end()) return g_emptyItemModel;
	return *findIt;
}

PG_DisaItemsModel* PG_DisaItemsModel::get() {
	return g_global;
}