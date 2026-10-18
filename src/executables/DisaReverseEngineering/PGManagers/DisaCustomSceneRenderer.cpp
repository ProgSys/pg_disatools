#include "DisaCustomSceneRenderer.h"

#include <gl/glew.h>
#include <Base/PG_Application.h>
#include <Base/PG_Window.h>
#include <Base/Events/PG_RenderEvent.h>
#include <Renderer/PG_RenderJobPool.h>
#include <Renderer/Command/PG_CommandRendererJob.h>
#include <Renderer/Command/PG_GuiCommands.h>
#include <Resources/PG_Context.h>
#include <Scene/Gui/PG_GuiMain.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Managers/DisaCustomRenderManager.h>

static PG_HashString g_sceneContext = "scene";

DisaCustomSceneRenderer::~DisaCustomSceneRenderer() {
	m_job.reset();
	m_garbage.reset();
	PG_RenderJobPool<PG_CommandRendererJob>::clear();
}

void DisaCustomSceneRenderer::init() {
	m_clipMat.load("M_GuiClip");
	m_clipGeo = PG_Geometry::gui();
	m_errorTexture.load("error");
	m_errorMaterial.load("M_GuiImageError");

	GLint stencilBits = 0;
	glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
	m_hasStencil = stencilBits > 0;
	if (!m_hasStencil) OUT_WARNING("The game's framebuffer has no stencil buffer, clipping gui items will draw unclipped!");
}

std::pair<PG_RenderJobEvent*, PG_GarbageJobEvent*> DisaCustomSceneRenderer::createJob(PG_RenderEvent* e) {
	const PG_Window* window = e->getWindow();
	if (!window) return { nullptr, nullptr };
	const glm::ivec2 size = window->getSize();
	if (size.x <= 0 || size.y <= 0) return { nullptr, nullptr };

	PG_CommandRendererJob_SPtr job = PG_RenderJobPool<PG_CommandRendererJob>::take();
	job->pass.resize(1);
	job->creationSize = 20;
	job->errorTexture = m_errorTexture;
	job->errorMaterial = m_errorMaterial;

	PG_CommandRenderPass& pass = job->pass.front();
	pass.context.setRender(g_sceneContext);
	pass.data.reserve(2000);
	pass.currentBucket = PG_RenderCommand::Generic;
	pass.renderData.window = window->getWindowData();
	pass.renderData.size = size; //the gui ortho is built from it, @see Bind_GuiProjectionMatrix
	pass.renderData.deltaTime = e->getDeltaTime();
	pass.renderData.ticks = static_cast<double>(PG_Application::ticks());
	pass.renderData.projectionMatrix = glm::mat4(1.f);
	pass.renderData.viewMatrix = glm::mat4(1.f);

	addGuiBegin(pass);
	addSceneTree(m_guiMain, *job, pass, m_clipMat, m_clipGeo);
	addGuiEnd(pass);
	sortGuiBucket(pass);

	auto renderJob = new PG_RenderJobEvent(this, job,
		std::bind(&DisaCustomSceneRenderer::render, this, std::placeholders::_1),
		std::bind(&DisaCustomSceneRenderer::merge, this, std::placeholders::_1, std::placeholders::_2));

	PG_Context::takeGarbageOut(job->garbage);
	PG_GarbageJobEvent* garbageJob = nullptr;
	if (!job->garbage.empty()) {
		garbageJob = new PG_GarbageJobEvent(this, job,
			std::bind(&DisaCustomSceneRenderer::destroyGarbage, this, std::placeholders::_1),
			std::bind(&DisaCustomSceneRenderer::mergeGarbage, this, std::placeholders::_1, std::placeholders::_2));
	}
	return { renderJob, garbageJob };
}

void DisaCustomSceneRenderer::renderStored() {
	if (!m_job && !m_garbage) return;

	//glPushAttrib covers none of these, the game's draws after this one may still rely on them
	GLint program = 0, vertexArray = 0, arrayBuffer = 0, activeTexture = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArray);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBuffer);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);

	DisaCustomRenderManager::resetDisaOpenGLStates();
	PG_Context::runCreate();

	if (m_garbage) {
		m_garbage->render();
		m_garbage.reset();
	}

	if (m_job) {
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//gui clipping counts up from 0, and the UI pass is the last user of the stencil in a frame
		if (m_hasStencil) {
			glStencilMask(0xFF);
			glClearStencil(0);
			glClear(GL_STENCIL_BUFFER_BIT);
		}
		m_job->render();
		m_job.reset();
		emit rendered();
	}

	glPopAttrib();
	glUseProgram(program);
	glBindVertexArray(vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
	glActiveTexture(activeTexture);
}

void DisaCustomSceneRenderer::render(const PG_RenderJob_SPtr& genericJob) {
	const auto job = std::static_pointer_cast<PG_CommandRendererJob>(genericJob);
	for (PG_CommandRenderPass& pass : job->pass) {
		pass.run();
	}
}

void DisaCustomSceneRenderer::merge(PG_RenderJob_SPtr& genericJob, PG_RenderJob_SPtr& genericJobOld) {
	const auto job = std::static_pointer_cast<PG_CommandRendererJob>(genericJob);
	const auto jobOld = std::static_pointer_cast<PG_CommandRendererJob>(genericJobOld);
	job->merge(jobOld);
}

void DisaCustomSceneRenderer::mergeGarbage(PG_RenderJob_SPtr& genericJob, PG_RenderJob_SPtr& genericJobOld) {
	const auto job = std::static_pointer_cast<PG_CommandRendererJob>(genericJob);
	const auto jobOld = std::static_pointer_cast<PG_CommandRendererJob>(genericJobOld);
	job->mergeGarbage(jobOld);
}

void DisaCustomSceneRenderer::destroyGarbage(const PG_RenderJob_SPtr& genericJob) {
	const auto job = std::static_pointer_cast<PG_CommandRendererJob>(genericJob);
	job->destroyGarbage();
}

bool DisaCustomSceneRenderer::isReady() const {
	return PG_Renderer::isReady() && m_guiMain && PG_Context::isInit();
}

void DisaCustomSceneRenderer::submitJobs(PG_RenderJobEvent* renderJob, PG_GarbageJobEvent* garbageJob) {
	if (renderJob) {
		if (m_job) renderJob->merge(m_job.get());
		m_job.reset(renderJob);
	}
	if (garbageJob) {
		if (m_garbage) garbageJob->merge(m_garbage.get());
		m_garbage.reset(garbageJob);
	}
}
