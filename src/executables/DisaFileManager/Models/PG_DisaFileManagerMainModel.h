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

	PG_PROPERTY(bool isModified GET isModified CHANGED isModifiedChanged)
	PG_PROPERTY(std::string navText GET getNavText CHANGED centerModelChanged)
signals:
	void isModifiedChanged();
	void centerModelChanged(PG_UiStateBaseModel* oldModel, PG_UiStateBaseModel* newModel);
public:
	using PG_DisaMainModel::PG_DisaMainModel;

	void init() override;

	static PG_DisaFileManagerMainModel* get();

	inline PG_UiStateBaseModel* getCenterModel() const { return m_modelStack.empty()? nullptr: m_modelStack.back(); }

	bool isModified() const;
	std::string getNavText() const;

	void addChangedFile(std::shared_ptr<PG_DisaContainerInterface>&& file) {
		assert(file);
		m_changedFiles.insert(std::move(file));
	};
	void eraseChangedFile(const std::shared_ptr<PG_DisaContainerInterface>& file) {
		m_changedFiles.erase(file);
	};

receivers:
	bool open(const PG_File& file);
	void pushStack(PG_UiStateBaseModel* model);
	void popStack();

	void save();
	void saveAs(const PG_File& file);


	void close();
	/**
	* @brief Move one file back
	*/
	void navBack();
private:
	std::vector<PG_UiStateBaseModel*> m_modelStack;
	std::set<std::shared_ptr<PG_DisaContainerInterface>> m_changedFiles;
};
