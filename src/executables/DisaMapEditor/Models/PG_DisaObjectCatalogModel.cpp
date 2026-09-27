#include "PG_DisaObjectCatalogModel.h"
#include <Disa/PG_DisaAssetsModel.h>
#include <Resources/PG_ResourceManager.h>

void PG_DisaObjectCatalogModel::setAssets(PG_DisaAssetsModel* assets) {
	if (m_assets == assets) return;

	if (m_assets) {
		disconnect(m_assets, &PG_Object::onDeleted, this, &PG_DisaObjectCatalogModel::onAssetsDeleted);
		disconnect(m_assets, &PG_DisaAssetsModel::enabledHDViewChanged, this, &PG_DisaObjectCatalogModel::onHDViewChanged);
		if (auto* geometries = m_assets->getGeometries()) {
			disconnect(geometries, &PG_ListModelBase::sizeChanged, this, &PG_DisaObjectCatalogModel::requestRebuild);
			disconnect(geometries, &PG_ListModelBase::itemsChanged, this, &PG_DisaObjectCatalogModel::onGeometriesChanged);
		}
	}

	m_assets = assets;

	if (assets) {
		connect(assets, &PG_Object::onDeleted, this, &PG_DisaObjectCatalogModel::onAssetsDeleted);
		connect(assets, &PG_DisaAssetsModel::enabledHDViewChanged, this, &PG_DisaObjectCatalogModel::onHDViewChanged);
		if (auto* geometries = assets->getGeometries()) {
			connect(geometries, &PG_ListModelBase::sizeChanged, this, &PG_DisaObjectCatalogModel::requestRebuild);
			connect(geometries, &PG_ListModelBase::itemsChanged, this, &PG_DisaObjectCatalogModel::onGeometriesChanged);
		}
	}
	requestRebuild();
}

void PG_DisaObjectCatalogModel::setNameSpace(std::string nameSpace) {
	if (m_nameSpace == nameSpace) return;
	m_nameSpace = std::move(nameSpace);
	requestRebuild();
}

void PG_DisaObjectCatalogModel::requestRebuild() {
	postCallEvent(PG_Event_Behaviour_Singleton, &PG_DisaObjectCatalogModel::rebuild);
}

void PG_DisaObjectCatalogModel::rebuild() {
	std::vector<PG_DisaObjectCatalogItem> items;

	if (m_assets && m_assets->getGeometries()) {
		const int geometryCount = static_cast<int>(m_assets->getGeometries()->size());
		items.reserve(geometryCount);
		for (int i = 0; i < geometryCount; ++i) {
			PG_DisaObjectCatalogItem& item = items.emplace_back();
			item.geometryIndex = i;
			item.displayName = std::to_string(i);
		}
	}

	if (!m_nameSpace.empty()) {
		for (const PG_ResourceCatalogItem& block : PG_ResourceManager::get()->findPGRBlocks<PG_Model>(m_nameSpace)) {
			PG_DisaObjectCatalogItem& item = items.emplace_back();
			item.resourceName = std::string(PG_ResourceName::split(block.name).name);
			item.displayName = item.resourceName;
		}
	}

	set(std::move(items));
}

PG_Variety PG_DisaObjectCatalogModel::getData(PG_Index index, Identity identity) const {
	if (!isIndexValid(index)) return PG_Variety();
	const PG_DisaObjectCatalogItem& item = getRef(index);

	switch (identity) {
	case PG_Identity_Text: return item.displayName;
	case PG_Identity_Tooltip: return item.isMod() ? PG_ResourceName::join(m_nameSpace, item.resourceName) : "Geometry " + item.displayName;
	case PG_Identity_PreviewImage: return getModel(item);
	case Identity_IsMod: return item.isMod();
	case Identity_GeometryIndex: return item.geometryIndex;
	case Identity_ResourceName: return item.resourceName;
	default: break;
	}
	return PG_ListModel<PG_DisaObjectCatalogItem>::getData(index, identity);
}

PG_Model PG_DisaObjectCatalogModel::getModel(const PG_DisaObjectCatalogItem& item) const {
	if (item.isMod()) {
		if (!item.model.isValid()) item.model = PG_Model(PG_ResourceName::join(m_nameSpace, item.resourceName));
		return item.model;
	}

	if (!m_assets || !m_assets->getGeometries()) return PG_Model();
	const auto& geometries = m_assets->getGeometries()->getItems();
	if (item.geometryIndex < 0 || item.geometryIndex >= static_cast<int>(geometries.size())) return PG_Model();

	const PG_DisaAssetsGeometryItem& geometry = geometries[item.geometryIndex];
	if (m_assets->isHDViewEnabled()) {
		if (const PG_Model& hd = geometry.getHDModel(); hd.isValid()) return hd;
	}
	return geometry.getSDModel();
}

void PG_DisaObjectCatalogModel::onGeometriesChanged(PG_Index startIndex, PG_Index endIndex, const PG_IdentitySet&) {
	//the geometries lead the list, so their indices are the same in here
	if (empty()) return;
	const std::size_t start = startIndex;
	const std::size_t end = std::min<std::size_t>(endIndex, size() - 1);
	if (start > end) return;
	emit itemsChanged(start, end, { PG_Identity_PreviewImage });
}

void PG_DisaObjectCatalogModel::onAssetsDeleted() {
	//its connections go with it, so there is nothing left to disconnect
	m_assets = nullptr;
	requestRebuild();
}

void PG_DisaObjectCatalogModel::onHDViewChanged() {
	if (!m_assets || !m_assets->getGeometries()) return;
	const std::size_t geometryCount = std::min(m_assets->getGeometries()->size(), size());
	if (geometryCount == 0) return;
	emit itemsChanged(0, geometryCount - 1, { PG_Identity_PreviewImage });
}
