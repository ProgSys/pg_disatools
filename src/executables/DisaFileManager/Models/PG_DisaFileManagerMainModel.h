#pragma once
#include <Base/PG_Object.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Util/PG_Dir.h>
#include <Scene/Model/PG_ListModelBase.h>
#include <Models/PG_UiStateBaseModel.h>
#include <stack>

class PG_DisaFileManagerMainModel : public PG_DisaMainModel {
	PG_OBJECT(PG_DisaFileManagerMainModel);
	PG_PARENT(PG_DisaMainModel);
signals:
	void centerModelChanged(PG_UiStateBaseModel* model);
public:
	using PG_DisaMainModel::PG_DisaMainModel;

	void init() override;

	static PG_DisaFileManagerMainModel* get();

	PG_UiStateBaseModel* getCenterModel() const { return m_modelStack.empty()? nullptr: m_modelStack.top();}

	bool open(const PG_File& file);
	void pushStack(PG_UiStateBaseModel* model);

	void close();

private:
	std::stack<PG_UiStateBaseModel*> m_modelStack;
};
