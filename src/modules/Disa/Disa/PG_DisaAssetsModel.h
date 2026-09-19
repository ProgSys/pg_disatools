#pragma once

#include <Base/PG_Object.h>
#include <Resources/PG_Model.h>
#include <Resources/PG_Texture.h>
#include <Scene/Model/PG_ListModel.h>
#include <Disa/fileformats/PG_DisaFileInfo.h>

class PG_MPPContainer;
class PG_GEO;
class PG_TX2;

/**
* @brief Holds texture assets
*/
struct PG_DisaAssetsTextureItem {
	enum EDisaAssetsTextureItemIdentity : Identity {
		Identity_SD = EIdentity::PG_Identity_Custom,
		Identity_SDModified,
		Identity_HD,
		Identity_HDModified,
		Identity_HDNormal,
		Identity_HDNormalModified,
	};

	constexpr static std::array<Identity, 4> allHDIdentities = { Identity_HD , Identity_HDNormal , Identity_HDModified , Identity_HDNormalModified };

	PG_ListModelLocation sdLoc;
	PG_ListModelLocation hdLoc;
	PG_ListModelLocation hdNormalLoc;

	mutable PG_Texture sd;
	mutable PG_Texture hd;
	mutable PG_Texture hdNormal;

	mutable bool sdChanged = true;
	mutable bool hdChanged = true;
	mutable bool hdNormalChanged = true;

	PG_Variety getData(Identity identity) const;

	const PG_Texture& getSDTexture() const;
	const PG_Texture& getHDTexture() const;
	const PG_Texture& getHDNormalTexture() const;

	bool containsSD(int index) const{
		return sdLoc.getIndex() == index;
	}
	bool containsHD(int index) const {
		return hdLoc.getIndex() == index || hdNormalLoc.getIndex() == index;
	}

	Identity getHDIdentityOf(int index) const {
		if (hdLoc.getIndex() == index) return Identity_HD;
		return Identity_HDNormal;
	}

	void markChange(const std::set<Identity>& identities) {
		sdChanged |= identities.contains(Identity_SDModified);
		hdChanged |= identities.contains(Identity_HDModified);
		hdNormalChanged |= identities.contains(Identity_HDNormalModified);
	}
};

/**
* @brief Holds geometry assets
*/
struct PG_DisaAssetsGeometryItem {
	enum EDisaAssetsTextureItemIdentity : Identity {
		Identity_SD = EIdentity::PG_Identity_Custom,
		Identity_SDModified,
		Identity_HD,
		Identity_HDModified
	};

	constexpr static std::array<Identity, 4> allHDIdentities = { Identity_HD  , Identity_HDModified  };

	PG_ListModelLocation sdLoc;
	PG_ListModelLocation hdLoc;

	mutable PG_Model sd;
	mutable PG_Model hd;

	mutable bool sdChanged = true;
	mutable bool hdChanged = true;

	PG_Variety getData(Identity identity) const;

	const PG_Model& getSDModel() const;
	const PG_Model& getHDModel() const;

	bool containsSD(int index) const {
		return sdLoc.getIndex() == index;
	}
	bool containsHD(int index) const {
		return hdLoc.getIndex() == index;
	}

	Identity getHDIdentityOf(int index) const {
		return Identity_HD;
	}

	void markChange(const std::set<Identity>& identities) {
		sdChanged |= identities.contains(Identity_SDModified);
		hdChanged |= identities.contains(Identity_HDModified);
	}
};

template<typename T>
concept PG_IsDisaAsset = std::is_same_v<T, PG_DisaAssetsTextureItem> || std::is_same_v<T, PG_DisaAssetsGeometryItem>;

template<PG_IsDisaAsset T>
class PG_DisaAssetsFilterModel : public PG_ListModelBase {
public:
	using PG_ListModelBase::PG_ListModelBase;

	bool empty() const override { return m_items.empty(); };
	std::size_t size() const override { return m_items.size(); };
	PG_ListModelLocation getLocation(const PG_Index& index) const override;

	PG_Variety getData(PG_Index index, Identity identity) const override;

	inline const std::shared_ptr<PG_MPPContainer>& getModelSD() const { return m_modelSD; }
	inline const std::shared_ptr<PG_MPPContainer>& getModelHD() const { return m_modelHD; }
	void setModels(const std::shared_ptr<PG_MPPContainer>& modelSD, const std::shared_ptr<PG_MPPContainer>& modelHD);

	void resetHD();
	void invalidate();

	inline const std::vector<T>& getItems() const { return m_items; }
	/**
	* @brief Adds an item that has no file behind it.
	* @details Such an item carries what it shows, nothing refills it out of a container. Used where a
	* geometry is shown on its own.
	* @returns The index it landed on
	*/
	int add(T item) {
		m_items.push_back(std::move(item));
		const int index = (int)m_items.size() - 1;
		emit itemsAdded(index, index);
		emit sizeChanged();
		return index;
	}

protected:
	void initialize() {
		initializeSD();
		initializeHD();
	}
	void initializeSD();
	void initializeHD();
	
	virtual void fillSD() = 0;
	virtual void fillHD() = 0;

	std::shared_ptr<PG_MPPContainer> m_modelSD = nullptr;
	std::shared_ptr<PG_MPPContainer> m_modelHD = nullptr;
	std::vector<T> m_items;
};

