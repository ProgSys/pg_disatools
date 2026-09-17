#include "PG_DisaMapModel.h"
#include <array>
#include <optional>
#include <unordered_set>
#include <numeric>
#include <Disa/resources/PG_DisaModel.h>
#include <Disa/PG_DisaAssetsModel.h>
#include <Disa/PG_DisaCharactersModel.h>
#include <Disa/map/PG_DisaEventTileModel.h>
#include <Disa/map/PG_DisaActorListModel.h>
#include <Disa/map/PG_DisaSpriteModel.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>
#include <resources/PG_ResourceSynchronisator.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Base/Thread/PG_Async.h>

constexpr float g_tileDim = 1.0f;

template<typename T>
inline float ecn(T c) {
	return -(float)c / 12.0f;
}
inline char ec(float c) {
	return static_cast<char>(std::round(glm::clamp(-c * 12.0f, -128.f, 128.f)));
}

inline bool isValid(const mpdfileTexdata& tex) {
	return tex.height != 0 && tex.width != 0 && tex.textureIndex != 0;
}

inline int getTextureIndex(const mpdfileTexdata& tex) {
	if (isValid(tex))
		return tex.textureIndex;
	return -1;
}

char PG_DisaMapTileModel::roundHeight(float value) {
	return ec(value);
}


/*
* =============== PG_DisaMapTileMaterialModel ===============
*/

template<typename T, typename F>
static inline void change(PG_DisaMapTileMaterialModel* self, T& target, const T& value, F&& changed) {
	if (target == value) return;
	target = value;
	emit (self->*changed)();
	self->getTile()->triggerChange(PG_DisaMapTileModel::Identity_Geometry);
}

template<typename T, typename F>
static inline void changeUV(PG_DisaMapTileMaterialModel* self, T& target, const T& value, F&& changed) {
	if (target == value) return;
	target = value;
	emit (self->*changed)();
	emit self->uvChanged();
	self->getTile()->triggerChange(PG_DisaMapTileModel::Identity_Geometry);
}


void  PG_DisaMapTileMaterialModel::setTileModel(PG_DisaMapTileModel* model) { 
	if (m_tile) {
		disconnect(m_tile, nullptr, this, nullptr);
	}
	m_tile = model;
	if (m_tile) {
		connect(m_tile, &PG_DisaMapTileModel::textureIndexChanged, this, [this](int slotIndex) {
			if (getIndex() == slotIndex) {
				emit textureIndexChanged();
			}
			});
	}
}

mpdfileTexdata& PG_DisaMapTileMaterialModel::getData() const {
	return getTile()->m_data.textures[m_index];
}

void PG_DisaMapTileMaterialModel::setTextureIndex(unsigned char index) {
	change(this, getData().textureIndex, index, &PG_DisaMapTileMaterialModel::textureIndexChanged);
}

PG_Texture PG_DisaMapTileMaterialModel::getTexture() const {
	if (!m_tile || getData().textureIndex == 0) return PG_Texture();
	auto* model = m_tile->findParent<PG_DisaMapModel>(4);
	if (!model) return PG_Texture();
	auto item = model->getAssets()->getTextures()->getData(
		getData().textureIndex - 1, 
		model->getAssets()->isHDViewEnabled()? PG_DisaAssetsTextureItem::Identity_HD: PG_DisaAssetsTextureItem::Identity_SD
	);
	if (!item) return PG_Texture();
	return item.get<PG_Texture>();
}

void PG_DisaMapTileMaterialModel::setU(unsigned char value) {
	changeUV(this, getData().u, value, &PG_DisaMapTileMaterialModel::uChanged);
}

void PG_DisaMapTileMaterialModel::setV(unsigned char value) {
	auto& data = getData();
	if (data.v == value) return;
	int diff = value - data.v;
	data.v = value;
	data.vRight = (data.vRight + diff) % 256;
	emit vChanged();
	emit vRightChanged();
	emit uvChanged();
	getTile()->triggerChange(PG_DisaMapTileModel::Identity_Geometry);
}

void PG_DisaMapTileMaterialModel::setVRight(unsigned char value) {
	changeUV(this, getData().vRight, value, &PG_DisaMapTileMaterialModel::vRightChanged);
}


void PG_DisaMapTileMaterialModel::setWidth(unsigned char value) {
	changeUV(this, getData().width, value, &PG_DisaMapTileMaterialModel::widthChanged);
}

void PG_DisaMapTileMaterialModel::setHeight(unsigned char value) {
	auto& data = getData();
	if (data.height == value) return;
	int diff = value - data.height;
	data.height = value;
	data.heightRight = (data.heightRight + diff) % 256;
	emit heightChanged();
	emit heightRightChanged();
	emit uvChanged();
	getTile()->triggerChange(PG_DisaMapTileModel::Identity_Geometry);
}

void PG_DisaMapTileMaterialModel::setHeightRight(unsigned char value) {
	changeUV(this, getData().heightRight, value, &PG_DisaMapTileMaterialModel::heightRightChanged);
}

void PG_DisaMapTileMaterialModel::setRotation(int value) {
	if (value < 0) return;
	unsigned char transform = (getData().transform & ~0b11) | (value & 0b11);
	change(this, getData().transform, transform, &PG_DisaMapTileMaterialModel::rotationChanged);
}


void PG_DisaMapTileMaterialModel::setMirrorVertical(bool value) {
	unsigned char transform = value? getData().transform | 0b100: getData().transform & ~0b100;
	change(this, getData().transform, transform, &PG_DisaMapTileMaterialModel::mirrorVerticalChanged);
}

void PG_DisaMapTileMaterialModel::setMirrorHorizontal(bool value) {
	unsigned char transform = value ? getData().transform | 0b1000 : getData().transform & ~0b1000;
	change(this, getData().transform, transform, &PG_DisaMapTileMaterialModel::mirrorHorizontalChanged);
}

void PG_DisaMapTileMaterialModel::setIsShadow(bool value) {
	unsigned char transform = value ? getData().transform | 0b110000 : getData().transform & ~0b110000;
	change(this, getData().transform, transform, &PG_DisaMapTileMaterialModel::isShadowChanged);
}

