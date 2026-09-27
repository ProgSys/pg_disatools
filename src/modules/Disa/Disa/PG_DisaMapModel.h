#pragma once

#include <Scene/Model/PG_ListSelectionInteractableModel.h>
#include <Disa/fileformats/PG_MPD.h>
#include <Disa/fileformats/PG_DisaFileInfo.h>
#include <Disa/map/PG_DisaMapChunkModel.h>
#include <Disa/map/PG_DisaObjectModel.h>
#include <Disa/PG_DisaColorEnum.h>
#include <resources/PG_Texture.h>
#include <Resources/PG_Model.h>

#include <Util/PG_iVec2Hasher.h>
#include <Util/PG_Transformation.h>
#include <optional>
#include <unordered_set>

#include <Base/Thread/PG_Worker.h>

class PG_DisaMapTileModel;
class PG_DisaEventTileListModel;
class PG_DisaActorListModel;
class PG_DisaAssetsModel;
class PG_DisaMPDContainer;


using PG_DisaUVOutlineType = std::array<glm::vec2, 4>;

class PG_DisaMapTileMaterialModel : public PG_Object {
	PG_OBJECT(PG_DisaMapTileMaterialModel);
	PG_PARENT(PG_Object);
	PG_PROPERTY(int index GET getIndex)

	PG_PROPERTY(unsigned char textureIndex GET getTexture CHANGED textureIndexChanged)
	PG_PROPERTY(PG_Texture texture GET getTexture CHANGED textureIndexChanged)

	PG_PROPERTY(unsigned char texU GET getU SET setU CHANGED uChanged)
	PG_PROPERTY(unsigned char texV GET getV SET setV CHANGED vChanged)
	PG_PROPERTY(unsigned char texVRight GET getVRight SET setVRight CHANGED vRightChanged)

	PG_PROPERTY(unsigned char texWidth GET getWidth SET setWidth CHANGED widthChanged)
	PG_PROPERTY(unsigned char texHeight GET getHeight SET setHeight CHANGED heightChanged)
	PG_PROPERTY(unsigned char texHeightRight GET getHeightRight SET setHeightRight CHANGED heightRightChanged)
	PG_PROPERTY(PG_DisaUVOutlineType texUvOutline GET getUVOutline CHANGED uvChanged)

	PG_PROPERTY(int texRotation GET getRotation SET setRotation CHANGED rotationChanged)
	PG_PROPERTY(bool texMirrorVertical GET isMirrorVertical SET setMirrorVertical CHANGED mirrorVerticalChanged)
	PG_PROPERTY(bool texMirrorHorizontal GET isMirrorHorizontal SET setMirrorHorizontal CHANGED mirrorHorizontalChanged)
	PG_PROPERTY(bool texIsShadow GET isShadow SET setIsShadow CHANGED isShadowChanged)
signals:
	void textureIndexChanged();
	void uChanged();
	void vChanged();
	void vRightChanged();

	void widthChanged();
	void heightChanged();
	void heightRightChanged();

	void rotationChanged();
	void mirrorVerticalChanged();
	void mirrorHorizontalChanged();
	void isShadowChanged();

	void uvChanged();
public:
	using PG_Object::PG_Object;

	enum ELayer {
		Base,
		Decal,
		Shadow
	};

	PG_DisaMapTileModel* getTile() const { return m_tile; }
	void setTileModel(PG_DisaMapTileModel* model);

	int getIndex() const { return m_index; }
	void setIndex(int index) { m_index = index; }

	int getSortIndex() const { return m_sortIndex; }
	void setSortIndex(int index) { m_sortIndex = index; }

	unsigned char getTextureIndex() const { return getData().textureIndex; }
	void setTextureIndex(unsigned char index);
	PG_Texture getTexture() const;

	unsigned char getU() const { return getData().u; }
	void setU(unsigned char value);

	unsigned char getV() const { return getData().v; }
	void setV(unsigned char value);

	unsigned char getVRight() const { return getData().vRight; }
	void setVRight(unsigned char value);

	unsigned char getWidth() const { return getData().width; }
	void setWidth(unsigned char value);

	unsigned char getHeight() const { return getData().height; }
	void setHeight(unsigned char value);