/**
* @brief Merges seperte MPP containers into one texture bundle
*/
class PG_DisaAssetsTextureFilterModel : public PG_DisaAssetsFilterModel<PG_DisaAssetsTextureItem> {
	PG_OBJECT(PG_DisaAssetsTextureFilterModel)
	PG_PARENT(PG_DisaAssetsFilterModel<PG_DisaAssetsTextureItem>)
public:
	using PG_DisaAssetsFilterModel<PG_DisaAssetsTextureItem>::PG_DisaAssetsFilterModel;

protected:
	void fillSD() override;
	void fillHD() override;
};

/**
* @brief Merges seperte MPP containers into one geoemtry bundle
*/

class PG_DisaAssetsGeoemtryFilterModel : public PG_DisaAssetsFilterModel<PG_DisaAssetsGeometryItem> {
	PG_OBJECT(PG_DisaAssetsGeoemtryFilterModel)
	PG_PARENT(PG_DisaAssetsFilterModel<PG_DisaAssetsGeometryItem>)
public:
	using PG_DisaAssetsFilterModel<PG_DisaAssetsGeometryItem>::PG_DisaAssetsFilterModel;

protected:
	void fillSD() override;
	void fillHD() override;
};

class PG_ResourceSynchronisator;


class PG_DisaAssetsModel : public PG_Object {
	PG_OBJECT(PG_DisaAssetsModel)
	PG_PARENT(PG_Object)
	PG_PROPERTY(bool enabled GET isEnabled SET setEnabled CHANGED enabledChanged)
	PG_PROPERTY(bool hdViewEnabled GET isHDViewEnabled SET setHDViewEnabled CHANGED enabledHDViewChanged)
signals:
	void enabledChanged();
	void enabledHDChanged();
	void enabledHDViewChanged();
public:
	enum ETextureType {
		TextureSD,
		TextureHD,
		TextureNormal,
	};

	using TextureListModel = PG_DisaAssetsTextureFilterModel;
	using GeometryListModel = PG_DisaAssetsGeoemtryFilterModel;

	using PG_Object::PG_Object;

	void init();
	bool open(PG_DisaFileEntry mppSDInfo, PG_DisaFileEntry mppHDInfo);
	void close();

	void exportTexture(const PG_File& file, PG_ListModelLocation location, ETextureType type) const;
	void importTexture(const PG_File& file, PG_ListModelLocation location, ETextureType type, unsigned short tx2Type = 999);

	void exportGeometry(const PG_File& file, PG_ListModelLocation location, bool hd) const;
	void importGeometry(const PG_File& file, PG_ListModelLocation location, bool hd);
	void importGeometry(const PG_GEO& geo, PG_ListModelLocation location, bool hd);
	void importGeometryGLTF(const PG_File& file, PG_ListModelLocation location, bool hd);
	/**
	* @brief Adds a geometry that is already built, rather than one read out of a file entry.
	* @details Used where a geometry has no file of its own to be reread from, a preview of a single geo.
	* The same model answers for SD and HD, there is only the one.
	* @returns The index it landed on, which is what an object's model_id names
	*/
	int addGeometry(PG_Model model);

	void addItem(bool texture);

	PG_ResourceSynchronisator* getResourceSynchronisator();

	bool isEnabled() const { return m_enabled; }
	void setEnabled(bool enabled) {
		if (m_enabled != enabled) {
			m_enabled = enabled;
			emit enabledChanged();
		}
	}

	bool isHDEnabled() const { return m_editEnabled || m_hdViewEnabled; }
	bool isHDViewEnabled() const { return m_hdViewEnabled; }

	/**
	* @brief Is the HD texture view enabled on map
	*/
	void setHDViewEnabled(bool enabled) {
		if (m_hdViewEnabled != enabled) {
			m_hdViewEnabled = enabled;
			if(!m_editEnabled) emit enabledHDChanged();
			emit enabledHDViewChanged();
		}
	}
	/**
	* @brief Is asset window open
	*/
	void setEditEnabled(bool enabled) {
		if (m_editEnabled != enabled) {
			m_editEnabled = enabled;
			if (!m_hdViewEnabled) emit enabledHDChanged();
		}
	}

	TextureListModel* getTextures() const { return m_textures; }
	GeometryListModel* getGeometries() const { return m_geomeries; }
	const std::shared_ptr<PG_MPPContainer>& getContainerSD() const { return m_containerSD; }
	const std::shared_ptr<PG_MPPContainer>& getContainerHD() const { return m_containerHD; }

	inline void clearModified() { m_modified.clear(); }

private:
	void importTexture(PG_TX2&& tx2, PG_ListModelLocation location, ETextureType type);

	std::shared_ptr<PG_MPPContainer> m_containerSD = nullptr;
	std::shared_ptr<PG_MPPContainer> m_containerHD = nullptr;
	TextureListModel* m_textures = nullptr;
	GeometryListModel* m_geomeries = nullptr;
	bool m_enabled = true;
	bool m_hdViewEnabled = false; //hd is enabled on map
	bool m_editEnabled = false; //assets window is open

	std::set<std::shared_ptr<class PG_DisaFileInterface>> m_modified;
};