std::array<glm::vec2, 4> PG_DisaMapTileMaterialModel::getUVOutline() const {
	return std::array<glm::vec2, 4>{
		glm::vec2(getU(), getV())/255.f,
		glm::vec2(getU(), getV() + getHeight()) / 255.f,
		glm::vec2(getU() + getWidth(), getVRight() + getHeightRight() ) / 255.f,
		glm::vec2(getU() + getWidth(), getVRight()) / 255.f,
	};
}

/*
* =============== PG_DisaMapTileModel ===============
*/

void PG_DisaMapTileModel::markAsBaseTile() {
	if (m_isBaseTile) return;
	if (auto* base = getParent<PG_DisaMapTileListModel>(); base && getChunk()) {
		for (auto* item : *base) {
			if (getChunk() == item->getChunk()) {
				if (item->m_isBaseTile) {
					item->m_isBaseTile = false;
					emit item->isBaseTileChanged();
				}
			}
		}
	}
	m_isBaseTile = true;
	emit isBaseTileChanged();
}

bool PG_DisaMapTileModel::isStartingPoint() const {
	auto* model = findParent<PG_DisaMapModel>(4);
	if (!model) return false;

	for (auto* e : *model->getEvents()) {
		if (e->getValue() == 1) {
			return e->getTilePosition() == getTilePosition();
		}
	}
	return false;
}

void PG_DisaMapTileModel::markAsStartingPoint() {
	auto* model = findParent<PG_DisaMapModel>(4);
	if (!model) return;
	for (auto* e : *model->getEvents()) {
		if (e->getValue() == 1) {
			if (e->getTilePosition() != getTilePosition()) {
				e->setTilePosition(getTilePosition());
				emit isStartingPointChanged();
			}
			return;
		}
	}
	//add new start event
	model->getEvents()->addEvent(getTilePosition());
	model->getEvents()->back()->setValue(1);
	emit isStartingPointChanged();
}

void PG_DisaMapTileModel::setChunk(PG_DisaMapChunkModel* model) {
	assert(model);
	if (m_chunk == model) return;
	m_chunk = model;

	if (auto* base = getParent<PG_ListModelBase>(); base && getLocation() >= 0) {
		emit base->itemsChanged(getLocation(), getLocation(), { Identity_Geometry, Identity_Translation });
	}
}

std::array<float, 4> PG_DisaMapTileModel::getTopValues() const {
	const float height = getTranslation().y;
	return std::array<float, 4>{
			(height + ecn(m_data.cornersTop[1])), // 0
			(height + ecn(m_data.cornersTop[0])), // 1
			(height + ecn(m_data.cornersTop[3])), // 2
			(height + ecn(m_data.cornersTop[2])), //3
	};
}

void PG_DisaMapTileModel::setTopValues(const std::array<float, 4>& values) {
	const float height = getTranslation().y;
	std::array<char, 4> newCornersTop{
			(ec(values[1] - height)), // 0
			(ec(values[0] - height)), // 1
			(ec(values[3] - height)), // 2
			(ec(values[2] - height)), //3
	};
	if (m_data.cornersTop == newCornersTop) return;
	std::array<char, 4> diff = newCornersTop;
	for(int i = 0; i < 4; ++i)
		diff[i] -= m_data.cornersTop[i];

	const auto apply = [&](int index, int indexTex, int indexTexRight) {
		if (m_data.textures[index].height != 0) {
			float unitHeight = float(m_data.textures[index].height) / -m_data.cornersTop[indexTex];
			if (glm::isnan(unitHeight)) unitHeight = 4.f;
			m_data.textures[index].height -= static_cast<unsigned char>(std::round(unitHeight * diff[indexTex]));
		}
		else {
			m_data.textures[index].height = ((int)newCornersTop[indexTex]) * 4;
		}

		if (m_data.textures[index].heightRight != 0) {
			float unitHeight = float(m_data.textures[index].heightRight) / -m_data.cornersTop[indexTexRight];
			if (glm::isnan(unitHeight)) unitHeight = 4.f;
			m_data.textures[index].heightRight -= static_cast<unsigned char>(std::round(unitHeight * diff[indexTexRight]));
		}
		else {
			m_data.textures[index].heightRight = ((int)newCornersTop[indexTexRight]) * 4;
		}
		
		};

	//mod side textures
	apply(0, 0, 1);
	apply(1, 1, 3);
	apply(2, 3, 2);
	apply(3, 2, 0);

	m_data.cornersTop = std::move(newCornersTop);

	if (auto* base = getParent<PG_ListModelBase>(); base && getLocation() >= 0) {
		emit base->itemsChanged(getLocation(), getLocation(), { Identity_Geometry });
	}
}

std::array<glm::vec3, 4> PG_DisaMapTileModel::getTopData(const mpdfileTile& data, const glm::vec3& offset) {
	return std::array<glm::vec3, 4>{
			(offset + glm::vec3(0.0f, ecn(data.cornersTop[1]), 0.0f)), // 0
			(offset + glm::vec3(g_tileDim, ecn(data.cornersTop[0]), 0.0f)), // 1
			(offset + glm::vec3(0.0f, ecn(data.cornersTop[3]), g_tileDim)), // 2
			(offset + glm::vec3(g_tileDim, ecn(data.cornersTop[2]), g_tileDim)), //3
	};
}

std::array<glm::vec3, 4> PG_DisaMapTileModel::getTopCustom(const glm::vec3& offset, std::array<char, 4> cornersTop) {
	return std::array<glm::vec3, 4>{
		(offset + glm::vec3(0.0f, ecn(cornersTop[1]), 0.0f)), // 0
			(offset + glm::vec3(g_tileDim, ecn(cornersTop[0]), 0.0f)), // 1
			(offset + glm::vec3(0.0f, ecn(cornersTop[3]), g_tileDim)), // 2
			(offset + glm::vec3(g_tileDim, ecn(cornersTop[2]), g_tileDim)), //3
	};
}

