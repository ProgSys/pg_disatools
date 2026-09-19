#include "PG_DisaAssetsModel.h"
#include <resources/PG_ResourceSynchronisator.h>
#include <Disa/fileformats/PG_TX2.h>
#include <Disa/resources/PG_DisaModel.h>
#include <Disa/fileformats/PG_MPP.h>
#include <Util/PG_ImageLoad.h>

#include <Stream/PG_StreamInOutByteVector.h>

struct TextureLoader {

	PG_Texture::ImageFormats operator()() const {
		PG_RGBAImage image;
		if (!decompressTX2(tx2, image)) {
			return image;
		}
		else {
			OUT_ERROR("Failed to decompress 'tx2'!");
		}
		return image;
	}

	PG_TX2 tx2;
};

struct TextureLoader2 {

	PG_Texture::ImageFormats operator()() const {
		PG_TX2 tx2;
		tx2.open(reader.get());

		PG_RGBAImage image;
		if (!decompressTX2(tx2, image)) {
			return image;
		}
		else {
			OUT_ERROR("Failed to decompress 'tx2'!");
		}
		return image;
	}
	std::shared_ptr<PG::STREAM::In> reader;
};

/*
* ============= PG_DisaAssetsTextureItem =============
*/

PG_Variety PG_DisaAssetsTextureItem::getData(Identity identity) const {
	switch (identity)
	{
	case Identity_SD: return getSDTexture();
	case Identity_HD: return getHDTexture();
	case Identity_HDNormal: return getHDNormalTexture();
	case Identity_SDModified: return sdLoc.getData(PG_DisaFileEntry::Identity_Modified);
	case Identity_HDModified: return hdLoc? hdLoc.getData(PG_DisaFileEntry::Identity_Modified): PG_Variety(false);
	case Identity_HDNormalModified: return hdNormalLoc? hdNormalLoc.getData(PG_DisaFileEntry::Identity_Modified) : PG_Variety(false);
	}
	return PG_Variety();
}

static inline const PG_Texture& textureUpdate(std::string&& name, PG_Texture& tex, const PG_ListModelLocation& loc, bool& changed) {
	if (changed && loc) {
		auto item = loc.getDataT<PG_DisaFileEntry>(PG_Identity_Item);
		if (item.getInfo().getSize() == 0 && !item.getContainer()) {
			tex.reset();
			return tex;
		}
		name += std::to_string(loc.getIndex());
		tex.load(name, TextureLoader2{ item.createReader() });
		changed = false;
	}
	return tex;
}

const PG_Texture& PG_DisaAssetsTextureItem::getSDTexture() const {
	return textureUpdate(PG_UNIQUE_RES("T_DisaSDTexture"), sd, sdLoc, sdChanged);
}
const PG_Texture& PG_DisaAssetsTextureItem::getHDTexture() const {
	return textureUpdate(PG_UNIQUE_RES("T_DisaHDTexture"), hd, hdLoc, hdChanged);
}
const PG_Texture& PG_DisaAssetsTextureItem::getHDNormalTexture() const {
	return textureUpdate(PG_UNIQUE_RES("T_DisaHDNormalTexture"), hdNormal, hdNormalLoc, hdNormalChanged);
}

PG_Variety PG_DisaAssetsGeometryItem::getData(Identity identity) const {
	switch (identity)
	{
	case Identity_SD: return getSDModel();
	case Identity_HD: return getHDModel();
	case Identity_SDModified: return sdLoc.getData(PG_DisaFileEntry::Identity_Modified);
	case Identity_HDModified: return hdLoc ? hdLoc.getData(PG_DisaFileEntry::Identity_Modified) : PG_Variety(false);
	}
	return PG_Variety();
}

/*
* ============= PG_DisaAssetsGeometryItem =============
*/

static inline const PG_Model& modelUpdate(std::string&& name, PG_Model& model, const PG_ListModelLocation& loc, bool& changed) {
	if (changed && loc) {
		PG_GEO geo;
		auto item = loc.getDataT<PG_DisaFileEntry>(PG_Identity_Item);
		geo.open(item.createReader().get());
		name += std::to_string(loc.getIndex());
		reinterpret_cast<PG_DisaModel*>(&model)->load(name, geo);
		changed = false;
	}
	return model;
}

