#pragma once
#include <Base/PG_Object.h>
#include <Util/PG_Dir.h>
#include <Scene/Model/PG_ListModelBase.h>
#include <Models/PG_UiStateBaseModel.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>
#include <stack>

class PG_DisaFileManagerMainModel : public PG_DisaMainModel {
	PG_OBJECT(PG_DisaFileManagerMainModel);
	PG_PARENT(PG_DisaMainModel);

	PG_PROPERTY(std::string navText GET getNavText CHANGED centerModelChanged)
signals:
	void centerModelChanged(PG_UiStateBaseModel* oldModel, PG_UiStateBaseModel* newModel);
public:
	using PG_DisaMainModel::PG_DisaMainModel;

	void init() override;

	static PG_DisaFileManagerMainModel* get();

	inline PG_UiStateBaseModel* getCenterModel() const { return m_modelStack.empty()? nullptr: m_modelStack.back(); }

	bool open(const PG_File& file);
	void pushStack(PG_UiStateBaseModel* model);
	void popStack();

	void close();
	/**
	* @brief Move one file back
	*/
	void navBack();
	std::string getNavText() const;

private:
	std::vector<PG_UiStateBaseModel*> m_modelStack;
};