float PG_DisaMapTileModel::getTopHeight() const {
	const auto values = getTopValues();
	return std::accumulate(values.begin(), values.end(), 0.f) / values.size();
}

float PG_DisaMapTileModel::getMaxHeight() const {
	const auto values = getTopValues();
	return *std::max_element(values.begin(), values.end());
}

float PG_DisaMapTileModel::getRefHeight() const {
	const auto values = getTopValues();
	return values[1];
}

bool PG_DisaMapTileModel::hasTop() const {
	return isValid(m_data.textures[4]);
}
bool PG_DisaMapTileModel::hasSideN() const {
	return isValid(m_data.textures[0]);
}
bool PG_DisaMapTileModel::hasSideE() const {
	return isValid(m_data.textures[1]);
}
bool PG_DisaMapTileModel::hasSideS() const {
	return isValid(m_data.textures[2]);
}
bool PG_DisaMapTileModel::hasSideW() const {
	return isValid(m_data.textures[3]);
}

void PG_DisaMapTileModel::setSideIVisible(int index, bool value) {
	assert(index <= 4);
	if (static_cast<bool>(m_data.textures[index].textureIndex) == value) return;
	if (value) {
		auto& tex = m_data.textures[index];
		tex.textureIndex = m_data.padding[index];
		if (tex.textureIndex <= 0) tex.textureIndex = 1;
		if (tex.width <= 0) tex.width = 32;
		if (tex.height <= 0) tex.height = 32;
	}
	else {
		m_data.padding[index] = m_data.textures[index].textureIndex;
		m_data.textures[index].textureIndex = 0;
	}
	if (auto* base = getParent<PG_ListModelBase>(); base && getLocation() >= 0) {
		emit base->itemsChanged(getLocation(), getLocation(), { Identity_Geometry });
	}
	emit sideVisibilityChanged();
}

int PG_DisaMapTileModel::getTopTextureIndex(const mpdfileTile& data) {
	return getTextureIndex(data.textures[4]);
}
int PG_DisaMapTileModel::getSideNTextureIndex(const mpdfileTile& data) {
	return getTextureIndex(data.textures[0]);
}
int PG_DisaMapTileModel::getSideETextureIndex(const mpdfileTile& data) {
	return getTextureIndex(data.textures[1]);
}
int PG_DisaMapTileModel::getSideSTextureIndex(const mpdfileTile& data) {
	return getTextureIndex(data.textures[2]);
}
int PG_DisaMapTileModel::getSideWTextureIndex(const mpdfileTile& data) {
	return getTextureIndex(data.textures[3]);
}

std::array<glm::vec3, 4> PG_DisaMapTileModel::getSideNData(const mpdfileTile& data, const glm::vec3& offset) {
	return std::array<glm::vec3, 4>{
		(offset + glm::vec3(0.0f, ecn(data.sideN[1]), 0.0f)), // 0
			(offset + glm::vec3(g_tileDim, ecn(data.sideN[0]), 0.0f)), // 1
			(offset + glm::vec3(0.0f, ecn(data.cornersTop[1]), 0.0f)), // 2
			(offset + glm::vec3(g_tileDim, ecn(data.cornersTop[0]), 0.0f)), //3
	};
}

std::array<glm::vec3, 4> PG_DisaMapTileModel::getSideWData(const mpdfileTile& data, const glm::vec3& offset) {
	return std::array<glm::vec3, 4>{
		(offset + glm::vec3(g_tileDim, ecn(data.sideW[0]), 0.0f)), // 0
			(offset + glm::vec3(g_tileDim, ecn(data.sideW[1]), g_tileDim)), // 1
			(offset + glm::vec3(g_tileDim, ecn(data.cornersTop[0]), 0.0f)), // 2
			(offset + glm::vec3(g_tileDim, ecn(data.cornersTop[2]), g_tileDim)), //3
	};
}

std::array<glm::vec3, 4> PG_DisaMapTileModel::getSideSData(const mpdfileTile& data, const glm::vec3& offset) {
	return std::array<glm::vec3, 4>{
		(offset + glm::vec3(g_tileDim, ecn(data.sideS[1]), g_tileDim)), // 0
			(offset + glm::vec3(0.0f, ecn(data.sideS[0]), g_tileDim)), // 1
			(offset + glm::vec3(g_tileDim, ecn(data.cornersTop[2]), g_tileDim)), // 2
			(offset + glm::vec3(0.0f, ecn(data.cornersTop[3]), g_tileDim)), //3
	};
}
std::array<glm::vec3, 4> PG_DisaMapTileModel::getSideEData(const mpdfileTile& data, const glm::vec3& offset) {
	return std::array<glm::vec3, 4>{
		(offset + glm::vec3(0.0f, ecn(data.sideE[0]), g_tileDim)), // 0
			(offset + glm::vec3(0.0f, ecn(data.sideE[1]), 0.0f)), // 1
			(offset + glm::vec3(0.0f, ecn(data.cornersTop[3]), g_tileDim)), // 2
			(offset + glm::vec3(0.0f, ecn(data.cornersTop[1]), 0.0f)), //3
	};
}

PG_Variety PG_DisaMapTileModel::getData(Identity identity) const {
	switch (identity) {
	case Identity_Geometry: return m_data;
	case Identity_Translation: return getTranslation();
	case Identity_GeoColor: return getGeoColor();
	case Identity_TopHeight: return getTopHeight();
	case Identity_MaxHeight: return getMaxHeight();
	case Identity_RefHeight: return getRefHeight();
	}
	return PG_Variety();
}

bool PG_DisaMapTileModel::setData(const PG_Variety& value, Identity identity){
	try {
		switch (identity) {
		case Identity_Translation: {
			if (auto v = value.get<glm::vec3>(); v != getTranslation()) {
				setTranslation(v);
				return true;
			}
		}break;
		case Identity_Geometry: {
			if (value.is<mpdfileTile>()) {
				setData(value.get<mpdfileTile>());
				return true;
			}
		}break;
		}
	}
	catch (...) {}

	return false;
}

