#pragma once
#include <Base/PG_Object.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Util/PG_Dir.h>
#include <Scene/Model/PG_ListModelBase.h>

class PG_DisaModManagerMainModel : public PG_DisaMainModel {
	PG_OBJECT(PG_DisaModManagerMainModel);
	PG_PARENT(PG_DisaMainModel);
public:
	enum EUiState {
		None,
		CustomCharacters,
		CustomCampaigns,
	};
signals:
	void uiStateChanged(EUiState state);
	void centerModelChanged(PG_ListModelBase* model);
public:
	using PG_DisaMainModel::PG_DisaMainModel;

	void init() override;

	static PG_DisaModManagerMainModel* get();

	void setUiState(EUiState state);
	EUiState getUiState() const { return m_state;}
	PG_ListModelBase* getCenterModel() const { return m_centerModel;}

private:
	EUiState m_state = None;
	PG_ListModelBase* m_centerModel = nullptr;
};