	unsigned char getHeightRight() const { return getData().heightRight; }
	void setHeightRight(unsigned char value);

	int getRotation() const { return getData().transform & 0b011; }
	void setRotation(int value);

	bool isMirrorVertical() const { return getData().transform & 0b100; }
	void setMirrorVertical(bool value);

	bool isMirrorHorizontal() const { return getData().transform & 0b1000; }
	void setMirrorHorizontal(bool value);

	bool isShadow() const { return getData().transform & 0b110000; }
	void setIsShadow(bool value);

	std::array<glm::vec2, 4> getUVOutline() const;
receivers:
	void clearTexture() { setTextureIndex(0); }
private:
	mpdfileTexdata& getData() const;
	PG_DisaMapTileModel* m_tile = nullptr;
	int m_index = 0;
	int m_sortIndex = 0;
};


class PG_DisaMapTileModel : public PG_Object {
	PG_OBJECT(PG_DisaMapTileModel);
	PG_PARENT(PG_Object);

	PG_PROPERTY(int mobility GET getMobility SET setMobility CHANGED mobilityChanged)
	PG_PROPERTY(int geoColor GET getGeoColor SET setGeoColor CHANGED geoColorChanged)
	PG_PROPERTY(bool isBaseTile GET isBaseTile SET markAsBaseTileProp CHANGED isBaseTileChanged)
	PG_PROPERTY(bool isStartingPoint GET isStartingPoint SET markAsStartingPoint CHANGED isStartingPointChanged) //the teleporter

	PG_PROPERTY(char topA GET getTopA SET setTopA CHANGED topAChanged)
	PG_PROPERTY(char topB GET getTopB SET setTopB CHANGED topBChanged)
	PG_PROPERTY(char topC GET getTopC SET setTopC CHANGED topCChanged)
	PG_PROPERTY(char topD GET getTopD SET setTopD CHANGED topDChanged)

	PG_PROPERTY(glm::i8vec2 bottomA GET getBottomA SET setBottomA CHANGED bottomAChanged)
	PG_PROPERTY(glm::i8vec2 bottomB GET getBottomB SET setBottomB CHANGED bottomBChanged)
	PG_PROPERTY(glm::i8vec2 bottomC GET getBottomC SET setBottomC CHANGED bottomCChanged)
	PG_PROPERTY(glm::i8vec2 bottomD GET getBottomD SET setBottomD CHANGED bottomDChanged)

	PG_PROPERTY(glm::bvec4 sideVisibility GET getSideVisibility SET setSideVisibility CHANGED sideVisibilityChanged)
signals:
	void mobilityChanged();
	void geoColorChanged();
	void isBaseTileChanged();
	void isStartingPointChanged();

	void topAChanged();
	void topBChanged();
	void topCChanged();
	void topDChanged();

	void bottomAChanged();
	void bottomBChanged();
	void bottomCChanged();
	void bottomDChanged();

	void sideVisibilityChanged();

	void textureIndexChanged(int slotIndex);
public:
	friend PG_DisaMapTileMaterialModel;

	enum ECustomIdentity : Identity {
		Identity_Geometry = EIdentity::PG_Identity_Custom, //returns m_data
		Identity_Translation,
		Identity_GeoColor,
		Identity_TopHeight, //avarage height
		Identity_MaxHeight, //max height
		Identity_RefHeight, //reference corner height used by objects
	};
	using PG_Object::PG_Object;

	int getLocation() const { return m_location; }
	void setLocation(int index) { m_location = index; }

	glm::vec3 getTranslationLocal() const { return glm::vec3(-m_data.x, 0.0f, m_data.z); }
	void setTranslationLocal(const glm::vec3& local) { m_data.x = -std::floor(local.x);  m_data.z = std::floor(local.z); }

	glm::vec3 getTranslation() const {
		return m_chunk->getTileTranslation() + getTranslationLocal();
	}
	void setTranslation(const glm::vec3& offset) { setTranslationLocal(offset - m_chunk->getTileTranslation()); }

	glm::ivec2 getTilePosition() const {
		auto t = getTranslation();
		return glm::ivec2(std::floor(t.x), std::floor(t.z));
	}