const PG_Model& PG_DisaAssetsGeometryItem::getSDModel() const {
	return modelUpdate(PG_UNIQUE_RES("T_DisaSDModel"), sd, sdLoc, sdChanged);
}
const PG_Model& PG_DisaAssetsGeometryItem::getHDModel() const {
	return modelUpdate(PG_UNIQUE_RES("T_DisaHDModel"), hd, hdLoc, hdChanged);
}
/*
* ============= PG_DisaAssetsTextureFilterModel =============
*/

template<PG_IsDisaAsset T>
PG_ListModelLocation PG_DisaAssetsFilterModel<T>::getLocation(const PG_Index& index) const {
	if (!isIndexValid(index)) return PG_ListModelLocation();
	return PG_ListModelLocation((PG_ListModelBase*)this, index);
}

template<PG_IsDisaAsset T>
PG_Variety PG_DisaAssetsFilterModel<T>::getData(PG_Index index, Identity identity) const {
	if (index >= m_items.size()) return PG_Variety();
	switch (identity)
	{
	case PG_Identity_Item: return m_items[index];
	case PG_Identity_Location: return getLocation(index);
	default:
		break;
	}

	return  m_items[index].getData(identity);
}

template<PG_IsDisaAsset T>
static inline std::vector<glm::ivec2> toRanges(typename std::vector<T>::iterator begin, std::set<typename std::vector<T>::iterator> changed) {
	std::vector<glm::ivec2> ranges;
	ranges.emplace_back(std::distance(begin, *changed.begin()), -1);
	ranges.back().y = ranges.back().x;
	for (auto it : changed | std::views::drop(1)) {
		const int i = std::distance(begin, *changed.begin());
		if (ranges.back().y + 1 == i) {
			++ranges.back().y;
			continue;
		}
		ranges.emplace_back(i, i);
	}
	return ranges;
}


template<PG_IsDisaAsset T>
void PG_DisaAssetsFilterModel<T>::setModels(const std::shared_ptr<PG_MPPContainer>& modelSD, const std::shared_ptr<PG_MPPContainer>& modelHD) {

	
	if (m_modelSD == modelSD && m_modelHD == modelHD) return;

	if (m_modelSD != modelSD) {
		if (m_modelSD) {
			disconnect(m_modelSD.get(), nullptr, this, nullptr);
		}

		m_modelSD = modelSD;

		if (m_modelSD) {

			connect(m_modelSD.get(), &PG_ListModelBase::sizeChanged, this, &PG_DisaAssetsFilterModel<T>::invalidate);
			connect(m_modelSD.get(), &PG_ListModelBase::itemsChanged, this,
				[this](unsigned int startIndex, unsigned int endIndex, const std::set<Identity>& identities) {
					const bool itemChange = identities.contains(PG_Identity_Item) || identities.contains(PG_Identity_Location);
					const bool modifiedChange = identities.contains(PG_DisaFileEntry::Identity_Modified) || identities.contains(PG_DisaFileEntry::Identity_Container);
					if (!itemChange && !modifiedChange) return;

					std::set<typename std::vector<T>::iterator> changed;
					for (int i = startIndex; i <= endIndex; ++i) {
						auto findIt = std::ranges::find_if(m_items, [i](const T& item) { return item.containsSD(i); });
						if (findIt == m_items.end()) continue;
						changed.insert(findIt);
					}
					if (changed.empty()) return;

					const std::vector<glm::ivec2> ranges = toRanges<T>(m_items.begin(), changed);
					std::set<Identity> whatChanged;
					if (itemChange) {
						whatChanged.insert(T::Identity_SD);
						whatChanged.insert(T::Identity_SDModified);
					}
					else if (modifiedChange) {
						whatChanged.insert(T::Identity_SDModified);
					}
					for (glm::ivec2 range : ranges) {
						for (int i = range.x; i <= range.y; ++i) {
							m_items[i].sdChanged = true;
						}
						emit itemsChanged(range.x, range.y, whatChanged);
					}
				});

		}
		initializeSD();
	}

	if (m_modelHD != modelHD) {
		if (m_modelHD) {
			disconnect(m_modelHD.get(), nullptr, this, nullptr);
		}

		m_modelHD = modelHD;

		if (m_modelHD) {
			connect(m_modelHD.get(), &PG_ListModelBase::sizeChanged, this, &PG_DisaAssetsFilterModel<T>::invalidate);
			connect(m_modelHD.get(), &PG_ListModelBase::itemsChanged, this,
				[this](unsigned int startIndex, unsigned int endIndex, const std::set<Identity>& identities) {
					const bool itemChange = identities.contains(PG_Identity_Item) || identities.contains(PG_Identity_Location);
					const bool modifiedChange = identities.contains(PG_DisaFileEntry::Identity_Modified) || identities.contains(PG_DisaFileEntry::Identity_Container);
					if (!itemChange && !modifiedChange) return;
					

					std::set<typename std::vector<T>::iterator> changed;
					std::set<Identity> whatChanged;
					for (int i = startIndex; i <= endIndex; ++i) {
						auto findIt = std::ranges::find_if(m_items, [i](const T& item) { return item.containsHD(i); });
						if (findIt == m_items.end()) continue;
						whatChanged.insert(findIt->getHDIdentityOf(i));
						changed.insert(findIt);
					}
					if (changed.empty()) return;

					const std::vector<glm::ivec2> ranges = toRanges<T>(m_items.begin(), changed);

					if (itemChange) {
						whatChanged.insert(T::allHDIdentities.begin(), T::allHDIdentities.end());
					}
					else if (modifiedChange) {
						std::set<Identity> toModified;
						for (Identity ident : whatChanged) {
							toModified.insert(ident + 1);
						}
						whatChanged = std::move(toModified);
					}

					for (glm::ivec2 range : ranges) {
						for (int i = range.x; i <= range.y; ++i) {
							m_items[i].markChange(whatChanged);
						}
						emit itemsChanged(range.x, range.y, whatChanged);
					}
				});
		}
		initializeHD();
	}
}

