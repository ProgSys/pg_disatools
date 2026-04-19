#include "DisaCustomRenderManager.h"

#include <GL/glew.h>
#include <Resources/PG_Material.h>
#include <Resources/PG_ResourceManager.h>
#include <DisaRendering.h>

static inline void resetDisaOpenGLStates() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    // Reset depth testing
    glDisable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);

    // Reset depth mask
    glDepthMask(GL_TRUE);

    // Reset stencil testing
    glDisable(GL_STENCIL_TEST);

    // Reset blending
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
    glBlendEquation(GL_FUNC_ADD);

    // Reset face culling
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    // Reset lighting
    glDisable(GL_LIGHTING);

    // Reset alpha testing
    glDisable(GL_ALPHA_TEST);
    glAlphaFunc(GL_ALWAYS, 0);

    // Reset scissor test
    glDisable(GL_SCISSOR_TEST);

    // Reset shading model
    glShadeModel(GL_SMOOTH);

    // Reset matrix modes
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_COLOR);
    glLoadIdentity();

    // Reset polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Reset color mask
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Reset texture upload
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelTransferi(GL_MAP_COLOR, 0);
    glPixelTransferi(GL_MAP_STENCIL, 0);
}


void DisaCustomRenderManager::init() {
    m_isInit = true;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        OUT_ERROR("glewInit failed, something is seriously wrong: " << glewGetErrorString(err));
        return;
    }
    PG_EventLoop::start();
    PG_ResourceManager::create();
    PG_Context::create();
    m_cachedResources.push_back(PG_Any::from(PG_Material("M_DisaFontTest")));
#ifdef _DEBUG
    m_errorTexure.load("nsfw/face.png");
#else
    m_errorTexure.load("gui/disa/pg_img_missing_thumbnail.png");
#endif // _DEBUG
    m_fullImageScreenModel.load("T_DisaFullScreenImg", { { PG_Material("M_DisaFullScreenImg"), PG_Geometry::sfqUVyFlipped()  } });
    m_imageScreenModel.load("T_DisaScreenImg", { { PG_Material("M_DisaScreenImg"), PG_Geometry::sfq()  } });
}

void DisaCustomRenderManager::glViewportReplace(int x, int y, int width, int height) {
    m_viewportPos = glm::ivec2(x, y);
    m_viewportSize = glm::ivec2(width, height);

    if (!m_isInit) init();

    m_modelsOld = std::move(m_models);
    m_models.reserve(std::max(m_modelsOld.size(), 32u));

    PG_EventLoop::doRun();
    {
        auto context = PG_Context::get();
        context->runCreate();
        context->destroyGarbage();
    }

    glViewport(x, y, width, height);

}

void DisaCustomRenderManager::glDrawArraysReplace(unsigned int mode, int first, int count) {
    PG_Context::get()->runCreate();
    glDrawArrays(mode, first, count);
}



void DisaCustomRenderManager::submit(float depth, PG_DisaModel&& model, int blendMode, glm::mat4* modelMatrix, bool isPerspective, bool depthMaskEnabled, float poligonOffset, int stencilMode, unsigned char stencilRef, bool param_13) {
    assert(model.isValid());
    if (!model.isValid()) return;

    size_t index = addModel(std::move(model));
    DisaRenderObjectManager::get()->submitCustomModel(depth, index, blendMode, modelMatrix, isPerspective, depthMaskEnabled, poligonOffset, stencilMode, stencilRef, param_13);
}

DisaCustomRenderManager* DisaCustomRenderManager::get() {
    static DisaCustomRenderManager manager;
    return &manager;
}

