#pragma once

#include <Disa/fileformats/PG_ZIP.h>
#include <Disa/resources/PG_ZIPResourceSource.h>
#include <Resources/PG_ResourceSourceScope.h>
#include <Resources/PG_Texture.h>
#include <Util/PG_ValueSetter.h>
#include <Scene/Model/PG_ListModel.h>
#include <nlohmann/json_fwd.hpp>

class PG_CustomModificationModel;
/**
* @brief Conainer for modefierd data
*/
class PG_DisaModificationJsonContainer : public PG_DisaChangedContainer {
	PG_OBJECT(PG_DisaModificationJsonContainer);
	PG_PARENT(PG_DisaChangedContainer);

	PG_PROPERTY(std::string name GET getName SET setName CHANGED nameChanged);
	PG_PROPERTY(std::string author GET getAuthor SET setAuthor CHANGED authorChanged);
	PG_PROPERTY(std::string description GET getDescription SET setDescription CHANGED descriptionChanged);
	PG_PROPERTY(std::string previewImgPath GET getPreviewImgPath SET setPreviewImgPath CHANGED previewImgPathChanged);

	PG_PROPERTY(PG_ListModelBase* rootModel GET getRootModel);
signals:
	void uiStateChanged();
	void subUiStateChanged();

	void idChanged();
	void nameChanged();
	void authorChanged();
	void descriptionChanged();
	void previewImgPathChanged();
public:
	using PG_DisaChangedContainer::PG_DisaChangedContainer;

	void init();

	bool open(std::shared_ptr<PG::STREAM::In> reader) override;
	bool refresh(std::shared_ptr<PG::STREAM::In> reader) override;

	bool save(std::shared_ptr<PG::STREAM::Out> writer) const override;

	bool isModified() const override { return m_isModified; }
	void markModified();

	const std::string& getId() const { return m_id; }
	void setId(const std::string& id) { PG_ValueSetter::change(m_id, id, this, &PG_DisaModificationJsonContainer::idChanged, &PG_DisaModificationJsonContainer::markModified); }

	const std::string& getName() const { return m_name; }
	void setName(const std::string& name) { PG_ValueSetter::change(m_name, name, this, &PG_DisaModificationJsonContainer::nameChanged, &PG_DisaModificationJsonContainer::markModified); }

	const std::string& getAuthor() const { return m_author; }
	void setAuthor(const std::string& author) { PG_ValueSetter::change(m_author, author, this, &PG_DisaModificationJsonContainer::authorChanged, &PG_DisaModificationJsonContainer::markModified); }

	const std::string& getDescription() const { return m_description; }
	void setDescription(const std::string& value) { PG_ValueSetter::change(m_description, value, this, &PG_DisaModificationJsonContainer::descriptionChanged, &PG_DisaModificationJsonContainer::markModified); }

	const std::string& getPreviewImgPath() const { return m_previewImgPath; }
	void setPreviewImgPath(const std::string& value) { PG_ValueSetter::change(m_previewImgPath, value, this, &PG_DisaModificationJsonContainer::previewImgPathChanged, &PG_DisaModificationJsonContainer::markModified); }

	/// The namespace the mod json declares, empty when it declares none
	const std::string& getNamespaceMod() const { return m_namespaceMod; }

	PG_CustomModificationModel* getRootModel() const;
private:
	bool m_isModified = false;

	std::string m_id;
	std::string m_name;
	std::string m_author;
	std::string m_description;
	std::string m_previewImgPath;
	std::string m_namespaceMod;
};

class PG_CustomModificationModel : public PG_ZIP {
	PG_OBJECT(PG_CustomModificationModel);
	PG_PARENT(PG_ZIP);
	PG_PROPERTY(PG_Object* modificationJson GET getModificationJsonContainer);
public:
	enum SubUiState: int {
		SubUiState_Settings,
		SubUiState_DungeonsPlan,
		SubUiState_Dungeons,
		SubUiState_Images,
		SubUiState_Scripts,
		SubUiState_SIZE,
	};

	using PG_ZIP::PG_ZIP;
	using PG_ZIP::save;

	void createNew(std::string name);
	/**
	* @brief Saves the mod to its file.
	* @details The resource source reads the same file, so it is suspended for the write and
	* indexes the new content afterwards.
	*/
	bool save() override;

	PG_DisaModificationJsonContainer* getModificationJsonContainer() const { return m_modificationJsonContainer.get(); }
	/**
	* @brief The namespace the mod content is offered to the resource manager under, the same one the game uses.
	* @returns Empty without a mod json, or when neither it nor the file name is a valid namespace.
	*/
	std::string getResourceNameSpace() const;

	PG_Texture getPreviewImg() const;

receivers:
	void importFile(const PG_File& file);
	void customRemove(std::size_t index); //called when item shall be removed

	void saveToFile() { save(); }

private:
	void afterOpen(zipUPtr& zip) final;
	/**
	* @brief Offers everything the mods file carries to the resource manager under its namespace.
	* @details Does nothing without a valid namespace, or for a mod never saved.
	*/
	void registerResourceSource();

	std::shared_ptr<PG_DisaModificationJsonContainer> m_modificationJsonContainer;
	/// The registration, so the mods content lives exactly as long as the mod does
	PG_ResourceSourceScope<PG_ZIPResourceSource> m_resourceScope;
};