void PG_DisaMapTileModel::setGeoColor(EDisaGeoColor color) {
	if (getGeoColor() == color) return;
	if (color == DisaGeoColor_None) {
		m_data.geo_mark = 0;
		m_data.geo_color = 0;
	}
	else {
		m_data.geo_mark = 100;
		m_data.geo_color = unsigned char(color) - 1;
	}

	emit geoColorChanged();
	triggerChange(Identity_GeoColor);
}

template<typename T, typename F>
static inline void change(PG_DisaMapTileModel* self, T& target, const T& value, F&& changed) {
	if (target == value) return;
	target = value;
	(self->*changed)();
	self->triggerChange(PG_DisaMapTileModel::Identity_Geometry);
}

void PG_DisaMapTileModel::setTopA(char value) {
	value *= -1;
	change(this, m_data.cornersTop[0], value, &PG_DisaMapTileModel::topAChanged);
}
void PG_DisaMapTileModel::setTopB(char value) {
	value *= -1;
	change(this, m_data.cornersTop[1], value, &PG_DisaMapTileModel::topBChanged);
}
void PG_DisaMapTileModel::setTopC(char value) {
	value *= -1;
	change(this, m_data.cornersTop[2], value, &PG_DisaMapTileModel::topCChanged);
}
void PG_DisaMapTileModel::setTopD(char value) {
	value *= -1;
	change(this, m_data.cornersTop[3], value, &PG_DisaMapTileModel::topDChanged);
}

void PG_DisaMapTileModel::setBottomA(glm::i8vec2 value) {
	value *= -1;
	change(this, m_data.sideN, value, &PG_DisaMapTileModel::bottomAChanged);
}

void PG_DisaMapTileModel::setBottomB(glm::i8vec2 value) {
	value *= -1;
	change(this, m_data.sideE, value, &PG_DisaMapTileModel::bottomBChanged);
}

void PG_DisaMapTileModel::setBottomC(glm::i8vec2 value) {
	value *= -1;
	change(this, m_data.sideS, value, &PG_DisaMapTileModel::bottomCChanged);
}

void PG_DisaMapTileModel::setBottomD(glm::i8vec2 value) {
	value *= -1;
	change(this, m_data.sideW, value, &PG_DisaMapTileModel::bottomDChanged);
}

void PG_DisaMapTileModel::setSideVisibility(glm::bvec4 value) {
	for(int i = 0; i < 4; ++i) setSideIVisible(i, value[i]);
}

void PG_DisaMapTileModel::clearTexture(int slotIndex) {
	if (slotIndex >= 12) return;
	if (m_data.textures[slotIndex].textureIndex == 0) return;
	m_data.textures[slotIndex].textureIndex = 0;
	emit textureIndexChanged(slotIndex);
	triggerChange(PG_DisaMapTileModel::Identity_Geometry);
}

class ItemChangedEvent : public PG_CallEvent {
	public:
		ItemChangedEvent(PG_ListModelBase* toIn, Identity ident, unsigned int loc) : PG_CallEvent(PG_Event_Behaviour_Singleton | PG_Event_Behaviour_Merge), 
			to(toIn), identity(ident), locations({loc}) {}

		void call() override {
			accept();
			if (locations.empty()) return;

			unsigned int startLoc = *locations.begin();
			unsigned int endLoc = startLoc;
			for (unsigned int loc: locations | std::views::drop(1)) {
				if (endLoc + 1 == loc) {
					endLoc = loc;
				}
				else {
					emit to->itemsChanged(startLoc, endLoc, { identity });
					startLoc = loc;
					endLoc = loc;
				}
			}
			emit to->itemsChanged(startLoc, endLoc, { identity });
		}

		void* getTo() const  override {
			return to;
		}

		bool isSame(const PG_Event* e) noexcept override {
			auto other = dynamic_cast<const ItemChangedEvent*>(e);
			return other && other->identity == other->identity;
		}

		void merge(PG_Event* e) override {
			auto other = dynamic_cast<const ItemChangedEvent*>(e);
			locations.insert(other->locations.begin(), other->locations.end());
		}

		PG_ListModelBase* to;
		const Identity identity;
		std::set<unsigned int> locations;
};


void PG_DisaMapTileModel::triggerChange(Identity identity) {
	if (auto* model = getParent<PG_DisaMapTileListModel>()) {
		model->postEvent(new ItemChangedEvent(model, identity, getLocation()));
		//emit model->itemsChanged(getLocation(), getLocation(), { identity });
	}
}

/*
* ======================== PG_DisaMapTileListModel ========================
*/

PG_ListModelLocation PG_DisaMapTileListModel::getTileLocation(glm::ivec2 pos) const {
	auto findLocIt = m_tiles.find(pos);
	if (findLocIt != m_tiles.end()) return findLocIt->second;
	return PG_ListModelLocation(const_cast<PG_DisaMapTileListModel*>(this), -1u);
}

PG_ListModelLocation PG_DisaMapTileListModel::addNewTile(glm::ivec2 pos, std::array<mpdfileTexdata, 12> textures) {
	auto location = getTileLocation(pos);
	if (location) return location;
	auto newTile = PG_new<PG_DisaMapTileModel>("PG_DisaMapTileModel", this);
	newTile->setChunk(getParent<PG_DisaMapModel>()->getChunks()->front());
	auto p = newTile->getChunk()->getTileTranslation();
	mpdfileTile data;
	std::ranges::copy(textures, data.textures);

	//std::ranges::fill(data.textures, mpdfileTexdata{0,0,0,0,0,0,0,0});
	//but why???
	/*
	constexpr int tb = 32;
	unsigned char u = tb * 7 - (pos.x * tb);
	unsigned char v = 128 - (pos.y * tb);
	data.textures[0] = mpdfileTexdata{ u, 0, 0, tb,tb,tb, 3,0};
	data.textures[1] = mpdfileTexdata{ unsigned char(256 - v), 0, 0, tb,tb,tb,3,0 };
	data.textures[2] = mpdfileTexdata{ unsigned char(256 - u), 0, 0, tb,tb,tb,3,0 };
	data.textures[3] = mpdfileTexdata{ v, 0, 0, tb,tb,tb,3,0 };
	data.textures[4] = mpdfileTexdata{ u, v, v, tb,tb,tb,1,0 }; //top
	*/


	std::ranges::fill(data.cornersTop, -8);
	data.sideN = glm::i8vec2(0);
	data.sideE = glm::i8vec2(0);
	data.sideS = glm::i8vec2(0);
	data.sideW = glm::i8vec2(0);
	data.visible = 1;
	data.x = 0;
	data.z = 0;
	data.mobility = 0;
	data.geo_color = 0;
	data.geo_mark = 0;
	newTile->setData(data);
	newTile->setTranslation(glm::vec3(pos.x, 0.0f, pos.y));
	auto t = newTile->getTranslation();
	push_back(newTile);
	location.setIndex(size()-1);
	return location;
}