	bool isBaseTile() const { return m_isBaseTile; }
	void markAsBaseTile();
	void markAsBaseTileProp(bool value) { if (value) markAsBaseTile(); } //so PG_PROPERTY does work

	bool isStartingPoint() const;
	void markAsStartingPoint();
	void markAsStartingPoint(bool value) { if (value) markAsStartingPoint(); } //so PG_PROPERTY does work

	const mpdfileTile& getData() const { return m_data; }
	void setData(const mpdfileTile& data) { m_data = data; }
	void setData(mpdfileTile&& data) { m_data = std::move(data); }

	PG_DisaMapChunkModel* getChunk() const { return m_chunk.get(); }
	void setChunk(PG_DisaMapChunkModel* model);

	bool hasTop() const;
	bool hasSideN() const;
	bool hasSideE() const;
	bool hasSideS() const;
	bool hasSideW() const;

	void setSideIVisible(int index, bool value); //N, E, S, W
	void setSideNVisible(bool value) { setSideIVisible(0, value); }
	void setSideEVisible(bool value) { setSideIVisible(1, value); }
	void setSideSVisible(bool value) { setSideIVisible(2, value); }
	void setSideWVisible(bool value) { setSideIVisible(3, value); }
	glm::bvec4 getSideVisibility() const {
		//N, E, S, W,
		return glm::bvec4(m_data.textures[0].textureIndex != 0, m_data.textures[1].textureIndex != 0,
			m_data.textures[2].textureIndex != 0, m_data.textures[3].textureIndex != 0);
	}
	void setSideVisibility(glm::bvec4 value);


	static int getTopTextureIndex(const mpdfileTile& data);
	static int getSideNTextureIndex(const mpdfileTile& data);
	static int getSideETextureIndex(const mpdfileTile& data);
	static int getSideSTextureIndex(const mpdfileTile& data);
	static int getSideWTextureIndex(const mpdfileTile& data);

	inline int getTopTextureIndex() const { return getTopTextureIndex(m_data); }
	inline int getSideNTextureIndex() const { return getSideNTextureIndex(m_data); }
	inline int getSideETextureIndex() const { return getSideETextureIndex(m_data); }
	inline int getSideSTextureIndex() const { return getSideSTextureIndex(m_data); }
	inline int getSideWTextureIndex() const { return getSideWTextureIndex(m_data); }
	/**
	* @returns the top 4 points
	* Order:
	*       S
	*    2-----3
	* E  |     |  W
	* z  |     |
	* ^  0-----1
	* |     N
	*  -> x
	*/
	static std::array<glm::vec3, 4> getTopData(const mpdfileTile& data, const glm::vec3& offset);
	static std::array<glm::vec3, 4> getSideNData(const mpdfileTile& data, const glm::vec3& offset);
	static std::array<glm::vec3, 4> getSideEData(const mpdfileTile& data, const glm::vec3& offset);
	static std::array<glm::vec3, 4> getSideSData(const mpdfileTile& data, const glm::vec3& offset);
	static std::array<glm::vec3, 4> getSideWData(const mpdfileTile& data, const glm::vec3& offset);

	inline std::array<glm::vec3, 4> getTop(const glm::vec3& offset) const { return getTopData(m_data, offset); }
	inline std::array<glm::vec3, 4> getSideN(const glm::vec3& offset) const { return getSideNData(m_data, offset); }
	inline std::array<glm::vec3, 4> getSideE(const glm::vec3& offset) const { return getSideEData(m_data, offset); }
	inline std::array<glm::vec3, 4> getSideS(const glm::vec3& offset) const { return getSideSData(m_data, offset); }
	inline std::array<glm::vec3, 4> getSideW(const glm::vec3& offset) const { return getSideWData(m_data, offset); }

	static std::array<glm::vec3, 4> getTopCustom(const glm::vec3& offset, std::array<char, 4> cornersTop);

	std::array<float, 4> getTopValues() const;
	void setTopValues(const std::array<float, 4>& values);
	static char roundHeight(float value);
	/**
	* @brief Avarage height of the tile
	*/
	float getTopHeight() const;
	/**
	* @brief Max height of the tile
	*/
	float getMaxHeight() const;
	/**
	* @brief Reference height of the tile used by objects
	*/
	float getRefHeight() const;