template<PG_IsDisaAsset T>
void PG_DisaAssetsFilterModel<T>::resetHD() {
	if (m_items.empty()) return;

	constexpr bool hasHDNormal = requires(T t) {
		{ t.hdNormalChanged };
	};

	for (auto& item : m_items) {
		item.hd.reset();
		item.hdChanged = true;
		if constexpr (hasHDNormal) {
			item.hdNormal.reset();
			item.hdNormalChanged = true;
		}
	}
	if constexpr (hasHDNormal) {
		emit itemsChanged(0, m_items.size() - 1, {
			T::Identity_HD, T::Identity_HDModified,
			T::Identity_HDNormal, T::Identity_HDNormalModified
			});
	}
	else {
		emit itemsChanged(0, m_items.size() - 1, {
			T::Identity_HD, T::Identity_HDModified
			});
	}
}

template<PG_IsDisaAsset T>
void PG_DisaAssetsFilterModel<T>::invalidate() {
	postCallEvent(PG_Event_Behaviour_Singleton, &PG_DisaAssetsFilterModel<T>::initialize);
}

template<PG_IsDisaAsset T>
void PG_DisaAssetsFilterModel<T>::initializeSD() {
	const auto originalSize = m_items.size();
	if (!m_modelSD || m_modelSD->empty()) {
		if (originalSize > 0) {
			m_items.clear();
			emit itemsRemoved(0, originalSize - 1);
			emit sizeChanged();
		}
		return;
	}

	fillSD();

	//peform emits
	if (originalSize > m_items.size()) emit itemsRemoved(m_items.size(), originalSize - 1);
	else if (originalSize < m_items.size()) emit itemsAdded(originalSize, m_items.size() - 1);
	if (!m_items.empty()) emit itemsChanged(0, m_items.size() - 1, { PG_Identity_Item });
	if (originalSize != m_items.size()) emit sizeChanged();
}

template<PG_IsDisaAsset T>
void PG_DisaAssetsFilterModel<T>::initializeHD() {
	constexpr bool hasHDNormal = requires(T t) {
		{ t.hdNormalChanged };
	};
	
	const auto originalSize = m_items.size();

	if (!m_modelHD || m_modelHD->empty()) {
		if (originalSize > 0) {
			for (auto& item : m_items) {
				item.hd.reset();
				item.hdLoc.reset();
				item.hdChanged = true;

				if constexpr (hasHDNormal) {
					item.hdNormal.reset();
					item.hdNormalLoc.reset();
					item.hdNormalChanged = true;
				}
			}
			emit itemsChanged(0, originalSize - 1, { PG_Identity_Item });
		}
		return;
	}

	fillHD();

	std::set<Identity> whatChanged{ T::allHDIdentities.begin(), T::allHDIdentities.end() };
	emit itemsChanged(0, originalSize - 1, whatChanged);
}

/*
* ============= PG_DisaAssetsTextureFilterModel =============
*/

