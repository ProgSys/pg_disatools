#pragma once

#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Model/PG_ListModelLocation.h>

class PG_PropertyDynamicWrapper;
class PG_DisaItemSelector;
class PG_GuiText;
class PG_GuiItem;
class PG_GuiTextField;

class PG_DisaCustomCampaignBase : public PG_GuiRectangle {
	PG_OBJECT(PG_DisaCustomCampaignBase);
	PG_PARENT(PG_GuiRectangle);
public:
	using PG_GuiRectangle::PG_GuiRectangle;

	PG_ListModelLocation getLocation() const { return m_location; }
	void setLocation(const PG_ListModelLocation& location);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);

	static PG_GuiText* textCreator(const std::string& name, PG_GuiItem* root, float fontSize = 17.f);
	static PG_GuiTextField* textFieldCreator(const std::string& name, PG_GuiItem* root);

protected:
	void callFunc(const std::string& funcName) const;
	void updateModel();

	PG_ListModelLocation m_location;
	PG_PropertyDynamicWrapper* m_model = nullptr;
};

class PG_DisaCustomCampaignDungeon : public PG_DisaCustomCampaignBase {
	PG_OBJECT(PG_DisaCustomCampaignDungeon);
	PG_PARENT(PG_DisaCustomCampaignBase);
public:
	using PG_DisaCustomCampaignBase::PG_DisaCustomCampaignBase;

	void init() override;
};

class PG_DisaCustomCampaignArea : public PG_DisaCustomCampaignBase {
	PG_OBJECT(PG_DisaCustomCampaignArea);
	PG_PARENT(PG_DisaCustomCampaignBase);
public:
	using PG_DisaCustomCampaignBase::PG_DisaCustomCampaignBase;

	void init() override;
};
namespace PG_Gui{
	PG_DisaItemSelector* createImageSelector(const std::string& name, PG_GuiItem* root);
}