PG_ListModelLocation PG_DisaMapTileListModel::setTileData(glm::ivec2 pos, const PG_Variety& data) {
	auto location = getTileLocation(pos);
	if (!location) {
		if (!data) return location;
		auto newTile = PG_new<PG_DisaMapTileModel>("PG_DisaMapTileModel", this);
		newTile->setTranslation(glm::vec3(pos.x, 0.0f, pos.y));
		push_back(newTile);
		location.setIndex(size() - 1);
	}

	if (data) {
		location.setData(data, PG_DisaMapTileModel::Identity_Geometry);
	}
	else {
		remove(location.getIndex());
	}
	return location;
}

void PG_DisaMapTileListModel::updateTileMap() {
	//kind slow but its ok
	m_tiles.clear();
	for (PG_DisaMapTileModel* item : *this) {
		m_tiles[item->getTilePosition()] = PG_ListModelLocation(this, item->getLocation());
	}
}

class TileChangedEvent : public PG_Event {
public:
	TileChangedEvent() : PG_Event(PG_Event_User + 100, PG_Event_Behaviour_Singleton | PG_Event_Behaviour_Merge) {}

	void merge(PG_Event* other) override {
		auto* e = dynamic_cast<TileChangedEvent*>(other);
		tiles.insert(e->tiles.begin(), e->tiles.end());
	}

	std::unordered_set<glm::ivec2, PG_iVec2Hasher> tiles;
};


void PG_DisaMapTileListModel::init() {
	connect(this, &PG_ListModelBase::itemsAdded, this, &PG_DisaMapTileListModel::updateTileMap);

	connect(this, &PG_ListModelBase::itemsRemoved, this, &PG_DisaMapTileListModel::updateTileMap);

	connect(this, &PG_ListModelBase::itemsChanged, this, [this](unsigned int startIndex, unsigned int endIndex, const PG_IdentitySet& identities) {
		const bool changedA = contains<PG_Identity_Location, PG_Identity_Item, PG_DisaMapTileModel::Identity_Translation>(identities);
		const bool changedB = contains<PG_DisaMapTileModel::Identity_Geometry>(identities);
		if (!changedA && !changedB || startIndex >= m_items.size()) return;
		assert(endIndex < m_items.size());

		auto* e = PG_new<TileChangedEvent>();

		for (int i = startIndex; i <= endIndex; ++i) {
			e->tiles.insert(m_items[i]->getTilePosition());
		}

		if (changedA) {
			updateTileMap();
			postEvent(e);
		}else if (changedB) {
			postEvent(e);
		}
	});
}

bool PG_DisaMapTileListModel::event(PG_Event* e) {

	if (auto* te = dynamic_cast<TileChangedEvent*>(e)) {
		emit tilesGeometryChanged(te->tiles);
		return true;
	}

	return PG_ListModel<PG_DisaMapTileModel*>::event(e);
}

/*
* ======================== PG_DisaMapModel ========================
*/

void PG_DisaMapModel::init() {
	m_chunksModel = PG_new<PG_DisaMapChunksListModel>("PG_DisaMapChunksListModel", this);
	m_tilesModel = PG_new<PG_DisaMapTileListModel>("PG_DisaMapTileListModel", this);
	m_objectsModel = PG_new<PG_DisaObjectListModel>("PG_DisaObjectListModel", this);
	m_eventsModel = PG_new<PG_DisaEventTileListModel>("PG_DisaEventTileListModel", this);
	m_actorsModel = PG_new<PG_DisaActorListModel>("PG_DisaActorListModel", this);
	m_assets = PG_new<PG_DisaAssetsModel>("PG_DisaAssetsModel", this);

	connect(m_assets, &PG_DisaAssetsModel::enabledHDViewChanged, m_tilesModel, [tilesModel = m_tilesModel] {
		emit tilesModel->itemsChanged(0, tilesModel->size() - 1, { PG_DisaMapTileModel::Identity_Geometry });
		});

	auto itemsChangedTest = [this](unsigned int, unsigned int, const PG_IdentitySet& identities) {
		for (auto i : identities) {
			if (i >= PG_Identity_Custom) {
				markModified();
				return;
			}
		}
	};

	connect(m_chunksModel, &PG_DisaMapTileListModel::sizeChanged, this, &PG_DisaMapModel::markModified);
	connect(m_chunksModel, &PG_DisaMapTileListModel::itemsChanged, this, itemsChangedTest);
	connect(m_tilesModel, &PG_DisaMapTileListModel::sizeChanged, this, &PG_DisaMapModel::markModified);
	connect(m_tilesModel, &PG_DisaMapTileListModel::itemsChanged, this, itemsChangedTest);
	connect(m_objectsModel, &PG_DisaMapTileListModel::sizeChanged, this, &PG_DisaMapModel::markModified);
	connect(m_objectsModel, &PG_DisaMapTileListModel::itemsChanged, this, itemsChangedTest);
	connect(m_eventsModel, &PG_DisaMapTileListModel::sizeChanged, this, &PG_DisaMapModel::markModified);
	connect(m_eventsModel, &PG_DisaMapTileListModel::itemsChanged, this, itemsChangedTest);
	connect(m_actorsModel, &PG_DisaMapTileListModel::sizeChanged, this, &PG_DisaMapModel::markModified);
	connect(m_actorsModel, &PG_DisaMapTileListModel::itemsChanged, this, itemsChangedTest);
	m_actorsModel->setupTeleporter(); //slot 203 keeps itself on the panel from here on
	connect(this, &PG_DisaMapModel::mapChanged, this, [this] {
		if (!m_isModified) return;
		m_isModified = false;
		emit modifiedChanged();
		});
}

