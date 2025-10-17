#pragma once
#include <Base/PG_Object.h>
#include <Disa/utils/PG_DisaConfig.h>
#include <Util/PG_Dir.h>


class PG_DisaMainBaseModel {
public:
	PG_DisaMainBaseModel();

	virtual PG_File getExePath() const = 0;

	PG_Dir getModDir() const;
	PG_Dir getCustomCampaignsDir() const;
	PG_Dir getCustomCharactersDir() const;
	PG_Dir getCustomScripsDir() const;
	PG_Dir getCustomTalkScripsDir() const;


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

private:
	PG_DisaConfig m_config;
};