	//settings
	enum EMobility : unsigned char {
		Unlimited,
		FlyOnly,
		Impassable
	};
	EMobility getMobility() const { return static_cast<EMobility>(m_data.mobility); }
	void setMobility(EMobility mobility) {
		if (m_data.mobility == mobility) return;
		m_data.mobility = mobility;
		emit mobilityChanged();
	}
	void setMobility(int mobility) { setMobility(static_cast<EMobility>(mobility)); }


	bool hasGeoColor() const { return static_cast<bool>(m_data.geo_mark); }
	EDisaGeoColor getGeoColor() const { return m_data.geo_mark ? static_cast<EDisaGeoColor>(m_data.geo_color + 1) : DisaGeoColor_None; }
	void setGeoColor(EDisaGeoColor color);
	void setGeoColor(int color) { setGeoColor(static_cast<EDisaGeoColor>(color)); }

	char getTopA() const { return -m_data.cornersTop[0]; }
	void setTopA(char value);

	char getTopB() const { return -m_data.cornersTop[1]; }
	void setTopB(char value);

	char getTopC() const { return -m_data.cornersTop[2]; }
	void setTopC(char value);

	char getTopD() const { return -m_data.cornersTop[3]; }
	void setTopD(char value);

	glm::i8vec2 getBottomA() const { return -m_data.sideN; }
	void setBottomA(glm::i8vec2 value);

	glm::i8vec2 getBottomB() const { return -m_data.sideE; }
	void setBottomB(glm::i8vec2 value);

	glm::i8vec2 getBottomC() const { return -m_data.sideS; }
	void setBottomC(glm::i8vec2 value);

	glm::i8vec2 getBottomD() const { return -m_data.sideW; }
	void setBottomD(glm::i8vec2 value);
	/**
	* @brief Sets the texture index to zero (empty)
	*/
	void clearTexture(int slotIndex);

	PG_Variety getData(Identity identity) const;
	bool setData(const PG_Variety& value, Identity identity);

	void triggerChange(Identity identity);
private:
	bool m_isBaseTile = false;
	PG_Lifeline_T<PG_DisaMapChunkModel> m_chunk;
	int m_location = 0;
	mpdfileTile m_data;
};

struct PG_ListModelLocationHasher {
	inline std::size_t operator()(const PG_ListModelLocation& v) const noexcept {
		return std::hash<int>()(v.getIndex());
	}
};

class PG_DisaMapTileListModel : public PG_ListModel<PG_DisaMapTileModel*> {
	PG_OBJECT(PG_DisaMapTileListModel);
	PG_PARENT(PG_ListModel<PG_DisaMapTileModel*>);
signals:
	void tilesGeometryChanged(const std::unordered_set<glm::ivec2, PG_iVec2Hasher>& tiles); //the geometery of some tile has changed, will be emited on event loop
public:
	using PG_ListModel<PG_DisaMapTileModel*>::PG_ListModel;

	void init();
	bool event(PG_Event* e) override;

	/*!
	* @brief Will return tile location for given 2D position
	* Can be invalid if there is not tile at that position
	*/
	PG_ListModelLocation getTileLocation(glm::ivec2 pos) const;

	/*!
	* @brief Add a new tile at given location
	* @param textures Texture for each tile face
	*/
	PG_ListModelLocation addNewTile(glm::ivec2 pos, std::array<mpdfileTexdata, 12> textures);

	PG_ListModelLocation setTileData(glm::ivec2 pos, const PG_Variety& data);

private:
	void updateTileMap();
	std::unordered_map<glm::ivec2, PG_ListModelLocation, PG_iVec2Hasher> m_tiles;
};


/**
* @brief Data representation of a disa map file
* Contains tiles, objects and actors (monsters)
*/
class PG_DisaMapModel : public PG_Object {
	PG_OBJECT(PG_DisaMapModel);
signals:
	void mapChanged(); //all map models have changed
	void modifiedChanged();
public:
	friend class PG_JobOpenMPD;
	friend class PG_JobOpenGEO;

