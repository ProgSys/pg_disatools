#include "PG_DisaModLoadOrderListModel.h"
#include <Disa/fileformats/PG_JsonUtil.h>


void PG_DisaModLoadOrderListModel::open(const PG_File& file) {
	readJson(file.readAllAsString());
}

bool PG_DisaModLoadOrderListModel::save(const PG_File& file) const {
	auto data = writeJson();
	if (data.empty()) {
		return file.remove();
	}
	return file.write(data);
}

void PG_DisaModLoadOrderListModel::readJson(const std::string& jsonStr) {
	if (jsonStr.empty()) {
		clear();
		return;
	}

	auto j = nlohmann::json::parse(jsonStr);
	std::vector<PG_DisaModLoadOrderListModelItem> items;
	items.reserve(j.size());

	for (const auto& entry : j) {
		PG_DisaModLoadOrderListModelItem item;
		if (!PG_JsonUtil::readFromJsonMandatory(entry, "id", item.id)) continue;
		if (!PG_JsonUtil::readFromJsonMandatory(entry, "name", item.name)) continue;
		item.disabled = false;
		PG_JsonUtil::readFromJson(entry, "disabled", item.disabled);
		items.push_back(std::move(item));
	}
	set(std::move(items));
}

std::string PG_DisaModLoadOrderListModel::writeJson() const {
	nlohmann::json j;
	for (const auto& item : *this) {
		nlohmann::json sub;
		sub["id"] = item.id;
		sub["name"] = item.name;
		if(item.disabled) sub["disabled"] = true;
		j.push_back(std::move(sub));
	}
	return j.dump(3);
}

std::unordered_map<std::string, std::size_t> PG_DisaModLoadOrderListModel::getOrderMapping() const {
	std::unordered_map<std::string, std::size_t> out;
	std::size_t index = 0;
	for (auto& i : *this) {
		if (!i.name.empty() && !i.id.empty()) {
			out[i.id] = index;
		}
		++index;
	}
	return out;
}

PG_Variety PG_DisaModLoadOrderListModel::getData(PG_Index index, Identity identity) const {
	switch (identity) {
	case Identity_Id: return m_items[index].id;
	case Identity_Disabled: return m_items[index].disabled;
	}
	return PG_ListModel<PG_DisaModLoadOrderListModelItem>::getData(index, identity);
}