void PG_DisaAssetsTextureFilterModel::fillSD() {
	std::vector<PG_DisaAssetsTextureItem> items;
	items.reserve(m_modelSD->size());
	for (const PG_DisaFileEntry& item : *m_modelSD) {
		if (item.getInfo().getType() == PG_DisaFileInfo::TX2 && item.getInfo().getName().getPath()[0] != 'N') {
			items.emplace_back(PG_DisaAssetsTextureItem{ m_modelSD->getLocation(item.getLocation()) });
		}
	}
	m_items = std::move(items);
}

void PG_DisaAssetsTextureFilterModel::fillHD() {

	auto it = m_items.begin();
	auto itNormal = m_items.begin();

	for (const PG_DisaFileEntry& item : *m_modelHD) {
		if (item.getInfo().getType() == PG_DisaFileInfo::TX2) {
			const bool isNormal = item.getInfo().getName().getPath()[0] == 'N';

			if (isNormal) {
				if (itNormal != m_items.end()) {
					itNormal->hdNormalChanged = true;
					itNormal->hdNormalLoc = m_modelHD->getLocation(item.getLocation());
					++itNormal;
				}
			}
			else {
				if (it != m_items.end()) {
					it->hdChanged = true;
					it->hdLoc = m_modelHD->getLocation(item.getLocation());
					++it;
				}
			}
		}
	}
}



/*
* ============= PG_DisaAssetsTextureFilterModel =============
*/

void PG_DisaAssetsGeoemtryFilterModel::fillSD() {
	std::vector<PG_DisaAssetsGeometryItem> items;
	items.reserve(m_modelSD->size());
	for (const PG_DisaFileEntry& item : *m_modelSD) {
		if (item.getInfo().getType() == PG_DisaFileInfo::GEO) {
			items.emplace_back(PG_DisaAssetsGeometryItem{ m_modelSD->getLocation(item.getLocation()) });
		}
	}
	m_items = std::move(items);
}

void PG_DisaAssetsGeoemtryFilterModel::fillHD() {
	auto it = m_items.begin();
	for (const PG_DisaFileEntry& item : *m_modelHD) {
		if (item.getInfo().getType() == PG_DisaFileInfo::GEO && it != m_items.end()) {
			it->hdChanged = true;
			it->hdLoc = m_modelHD->getLocation(item.getLocation());
			++it;
		}
	}
}

/*
* ============= PG_DisaAssetsModel =============
*/

void PG_DisaAssetsModel::init() {
	m_textures = PG_new<TextureListModel>("PG_TextureListModel", this);
	m_geomeries = PG_new<GeometryListModel>("PG_GeometryListModel", this);

	connect(this, &PG_DisaAssetsModel::enabledHDChanged, this, [this]() {
		if (!isHDEnabled()) {
			m_textures->resetHD();
			m_geomeries->resetHD();
		}
		});
}


bool PG_DisaAssetsModel::open(PG_DisaFileEntry mppSDInfo, PG_DisaFileEntry mppHDInfo) {
	m_containerSD = mppSDInfo.open<PG_MPPContainer>();
	m_containerHD = mppHDInfo.open<PG_MPPContainer>();

	m_textures->setModels(m_containerSD, m_containerHD);
	m_geomeries->setModels(m_containerSD, m_containerHD);

	return true;
}

void PG_DisaAssetsModel::close() {
	m_modified.clear();
	m_containerSD.reset();
	m_containerHD.reset();
	m_textures->setModels(nullptr, nullptr);
	m_geomeries->setModels(nullptr, nullptr);
}

void PG_DisaAssetsModel::exportTexture(const PG_File& file, PG_ListModelLocation location, ETextureType type) const {
	if (!location || location.getIndex() >= m_textures->size()) return;

	PG_ListModelLocation loc;
	switch (type)
	{
	case PG_DisaAssetsModel::TextureSD: {
		loc = m_textures->getItems()[location.getIndex()].sdLoc;
	} break;
	case PG_DisaAssetsModel::TextureHD: {
		loc = m_textures->getItems()[location.getIndex()].hdLoc;
	} break;
	case PG_DisaAssetsModel::TextureNormal: {
		loc = m_textures->getItems()[location.getIndex()].hdNormalLoc;
	} break;
	default: return;
	}

	if (!loc) {
		OUT_ERROR("Texture type location not found!");
		return;
	}

	auto fileEntry = loc.getDataT<PG_DisaFileEntry>(PG_Identity_Item);
	if (!fileEntry) {
		OUT_ERROR("Failed to get file entry!");
		return;
	}

	PG_TX2 tx2;
	if (!tx2.open(fileEntry.createReader().get())) {
		OUT_ERROR("Failed to open TX2!");
		return;
	}

	if ("png" == file.getFileExtensionFormated()) {
		PG_RGBAImage image;
		if (decompressTX2(tx2, image)) {
			OUT_ERROR("Failed to decompress TX2!");
			return;
		}
		PG_ImageLoad::save(file, image);
	}
	else {
		tx2.save(file);
	}
}

