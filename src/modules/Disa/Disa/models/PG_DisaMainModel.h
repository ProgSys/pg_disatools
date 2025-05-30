#pragma once
#include <Base/PG_Object.h>
#include <Disa/utils/PG_DisaConfig.h>
#include <Util/PG_Dir.h>

class PG_DisaMainModel : public PG_Object {
	PG_OBJECT(PG_DisaMainModel);
public:
	using PG_Object::PG_Object;

	virtual void init();

	static PG_DisaMainModel* getBase();

	//Config
	PG_DisaConfig& getConfig() { return m_config; }
	const PG_DisaConfig& getConfig() const { return m_config; }

	PG_File getExePath() const;

	PG_Dir getModDir() const;
	PG_Dir getCustomCampaignsDir() const;
	PG_Dir getCustomCharactersDir() const;

private:
	PG_DisaConfig m_config;
};