	using PG_Object::PG_Object;
	void init();
	void clear();
	/**
	* @brief Opens a map, its geometry out of the mpd and its textures out of the mpp files
	* @param mppSD May be invalid, the map then renders on the fallback tile material
	* @param mppHD May be invalid, HD mode then has nothing to show
	* @details The geometry is read on a worker, mapChanged() tells when it arrived. The entries read
	* through their parent container, so that container has to outlive the map.
	* @returns false when the mpd gives no reader
	*/
	bool open(PG_DisaFileEntry mpd, PG_DisaFileEntry mppSD, PG_DisaFileEntry mppHD);
	void openMap(std::shared_ptr<PG::STREAM::In> reader);
	void openAsset(PG_DisaFileEntry mpp, int topIndex = 1, int sideIndex = 2, int decalIndex = 0);
	void openMapAssets(PG_DisaFileEntry mppSDInfo, PG_DisaFileEntry mppHDInfo);

	void openGeo(PG_DisaFileEntry geoFile);
	/**
	* @brief Shows a finished model as the only object of a map with no tiles
	* @details The same as openGeo, only there is nothing to read, so the object is there right away.
	*/
	void openModel(PG_Model model);

	void createBaseMap(int topIndex = 1, int sideIndex = 2, int decalIndex = 0);

	PG_MPD toMPD() const;
	void saveMPD(const PG_File& mpdFile);

	bool isModified() const { return m_isModified; }
	void markModified() { 
		if (!m_isModified) {
			m_isModified = true; 
			emit modifiedChanged();
		}
	}

	PG_DisaMapChunksListModel* getChunks() const { return m_chunksModel; }
	PG_DisaMapTileListModel* getTiles() const { return m_tilesModel; }
	PG_DisaObjectListModel* getObjects() const { return m_objectsModel; }
	PG_DisaEventTileListModel* getEvents() const { return m_eventsModel; }
	PG_DisaActorListModel* getActors() const { return m_actorsModel; }
	PG_DisaAssetsModel* getAssets() const { return m_assets; }

	/**
	* @brief The resource namespace mod objects are looked up in, the one of the campaign the map belongs to.
	* @details Empty outside a campaign. Part of where the map is opened from, not of the map, so clear() keeps it.
	*/
	const std::string& getResourceNameSpace() const { return m_resourceNameSpace; }
	void setResourceNameSpace(std::string nameSpace);
	/// The name a mod object's resource is requested under, its name joined with getResourceNameSpace()
	std::string getResourceName(const PG_DisaObjectModel& object) const;

private:
	/// Puts the model into the assets and hands the object the index it landed on
	void addPreviewObject(PG_DisaObjectModel* object, PG_Model model);

	PG_JobHandle m_jobHandle;
	bool m_isModified = false;

	PG_DisaMapChunksListModel* m_chunksModel = nullptr;
	PG_DisaMapTileListModel* m_tilesModel = nullptr;
	PG_DisaObjectListModel* m_objectsModel = nullptr;
	PG_DisaEventTileListModel* m_eventsModel = nullptr;
	PG_DisaActorListModel* m_actorsModel = nullptr;

	//cashed resources
	PG_DisaAssetsModel* m_assets = nullptr;
	std::string m_resourceNameSpace;
};

#include <Disa/fileformats/PG_DisaFileContainer.h>


class PG_DisaMPDContainer : public PG_DisaContainerInterface, public PG_Object {
	PG_OBJECT(PG_DisaMPDContainer);

	PG_PROPERTY(bool isModified GET isModified CHANGED isModifiedChanged);
signals:
	void isModifiedChanged();
public:
	using PG_Object::PG_Object;
	~PG_DisaMPDContainer() override {
		delete m_model;
		m_model = nullptr;
	}

	bool open(std::shared_ptr<PG::STREAM::In> reader) override {
		createModel()->openMap(reader);
		return SUCCESS;
	};

	bool refresh(std::shared_ptr<PG::STREAM::In> reader) override { return SUCCESS; };

	bool save(std::shared_ptr<PG::STREAM::Out> writer) const override {
		if (!m_model) return FAILURE;
		PG_MPD mpd = m_model->toMPD();
		return mpd.save(writer.get());
	};

