#pragma once
#include <Base/PG_Object.h>
#include <Disa/utils/PG_DisaConfig.h>
#include <Util/PG_Dir.h>
#include <string>

class PG_DisaFileContainer;
struct PG_DisaModelResources;

class PG_DisaMainBaseModel {
public:
	PG_DisaMainBaseModel();

	virtual PG_File getExePath() const = 0;

	PG_Dir getModDir() const;
	PG_Dir getCustomCampaignsDir() const;
	PG_Dir getCustomModificationsDir() const;
	PG_Dir getCustomScripsDir() const;
	PG_Dir getCustomTalkScripsDir() const;
	PG_Dir getCustomTranslationDir() const;


	/// Same as getCustomModificationsDir, for code that has an exe path but no main model
	static PG_Dir customModificationsDir(const PG_File& exePath);

	static PG_DisaMainBaseModel* get();
	static PG_File exePath() { return get()->getExePath(); }
};

class PG_DisaMainModel : public PG_Object, public PG_DisaMainBaseModel {
	PG_OBJECT(PG_DisaMainModel);
public:
	using PG_Object::PG_Object;

	virtual void init();

	static PG_DisaMainModel* getBase() { return dynamic_cast<PG_DisaMainModel*>(PG_DisaMainBaseModel::get()); }

	//Config
	PG_DisaConfig& getConfig() { return m_config; }
	const PG_DisaConfig& getConfig() const { return m_config; }

	PG_File getExePath() const override;

	/**
	* @brief Reads a GEO out of the container and keeps it as a global model resource
	* @param resourceName The name the resource takes, "R_" plus the file name when left empty
	* @details Scene nodes ask for these by name, the teleporter slot wants "R_DisaTeleporterBase".
	* Holding them here is what keeps them loaded, a map that needs one loads nothing itself.
	* @returns false when the file is missing or is no GEO
	*/
	bool cachDisaGeometry(PG_DisaFileContainer* container, const std::string& fileName, const std::string& resourceName = {});

private:
	PG_DisaConfig m_config;
};