void PG_DisaMapModel::clear() {
	m_jobHandle.cancel();
	m_chunksModel->clear();
	m_tilesModel->clear();
	m_objectsModel->clear();
	m_eventsModel->clear();
	m_actorsModel->clear();
	m_actorsModel->resetMapSlots();
}

struct PG_JobOpenMPD : public PG_Job {
	PG_JobOpenMPD(std::shared_ptr<PG::STREAM::In> _reader, const PG_Lifeline& _owner, bool _isJson = false) :
		reader(_reader), owner(_owner), isJson(_isJson)
	{}
	std::shared_ptr<PG::STREAM::In> reader;
	PG_Lifeline owner;
	bool isJson = false;

	void run() override {
		PG_EventLoop::start();
		openMPD();
		PG_EventLoop::stop();
	}

	void openMPD() {
		PG_MPD mpd;
		const bool mpdOpen = isJson ? mpd.openJson(reader.get()) : mpd.open(reader.get());
		reader.reset();

		if (!mpdOpen) {
			OUT_ERROR("Failed to open mpd!");
			return;
		}

		if (!mpd.getCustomMetaData().isEmpty()) {
			const auto charsInfo = mpd.getCustomMetaData()["characters"];
			for (auto& actor : mpd.getActors()) {
				if (!PG_DisaIdentifier::isCustomId(actor.id)) continue;

				if (auto info = charsInfo[std::to_string(actor.id)]; info.isValid()) {
					auto identifier = PG_DisaIdentifier(info.asString());
					actor.id = PG_DisaCharactersModel::get()->getCharacterId(identifier);
				}
				else {
					actor.id = 0;
				}

				if (actor.id == 0) {
					actor.id = 2210; //prinny
				}
			}
		}
		
		if (isCanceled()) return;

		if (mpd.getChunks().empty()) {
			owner.get()->postCallEvent([owner = dynamic_cast<PG_DisaMapModel*>(owner.get())]() {
				owner->m_chunksModel->clear();
				owner->m_tilesModel->clear();
				owner->m_objectsModel->clear();
				owner->m_eventsModel->clear();
				owner->m_actorsModel->clear();
				owner->m_actorsModel->resetMapSlots();
			});
			return;
		}


		struct LoadResult {
			std::vector<PG_DisaMapChunkModel*> chunks;
			std::vector<PG_DisaMapTileModel*> tiles;
			std::vector<PG_DisaObjectModel*> objects;
			std::vector<PG_DisaEventTileModel*> events;
			std::vector<PG_DisaActorModel*> actors;
		};
		auto* map = dynamic_cast<PG_DisaMapModel*>(owner.get());
		//load map
		LoadResult result;
		for (const mpdfileChunk& mpdChunk : mpd.getChunks()) {
			result.chunks.push_back(PG_new<PG_DisaMapChunkModel>("PG_DisaMapChunkModel"));
			result.chunks.back()->setData(mpdChunk.info.header);

			//load tiles
			for (mpdfileTile mpdTile : mpdChunk.tiles) {
				result.tiles.push_back(PG_new<PG_DisaMapTileModel>("PG_DisaMapTileModel"));
				if (mpdTile.x == mpdChunk.info.baseTile.x && mpdTile.z == mpdChunk.info.baseTile.z)
					result.tiles.back()->markAsBaseTile();

				result.tiles.back()->setChunk(result.chunks.back());
				result.tiles.back()->setData(std::move(mpdTile));
			}
			//load objects
			for (const mpdfileChunkObject& object : mpdChunk.info.objects) {
				if (object.visible == 0 && object.x == 0 && object.y == 0 && object.z == 0) continue;
				result.objects.push_back(PG_new<PG_DisaObjectModel>("PG_DisaObjectModel"));
				result.objects.back()->setChunk(result.chunks.back());
				result.objects.back()->setData(object);
			}
			//load event tiles
			for (const mpdfileChunkEventTile& tile : mpdChunk.info.eventTiles) {
				if (tile.index == 0) continue;
				result.events.push_back(PG_new<PG_DisaEventTileModel>("PG_DisaEventTileModel"));
				result.events.back()->setChunk(result.chunks.back());
				result.events.back()->setData(tile);
			}
		}

		
		for (const mpdfileActor& actor : mpd.getActors()) {
			auto* sprite = PG_new<PG_DisaSpriteModel>("PG_DisaSpriteModel");
			sprite->setData(actor);
			sprite->fixStats();
			result.actors.push_back(sprite);
		}

		PG_EventLoop::doRun();

		owner.get()->postCallEvent(
			[owner = dynamic_cast<PG_DisaMapModel*>(owner.get()), result = std::move(result)]() mutable {

				owner->m_chunksModel->set(std::move(result.chunks));
				owner->m_tilesModel->set(std::move(result.tiles));
				owner->m_objectsModel->set(std::move(result.objects));
				owner->m_eventsModel->set(std::move(result.events));
				owner->m_actorsModel->set(std::move(result.actors));
				owner->m_actorsModel->resetMapSlots();

				emit owner->mapChanged();
			});

	}
};

static inline mpdfileTexdata createTexdata(unsigned char u, unsigned char v, unsigned char height, unsigned char width, unsigned char textureIndex) {
	mpdfileTexdata texdata{
	.u = u,
	.v = v,
	.vRight = v,
    .width = width,
    .height = height,
    .heightRight = height,
    .textureIndex = textureIndex,
    .transform = 0
	};
	return texdata;
}