void PG_DisaAssetsModel::importTexture(PG_TX2&& tx2, PG_ListModelLocation location, ETextureType type) {
	PG_ListModelLocation loc;
	switch (type)
	{
	case PG_DisaAssetsModel::TextureSD: {
		loc = m_textures->getItems()[location.getIndex()].sdLoc;
	} break;
	case PG_DisaAssetsModel::TextureHD: {
		loc = m_textures->getItems()[location.getIndex()].hdLoc;
	} break;
	case PG_DisaAssetsModel::TextureNormal: {
		loc = m_textures->getItems()[location.getIndex()].hdNormalLoc;
	} break;
	default: return;
	}

	if (!loc) {
		OUT_ERROR("Texture type location not found!");
		return;
	}

	auto fileEntry = loc.getDataT<PG_DisaFileEntry>(PG_Identity_Item);
	if (!fileEntry) {
		OUT_ERROR("Failed to get file entry!");
		return;
	}

	PG::STREAM::InOutByteVector outVec;
	if (!tx2.save(&outVec)) {
		OUT_ERROR("Failed to save tx2!");
		return;
	}

	auto oldContainer = fileEntry.getContainer();
	auto container = fileEntry.replace<PG_DisaChangedDataContainer>(std::move(outVec.getData()));
	if (!container) {
		OUT_ERROR("File already open as different type!");
		return;
	}

	if (oldContainer) {
		if (auto findIt = std::ranges::find(m_modified, oldContainer); findIt != m_modified.end()) {
			m_modified.erase(findIt);
		}
		oldContainer.reset();
	}

	m_modified.insert(std::move(container));
}

void PG_DisaAssetsModel::importTexture(const PG_File& file, PG_ListModelLocation location, ETextureType type, unsigned short tx2Type) {
	auto extension = file.getFileExtensionFormated();
	if (extension == "png") {
		PG_RGBAImage img;
		if (!PG_ImageLoad::load(file, img) || img.empty()) {
			OUT_ERROR("Failed to open png!");
			return;
		}
		PG_TX2 tx2;
		tx2.setWithRGBA(img.getWidth(), img.getHeight(), (char*)img.data());
		if (!tx2.convertTo((PG_TX2::Type)tx2Type)) {
			OUT_ERROR("Failed to compress tx2!");
		}
		importTexture(std::move(tx2), location, type);
	}
	else if(extension == "tx2"){
		PG_TX2 tx2;
		if (!tx2.open(file)) {
			OUT_ERROR("Failed to open tx2!");
			return;
		}

		importTexture(std::move(tx2), location, type);
	}
	else {
		OUT_ERROR("Unknown texture format: "<< extension);
	}
}

void PG_DisaAssetsModel::exportGeometry(const PG_File& file, PG_ListModelLocation location, bool hd) const {
	if (!location || location.getIndex() >= m_geomeries->size()) return;

	PG_ListModelLocation loc = hd ?
		m_geomeries->getItems()[location.getIndex()].hdLoc :
		m_geomeries->getItems()[location.getIndex()].sdLoc;

	if (!loc) {
		OUT_ERROR("Geometry type location not found!");
		return;
	}

	auto fileEntry = loc.getDataT<PG_DisaFileEntry>(PG_Identity_Item);
	if (!fileEntry) {
		OUT_ERROR("Failed to get file entry!");
		return;
	}

	PG_GEO geo;
	if (!geo.open(fileEntry.createReader().get())) {
		OUT_ERROR("Failed to open GEO!");
		return;
	}

	geo.save(file);
}

void PG_DisaAssetsModel::importGeometry(const PG_File& file, PG_ListModelLocation location, bool hd) {
	PG_GEO geo;
	if (!geo.open(file)) {
		OUT_ERROR("Failed to open Geo!");
		return;
	}

	importGeometry(geo, location, hd);
}

