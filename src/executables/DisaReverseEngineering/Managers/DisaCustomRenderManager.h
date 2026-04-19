#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>
#include <vector>
#include <ranges>
#include <Util/PG_Any.h>
#include <Util/PG_DeltaTimer.h>
#include <Resources/PG_DisaModel.h>

class DisaCustomRenderManager {
	friend struct DisaRenderObjectManager;
public:
	bool isEnabled() const{ return m_enabled; }
	void setEnabled(bool value) { m_enabled = value; }

	void init();
	/**
	* @brief Injection of the very first glViewport command
	*/
	void glViewportReplace(int x, int y, int width, int height);
	/**
	* @brief Injection of DrawArrays
	*/
	void glDrawArraysReplace(unsigned int mode, int first, int count);

	/**
	* @brief Will add a new render object to render queue
	* Original function: FUN_00425df0
	* @param depth Used for layering
	*/
	void submit(float depth, PG_DisaModel&& model, int blendMode, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled, float poligonOffset, int stencilMode, unsigned char stencilRef, bool param_13);

	const std::vector<PG_DisaModel>& getModels() const { return m_models; };

	const PG_Texture& getErrorTexture() const { return m_errorTexure; }
	const PG_Texture& getLoadTexture() const { return m_loadTexture; }
	const PG_Model& getFullImageScreenModel() const { return m_fullImageScreenModel; }
	const PG_Model& getImageScreenModel() const { return m_imageScreenModel; }

	const PG_DeltaTimer& getDeltaTimer() const { return m_deltaTimer; }
	/**
	* @returns Singelton
	*/
	static DisaCustomRenderManager* get();

	static void resetDisaOpenGLStates();
protected:
	size_t addModel(PG_DisaModel&& model) {
		auto i = m_models.size();
		m_models.emplace_back(std::move(model));
		return i;
	}
private:
	bool m_isInit = false;
	bool m_enabled = true;
	glm::ivec2 m_viewportPos;
	glm::ivec2 m_viewportSize;
	PG_DeltaTimer m_deltaTimer;

	PG_Texture m_errorTexure;
	PG_Texture m_loadTexture;
	PG_Model m_fullImageScreenModel;
	PG_Model m_imageScreenModel;
	std::vector<PG_Any> m_cachedResources;

	std::vector<PG_DisaModel> m_models;
	std::vector<PG_DisaModel> m_modelsOld;
};