static inline mpdfileTile createTile(unsigned char x, unsigned char z, char height, int topIndex, int sideIndex, int decalIndex) {
	height *= -1;
	mpdfileTile tile{
		.textures = {
			createTexdata(0, 0, -height * 3, 32, sideIndex), createTexdata(0, 0, -height * 3, 32, sideIndex), createTexdata(0, 0, -height * 3, 32, sideIndex), createTexdata(0, 0, -height * 3, 32, sideIndex),
			createTexdata(x * 32, (255 - z) * 32, 32, 32, topIndex),
			createTexdata(0, 0, height, 16, 0), createTexdata(0, 0, height, 16, 0), createTexdata(0, 0, height, 16, 0), createTexdata(0, 0, height, 16, 0),
			createTexdata(x * 32, (255 - z) * 32, 32, 32, decalIndex),
			createTexdata(x * 32, (255 - z) * 32, 32, 32, 0),
			},

		.cornersTop = std::array<char, 4>{height, height, height, height},
		.sideN = {0, 0},
		.sideE = {0, 0},
		.sideS = {0, 0},
		.sideW = {0, 0},
		.visible = 1,
		.x = x,
		.z = z,
		.mobility = 0,
		.geo_color = 0,
		.geo_mark = 0,
	};
	return tile;
}

void PG_DisaMapModel::openAsset(PG_DisaFileEntry mpp, int topIndex, int sideIndex, int decalIndex) {
	getAssets()->open(mpp, {});

	/* The indices are the callers idea of a nice looking sample, an mpp holding fewer textures than
	 * that would show the fallback material instead of what it actually has. Texture slots count from
	 * one, zero is the empty slot the decal is allowed to keep. */
	if (const int textureCount = static_cast<int>(getAssets()->getTextures()->size()); textureCount > 0) {
		topIndex = glm::clamp(topIndex, 1, textureCount);
		sideIndex = glm::clamp(sideIndex, 1, textureCount);
		decalIndex = glm::clamp(decalIndex, 0, textureCount);
	}

	createBaseMap(topIndex, sideIndex, decalIndex);
}

void PG_DisaMapModel::createBaseMap(int topIndex, int sideIndex, int decalIndex ) {
	//create simple display map

	std::vector<PG_DisaMapChunkModel*> chunks;
	std::vector<PG_DisaMapTileModel*> tiles;

	chunks.push_back(PG_new<PG_DisaMapChunkModel>("PG_DisaMapChunkModel", m_chunksModel));
	{
		mpdfileChunkHeader chunk{
			.map_offset_x = 6.f,
			.map_offset_y = 0.f,
			.map_offset_z = 6.f,
			.xRot = 0.f,
			.yRot = 0.f,
			.zRot = 0.f,
			.number_of_tiles = 3,
			.index = 0,
		};

		chunks.back()->setData(chunk);
	}
	{
		tiles.push_back(PG_new<PG_DisaMapTileModel>("PG_DisaMapTileModel", m_tilesModel));
		tiles.back()->setChunk(chunks.back());
		tiles.back()->setData(createTile(15, 14, 30, topIndex, sideIndex, decalIndex));

		tiles.push_back(PG_new<PG_DisaMapTileModel>("PG_DisaMapTileModel", m_tilesModel));
		tiles.back()->setChunk(chunks.back());
		tiles.back()->setData(createTile(15, 15, 24, topIndex, sideIndex, decalIndex));

		tiles.push_back(PG_new<PG_DisaMapTileModel>("PG_DisaMapTileModel", m_tilesModel));
		tiles.back()->setChunk(chunks.back());
		tiles.back()->setData(createTile(14, 15, 12, topIndex, sideIndex, decalIndex));

		tiles.push_back(PG_new<PG_DisaMapTileModel>("PG_DisaMapTileModel", m_tilesModel));
		tiles.back()->setChunk(chunks.back());
		tiles.back()->setData(createTile(14, 14, 12, topIndex, sideIndex, decalIndex));
	}

	m_chunksModel->set(std::move(chunks));
	m_tilesModel->set(std::move(tiles));
	emit mapChanged();
}

void PG_DisaMapModel::openMapAssets(PG_DisaFileEntry mppSDInfo, PG_DisaFileEntry mppHDInfo) {
	getAssets()->open(mppSDInfo, mppHDInfo);
}

void PG_DisaMapModel::openMap(std::shared_ptr<PG::STREAM::In> reader) {
	clear();
	m_jobHandle = PG_Worker::get()->addJob(new PG_JobOpenMPD(reader, PG_Lifeline(this)));
}

bool PG_DisaMapModel::open(PG_DisaFileEntry mpd, PG_DisaFileEntry mppSD, PG_DisaFileEntry mppHD) {
	auto reader = mpd.createReader();
	if (!reader) {
		OUT_ERROR("Failed to read \"" << mpd.getName() << "\" as a map!");
		return FAILURE;
	}

	openMap(reader);
	openMapAssets(mppSD, mppHD);
	return SUCCESS;
}

struct PG_JobOpenGEO : public PG_Job {
	PG_JobOpenGEO(const std::shared_ptr<PG::STREAM::In>& _geoStream, const PG_Lifeline& _owner) :
		geoStream(_geoStream),  owner(_owner)
	{}
	std::shared_ptr<PG::STREAM::In> geoStream;
	PG_Lifeline owner;

	void run() override {
		PG_EventLoop::start();
		openGEO();
		PG_EventLoop::stop();
	}

