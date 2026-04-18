#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>
#include <vector>
#include <ranges>
#include <Util/PG_Image.h>
#include <Util/PG_Any.h>
#include <Resources/PG_DisaModel.h>

namespace GL {
	class Texture;
	class Mesh;
	class Shader;
}
struct RenderObject;

class DisaCustomRenderManager {
	friend struct DisaRenderObjectManager;
public:
	enum ELayer {
		Front,
		Back,
		Inbetween
	};
	struct RenderTask {
		RenderTask(const std::string& _identifier) : identifier(_identifier) {}
		RenderTask(std::string&& _identifier) : identifier(std::move(_identifier)) {}

		virtual void create() = 0;
		virtual void render() = 0;
		virtual void fillUniforms() = 0;
		virtual void remove() = 0;

		std::string identifier;
		bool refreshRequested = true;
		bool removeRequested = false;
	};

	struct RenderFullScreenImg: RenderTask {
		RenderFullScreenImg(std::string&& _identifier, PG_RGBAImage&& img) :
			RenderTask(std::move(_identifier)), imgData(std::move(img)) {}
		RenderFullScreenImg(const std::string& _identifier, const PG_RGBAImage& img) :
			RenderTask(_identifier), imgData(img) {}

		PG_RGBAImage imgData;
		std::string path;
		GL::Shader* shader = nullptr;
		GL::Texture* texture = nullptr;
		GL::Mesh* mesh = nullptr;

		void create() override;
		void render() override;
		void fillUniforms() override;
		void remove() override;
		void update(const PG_RGBAImage& img);
		void update(PG_RGBAImage&& img);
	};

	struct RenderScreenImg : RenderFullScreenImg {
		RenderScreenImg(std::string&& _identifier, PG_RGBAImage&& img, glm::vec2 _pos, glm::vec2 _size) :
			RenderFullScreenImg(std::move(_identifier), std::move(img)), pos(_pos), size(_size){}
		RenderScreenImg(const std::string& _identifier, const PG_RGBAImage& img, glm::vec2 _pos, glm::vec2 _size) :
			RenderFullScreenImg(_identifier, img), pos(_pos), size(_size) {}

		glm::vec2 pos;
		glm::vec2 size;

		~RenderScreenImg();
		void create() override;
		void fillUniforms() override;

		void update(const PG_RGBAImage& img, glm::vec2 _pos, glm::vec2 _size);
		void update(PG_RGBAImage&& img, glm::vec2 _pos, glm::vec2 _size);
	};

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
	* @brief Render last
	*/
	void lastReplace();
	/**
	* @brief Render trigger inbeween layers
	*/
	void renderInbetween();
	void clearInbetween();

	template<typename TTask, typename... Args>
	TTask* addRenderTask(ELayer layer, std::string identifier, Args&&... args) {
		std::vector<RenderTask*>* target = nullptr;
		switch (layer)
		{
		case DisaCustomRenderManager::Front: target = &m_frontLayer; break;
		case DisaCustomRenderManager::Back: target = &m_backLayer; break;
		case DisaCustomRenderManager::Inbetween: target = &m_inbetweenLayer; break;
		default: return nullptr;
		}

		if (auto findIt = std::ranges::find(*target, identifier, &RenderTask::identifier);
			findIt != target->end()) {
			TTask* t = dynamic_cast<TTask*>(*findIt);
			assert(t);
			if (!t) return nullptr; //mixed ids and types
			t->removeRequested = false;
			t->update(std::forward<Args>(args)...);
			return t;
		}

		auto* task = new TTask(identifier, std::forward<Args>(args)...);
		target->push_back(task);
		return task;
	}

	RenderTask* findRenderTask(ELayer layer, const std::string& identifier);

	void eraseRenderTask(ELayer layer, const std::string& identifier);

	void clear();

	/**
	* @brief Will add a new render object to render queue
	* Original function: FUN_00425df0
	* @param depth Used for layering
	*/
	void submit(float depth, PG_DisaModel&& model, int blendMode, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled, float poligonOffset, int stencilMode, unsigned char stencilRef, bool param_13);

	const std::vector<PG_DisaModel>& getModels() const { return m_models; };
	/**
	* @returns Singelton
	*/
	static DisaCustomRenderManager* get();
protected:
	size_t addModel(PG_DisaModel&& model) {
		auto i = m_models.size();
		m_models.emplace_back(std::move(model));
		return i;
	}
private:
	bool m_isInit = false;
	bool m_enabled = false;
	glm::ivec2 m_viewportPos;
	glm::ivec2 m_viewportSize;
	int m_drawArraysCount = 0;

	std::vector<RenderTask*> m_frontLayer;
	std::vector<RenderTask*> m_backLayer;
	std::vector<RenderTask*> m_inbetweenLayer;
	bool m_fullClean = false;
	std::vector<PG_Any> m_cachedResources;

	std::vector<PG_DisaModel> m_models;
	std::vector<PG_DisaModel> m_modelsOld;
};