void PG_DisaAssetsModel::importGeometry(const PG_GEO& geo, PG_ListModelLocation location, bool hd) {
	PG_ListModelLocation loc = hd ?
		m_geomeries->getItems()[location.getIndex()].hdLoc :
		m_geomeries->getItems()[location.getIndex()].sdLoc;


	if (!loc) {
		OUT_ERROR("Texture type location not found!");
		return;
	}

	auto fileEntry = loc.getDataT<PG_DisaFileEntry>(PG_Identity_Item);
	if (!fileEntry) {
		OUT_ERROR("Failed to get file entry!");
		return;
	}

	PG::STREAM::InOutByteVector outVec;
	if (!geo.save(&outVec)) {
		OUT_ERROR("Failed to save Geo!");
		return;
	}

	auto container = fileEntry.replace<PG_DisaChangedDataContainer>(std::move(outVec.getData()));
	if (!container) {
		OUT_ERROR("File already open as different type!");
		return;
	}
	m_modified.insert(std::move(container));
}


void PG_DisaAssetsModel::importGeometryGLTF(const PG_File& file, PG_ListModelLocation location, bool hd) {
	PG_GEO geo;
	if (!geo.importGLTF(file)) {
		OUT_ERROR("Failed to read GLTF!");
		return;
	}

	//geo.save(file.replaceFileNameAndExtension("imporetedDebug.geo"));
	importGeometry(geo, location, hd);
}

int PG_DisaAssetsModel::addGeometry(PG_Model model) {
	PG_DisaAssetsGeometryItem item;
	item.sd = model;
	item.hd = std::move(model);
	//there is no location to reread from, so what is in there is what it stays
	item.sdChanged = false;
	item.hdChanged = false;
	return m_geomeries->add(std::move(item));
}

void PG_DisaAssetsModel::addItem(bool texture) {
	if (!m_containerSD || !m_containerHD ||
		m_containerSD->size() > 254 || m_containerHD->size() > 254) return;

	const auto statsSD = m_containerSD->getStats();
	const auto statsHD = m_containerHD->getStats();

	PG_DisaFileInfo info;
	if (texture) {

		std::vector<char> mocData = {
			'\x04','\x00','\x04','\x00','\x00','\x00','\x02','\x02',
			'\x00','\x00','\x00','\x00','\x00','\x00','\x01','\x00',
			'\x1F','\xF8','\x00','\x00','\x44','\x11','\x44','\x11'
		};
		
		info.setName(PG_File("TextureNewSD.TX2"));
		info.setType(PG_DisaFileInfo::TX2);

		auto it = m_containerSD->insert(std::next(m_containerSD->begin(), statsSD.texturesSize), PG_DisaFileEntry(info, m_containerSD.get()));
		m_modified.insert(it->open<PG_DisaChangedDataContainer>(mocData));
			
		info.setName(PG_File("TextureNewHD.TX2"));
		it = m_containerHD->insert(std::next(m_containerHD->begin(), statsHD.texturesSize), PG_DisaFileEntry(info, m_containerHD.get()));
		m_modified.insert(it->open<PG_DisaChangedDataContainer>(mocData));
		info.setName(PG_File("NormalNewHD.TX2"));
		it = m_containerHD->insert(std::next(m_containerHD->begin(), statsHD.texturesSize + 1 + statsHD.normalsSize), PG_DisaFileEntry(info, m_containerHD.get()));
		m_modified.insert(it->open<PG_DisaChangedDataContainer>(std::move(mocData)));
	}
	else {
		//TODO add moc data
		info.setName(PG_File("GeometryNewSD.GEO"));
		info.setType(PG_DisaFileInfo::GEO);

		m_containerSD->insert(std::next(m_containerSD->begin(), statsSD.totalSize()), PG_DisaFileEntry(info, m_containerSD.get()));
		
		info.setName(PG_File("GeometryNewHD.TX2"));
		m_containerHD->insert(std::next(m_containerHD->begin(), statsHD.totalSize()), PG_DisaFileEntry(info, m_containerHD.get()));
	}

}


PG_ResourceSynchronisator* PG_DisaAssetsModel::getResourceSynchronisator() {
	auto* synchronisator = PG_new<PG_ResourceSynchronisator>("PG_ResourceSynchronisator", this);
	for (const auto& item : m_textures->getItems()) {
		synchronisator->add(item.sd);
		synchronisator->add(item.hd);
		synchronisator->add(item.hdNormal);
	}
	for (const auto& item : m_geomeries->getItems()) {
		synchronisator->add(item.sd);
		synchronisator->add(item.hd);
	}
	return synchronisator;
}