	std::shared_ptr<PG::STREAM::In> createReader() const override;
	std::shared_ptr<PG::STREAM::In> createReaderOriginal() const override {
		if (auto entity = getParentEntry()) {
			return entity.createReaderOriginal();
		}
		return nullptr;
	}

	bool isModified() const override { return m_model && m_model->isModified(); }
	PG_DisaMapModel* getModel() const { return m_model; }
	PG_DisaMapModel* createModel() { 
		if (!m_model) {
			m_model = PG_new<PG_DisaMapModel>(this);
			connect(m_model, &PG_DisaMapModel::modifiedChanged, this, &PG_DisaMPDContainer::isModifiedChanged);
		}
		return m_model;
	}

	/**
	* @brief Where the camera should stand when a scene opens this map
	* @returns Empty when nobody picked a spot, the scene then keeps the camera it has
	*/
	const std::optional<PG_Transformation>& getCameraStartTransformation() const { return m_cameraStartTransform; }
	void setCameraStartTransformation(const PG_Transformation& transform) { m_cameraStartTransform = transform; }

	/**
	* @brief Opens a map, its geometry out of the mpd and its textures out of the mpp files
	* @param mppSD May be invalid, the map then renders on the fallback tile material
	* @param mppHD May be invalid, HD mode then has nothing to show
	* @tparam T The container to create, so an owner can keep its own derived type. An entry can only be
	* open as one type at a time, so everybody opening the same mpd has to name the same one.
	* @returns Null when the mpd has no parent, or is already open as a container that is not a T
	*/
	template<typename T = PG_DisaMPDContainer>
	static std::shared_ptr<T> createFromMPD(PG_DisaFileEntry mpd, PG_DisaFileEntry mppSD, PG_DisaFileEntry mppHD) {
		static_assert(std::is_base_of_v<PG_DisaMPDContainer, T>, "T must derived from PG_DisaMPDContainer!");
		auto container = mpd.open<T>();
		if (!container) return nullptr;
		container->createModel()->openMapAssets(mppSD, mppHD);
		return container;
	}
	/**
	* @brief Opens only the textures of an mpp and puts them on a few sample tiles
	* @details There is no mpd here, so the tiles come from PG_DisaMapModel::createBaseMap.
	*/
	template<typename T = PG_DisaMPDContainer>
	static std::shared_ptr<T> createFromMPP(PG_DisaFileEntry mpp, int topIndex = 1, int sideIndex = 2, int decalIndex = 0) {
		static_assert(std::is_base_of_v<PG_DisaMPDContainer, T>, "T must derived from PG_DisaMPDContainer!");
		auto container = std::shared_ptr<T>(PG_new<T>());
		container->createModel()->openAsset(mpp, topIndex, sideIndex, decalIndex);
		return container;
	}
	/**
	* @brief Opens a single geometry as a map that holds nothing but that one object
	* @details There are no tiles here, the geo goes into the object list and the camera frames it. The
	* geo is read on a worker, so the container comes back before there is anything to see.
	*/
	template<typename T = PG_DisaMPDContainer>
	static std::shared_ptr<T> createFromGEO(PG_DisaFileEntry geo) {
		static_assert(std::is_base_of_v<PG_DisaMPDContainer, T>, "T must derived from PG_DisaMPDContainer!");
		auto container = std::shared_ptr<T>(PG_new<T>());
		container->createModel()->openGeo(geo);
		return container;
	}
	/// @see PG_DisaMapModel::openModel
	template<typename T = PG_DisaMPDContainer>
	static std::shared_ptr<T> createFromModel(PG_Model model) {
		static_assert(std::is_base_of_v<PG_DisaMPDContainer, T>, "T must derived from PG_DisaMPDContainer!");
		auto container = std::shared_ptr<T>(PG_new<T>());
		container->createModel()->openModel(std::move(model));
		return container;
	}
receivers:
	//a bit ugly
	bool saveFile();
private:
	PG_DisaMapModel* m_model = nullptr;
	std::optional<PG_Transformation> m_cameraStartTransform;
};