#pragma once
#include <Base/PG_Object.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Util/PG_Dir.h>
#include <Scene/Model/PG_ListModelBase.h>

class PG_DisaFileManagerMainModel : public PG_DisaMainModel {
	PG_OBJECT(PG_DisaFileManagerMainModel);
	PG_PARENT(PG_DisaMainModel);
signals:
	void centerModelChanged(PG_ListModelBase* model);
public:
	using PG_DisaMainModel::PG_DisaMainModel;

	void init() override;

	static PG_DisaFileManagerMainModel* get();

	PG_ListModelBase* getCenterModel() const { return m_centerModel;}

	bool open(const PG_File& file);

private:
	PG_ListModelBase* m_centerModel = nullptr;
	PG_File m_openFile;
};