	void openGEO() {
		return; //TODO!
		assert(geoStream);

		PG_GEO geo;
		if (!geo.open(geoStream.get())) {
			geoStream.reset();
			return;
		}
		geoStream.reset();

		auto disaModel = PG_DisaModel();
		disaModel.load("GeoModel", geo);

		auto objectModel = PG_new<PG_DisaObjectModel>("PG_DisaObjectModel");
		mpdfileChunkObject data{
			.x = 0, .y = 0, .z = 0, 
			.xRot = 0, .yRot = 0, .zRot = 0, 
			.xScale = 100, .yScale = 100, .zScale = 100,
			.unk10 = 0,
			.unk11 = 0, .model_id = 0, .visible = 1, .unk12 = 0,
			.textureScrollX = 0, .textureScrollY = 0
		};
		objectModel->setData(data);

		if (isCanceled()) return;
		owner.get()->postCallEvent(
			[owner = dynamic_cast<PG_DisaMapModel*>(owner.get()), objectModel, geo = std::move(disaModel)]() mutable {
				objectModel->setThreadId(owner->getThreadId());
				//owner->m_assets->addGeometry({ std::move(geo) });
				owner->m_objectsModel->push_back(objectModel);
			});
	}
};

void PG_DisaMapModel::openGeo(PG_DisaFileEntry geoFile) {
	clear();

	auto geoStream = geoFile.createReader();
	if (!geoStream) {
		return;
	}

	m_jobHandle = PG_Worker::get()->addJob(new PG_JobOpenGEO(geoStream, PG_Lifeline(this)));
}

PG_MPD PG_DisaMapModel::toMPD() const {
	PG_MPD mpd;
	//save chunks
	std::vector<mpdfileChunk> chunks;
	struct Tracker {
		bool baseSet = false;
		int objectCounter = 0;
		int eventCounter = 0;
	};
	std::vector<Tracker> trackers;
	trackers.resize(getChunks()->size());
	mpd.getHeader().number_of_chunks = getChunks()->size();

	for (auto* chunk : *getChunks()) {
		chunks.emplace_back(mpdfileChunkDef{ chunk->getData() });
		chunks.back().info.header.number_of_tiles = 0;
	}

	for (auto* tile : *getTiles()) {
		auto& c = chunks[tile->getChunk()->getLocation()];
		c.tiles.push_back(tile->getData());
		++c.info.header.number_of_tiles;
		if (tile->isBaseTile()) {
			c.info.baseTile = tile->getData();
		}
	}

	for (auto* item : *getObjects()) {
		auto& c = chunks[item->getChunk()->getLocation()];
		int& i = trackers[c.info.header.index].objectCounter;
		if (i >= c.info.objects.size()) continue;
		c.info.objects[i] = item->getData();
		++i;
	}

	for (auto* item : *getEvents()) {
		auto& c = chunks[item->getChunk()->getLocation()];
		int& i = trackers[c.info.header.index].eventCounter;
		if (i >= c.info.eventTiles.size()) continue;
		c.info.eventTiles[i] = item->getData();
		++i;
	}

	{ //fill remaining with 0
		auto it =chunks.begin();
		for (auto track : trackers) {
			if (int size = (it->info.objects.size() - track.objectCounter); size > 0 ) {
				memset(&it->info.objects[track.objectCounter], 0, sizeof(mpdfileChunkObject)* size);
			}
			if (int size = (it->info.eventTiles.size() - track.eventCounter); size > 0) {
				memset(&it->info.eventTiles[track.eventCounter], 0, sizeof(mpdfileChunkEventTile) * size);
			}
			++it;
		}
	}

	mpd.getChunks() = std::move(chunks);

	//save actors
	mpd.getActors().reserve(getActors()->size());

	/* Slots 0 to 63 in order are what the file placed, which is exactly what the game's own actor
	 * array holds. A script's actors sit at 64 and up and the teleporter at 203, so walking the
	 * slots rather than the rows keeps them out of the file. */
	for (int slot = PG_DisaActorListModel::firstMapSlot; slot <= PG_DisaActorListModel::lastMapSlot; ++slot) {
		auto* actor = dynamic_cast<PG_DisaSpriteModel*>(getActors()->getSlotItem(slot));
		if (!actor) continue;
		mpd.getActors().push_back(actor->getData());
		//store meta data to be able to remap the ids
		if (const PG_DisaIdentifier& customId = actor->getCharacterIdentifier(); customId.isCustom()) {
			auto charMeta = mpd.getCustomMetaData()["characters"];
			if(auto info = charMeta[std::to_string(actor->getCharacterId())]; !info.isSet()){
				info = customId.getIdentifier();
			}
		}
	}
	//counted after the loop, because the list may hold items that are not the map's own
	mpd.getHeader().number_of_actors = static_cast<unsigned short>(mpd.getActors().size());

	mpd.getHeader().field_0x4 = 0;
	mpd.getHeader().field_0x6 = 0;
	mpd.getHeader().field_0x8 = 0;
	mpd.getHeader().field_0xA = 0;
	mpd.getHeader().field_0xC = 0;
	mpd.getHeader().field_0xE = 0;
	return mpd;
}

#include <Disa/fileformats/PG_MPP.h>

void PG_DisaMapModel::saveMPD(const PG_File& mpdFile) {
	auto mpd = toMPD();
	mpd.save(mpdFile);

	auto file = mpdFile;
	file.replaceFileExtension("mpp");
	auto fileEntry = getAssets()->getContainerSD()->getParentEntry();
	fileEntry.saveTo(file);

	file.replaceFileName("new_"+ file.getName());
	fileEntry = getAssets()->getContainerHD()->getParentEntry();
	fileEntry.saveTo(file);
}


std::shared_ptr<PG::STREAM::In> PG_DisaMPDContainer::createReader() const {
	if (m_model /* && m_model->isModified()*/) {
		auto writer = std::make_shared<PG::STREAM::InOutByteVector>();
		PG_MPD mpd = m_model->toMPD();
		mpd.save(writer.get());
		writer->seek(0);
		return writer;
	}
	else if (auto entity = getParentEntry()) {
		return entity.createReaderOriginal();
	}
	return nullptr;
}

bool PG_DisaMPDContainer::saveFile() {
	if (!PG_DisaContainerInterface::save()) return FAILURE;
	if (m_model && m_model->getAssets()) {
		m_model->getAssets()->clearModified();
		emit isModifiedChanged();
	}
	return SUCCESS;
}