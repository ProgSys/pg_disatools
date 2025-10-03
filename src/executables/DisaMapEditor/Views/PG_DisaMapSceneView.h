#pragma once

#include <Scene/PG_Scene.h>
#include <Models/PG_UiStateBaseModel.h>
#include <Stream/PG_StreamIn.h>
#include <Disa/fileformats/PG_DisaFileInfo.h>
#include <Util/PG_File.h>

class PG_DisaMapNode;
class PG_DisaMapViewNode;
class PG_DisaMapModel;
class PG_DisaFileEntry;

class PG_DisaMapSceneView : public PG_Scene, public PG_UiStateBaseModel
{
public:
	using PG_Scene::PG_Scene;

	UiState getUiState() const override { return UiState::MapView; }
	void init() override;
	void copy(PG_DisaMapSceneView* scene);

	bool event(PG_Event* e) override;

	inline bool getPreviewMode() const { return m_previewMode; }
	inline void setPreviewMode(bool v) { m_previewMode = v; }
	void setAxisVisible(bool visible);
	void setShowHiddenModel(bool visible);

	bool isActorsVisible() const;
	void setActorsVisible(bool visible);

	bool isGeoPanelsVisible() const;
	void setGeoPanelsVisible(bool visible);

	void setEnableEditing(bool enable);

	bool isHDMode() const;
	void setHDMode(bool state);

	void exportGLTFClicked();
	void addModelClicked(int index, glm::vec3 pos);
	void addDublicateClicked(int objectIndex);

	PG_DisaMapViewNode* getMapNode() const { return m_mapNodeV2; }
	PG_DisaMapModel* getMapModel() const;

	void setModel(const std::shared_ptr<PG_UiStateBaseModel>& container);

private:
	void orbitCamera(float value);

	bool m_previewMode = false; //preview mode
	
	PG_DisaMapViewNode* m_mapNodeV2 = nullptr;
};

