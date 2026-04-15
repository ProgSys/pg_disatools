#include "DisaCustomRenderManager.h"

#include <GL/glew.h>
#include <Rendering/Geometry.h>
#include <Rendering/Texture.h>
#include <Rendering/Shader.h>
#include <Rendering/Mesh.h>
#include <Resources/PG_Material.h>
#include <Resources/PG_ResourceManager.h>

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

void DisaCustomRenderManager::RenderFullScreenImg::create() {
    auto dir = PG_ResourceManager::get()->getResourceLocation();
    if (!shader) shader = new GL::Shader(StringList{ dir + "/shaders/disa/screenFill.vert", dir + "/shaders/disa/screenFill.frag" });
    if (!texture) texture = new GL::RGBATexture();
    if (!imgData.empty()) static_cast<GL::RGBATexture*>(texture)->load(imgData.getWidth(), imgData.getHeight(), imgData.data());
    if (!mesh) mesh = new GL::Mesh(GL::Geometry::sfqUVyFlipped(), GL_TRIANGLES);
    imgData.clear();
    refreshRequested = false;
}

void DisaCustomRenderManager::RenderFullScreenImg::render() {
    if (!shader || !texture) return;
    shader->use();
    fillUniforms();
    texture->bind(0);
    mesh->draw();
    checkGLError();
}

void DisaCustomRenderManager::RenderFullScreenImg::fillUniforms() {
    shader->setUniform<int>("diffusetexture", 0);
}

void DisaCustomRenderManager::RenderFullScreenImg::remove() {
    if (shader) delete shader;
    if (texture) delete texture;
    if (mesh) delete mesh;
    removeRequested = false;
    shader = nullptr;
    texture = nullptr;
    mesh = nullptr;
}

void DisaCustomRenderManager::RenderFullScreenImg::update(const PG_RGBAImage& img) {
    imgData = img;
    refreshRequested = true;
}
void DisaCustomRenderManager::RenderFullScreenImg::update(PG_RGBAImage&& img) {
    imgData = std::move(img);
    refreshRequested = true;
}

/// 
/// =================== RenderScreenImg ===================
/// 
/// 
void DisaCustomRenderManager::RenderScreenImg::fillUniforms() {
    shader->setUniform<int>("diffusetexture", 0);
    shader->setUniform<glm::vec2>("pos", pos);
    shader->setUniform<glm::vec2>("size", size);
}

void DisaCustomRenderManager::RenderScreenImg::create() {
    auto dir = PG_ResourceManager::get()->getResourceLocation();
    if (!shader) shader = new GL::Shader(StringList{ dir + "/shaders/disa/screenImage.vert", dir + "/shaders/disa/screenFill.frag" });
    if (!texture) texture = new GL::RGBATexture();
    if (!imgData.empty()) static_cast<GL::RGBATexture*>(texture)->load(imgData.getWidth(), imgData.getHeight(), imgData.data());
    if (!mesh) mesh = new GL::Mesh(GL::Geometry::sfqUVyFlipped(), GL_TRIANGLES);
    imgData.clear();
    refreshRequested = false;
}

void DisaCustomRenderManager::RenderScreenImg::update(const PG_RGBAImage& img, glm::vec2 _pos, glm::vec2 _size) {
    imgData = img;
    pos = _pos;
    size = _size;
    refreshRequested = true;
}

void DisaCustomRenderManager::RenderScreenImg::update(PG_RGBAImage&& img, glm::vec2 _pos, glm::vec2 _size) {
    imgData = std::move(img);
    pos = _pos;
    size = _size;
    refreshRequested = true;
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
}

void DisaCustomRenderManager::glViewportReplace(int x, int y, int width, int height) {
    m_viewportPos = glm::ivec2(x, y);
    m_viewportSize = glm::ivec2(width, height);
    m_drawArraysCount = 0;

    if (!m_isInit) init();
    PG_EventLoop::doRun();
    {
        auto context = PG_Context::get();
        context->runCreate();
        context->destroyGarbage();
    }

    glViewport(x, y, width, height);


    auto cleanTasks = [](std::vector<RenderTask*>& v) {
        auto it = v.begin();
        while (it != v.end()) {
            if ((*it)->removeRequested) {
                (*it)->remove();
                delete (*it);
                it = v.erase(it);
            }
            else ++it;
        }
     };

    cleanTasks(m_backLayer);
    cleanTasks(m_frontLayer);
    cleanTasks(m_inbetweenLayer);
    if (m_fullClean) {
        m_backLayer.shrink_to_fit();
        m_frontLayer.shrink_to_fit();
    }

    bool needsRefresh = false;
    auto refreshTasks = [&needsRefresh](std::vector<RenderTask*>& v) {
        for (RenderTask* t : v) {
            if (t->refreshRequested) {
                if (!needsRefresh) {
                    resetDisaOpenGLStates(); //set to a known gl state
                    needsRefresh = true;
                }
                t->create();
            }
        }
        };

    refreshTasks(m_backLayer);
    refreshTasks(m_frontLayer);
    refreshTasks(m_inbetweenLayer);


    if (needsRefresh) {
        glPopAttrib(); //reset to original disa state
    }

}

void DisaCustomRenderManager::glDrawArraysReplace(unsigned int mode, int first, int count) {
    PG_Context::get()->runCreate();
    glDrawArrays(mode, first, count);
    if (!m_enabled) return;
    if (m_drawArraysCount == 0 && !m_backLayer.empty() ) {
        resetDisaOpenGLStates();

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        checkGLError();
        for (RenderTask* t : m_backLayer) {
            t->render();
            //t->removeRequested = true;
        }

        glPopAttrib();
    }
    ++m_drawArraysCount;
}

void DisaCustomRenderManager::lastReplace() {
    if (!m_enabled || m_frontLayer.empty()) return;

    resetDisaOpenGLStates();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    checkGLError();
    for (RenderTask* t : m_frontLayer) {
        t->render();
        //t->removeRequested = true;
    }

    glPopAttrib();
}

void DisaCustomRenderManager::renderInbetween() {
    if (m_inbetweenLayer.empty()) return;

    resetDisaOpenGLStates();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    checkGLError();
    for (RenderTask* t : m_inbetweenLayer) {
        t->render();
    }

    glPopAttrib();
}

void DisaCustomRenderManager::clearInbetween() {
    for (RenderTask* t : m_inbetweenLayer) {
        t->removeRequested = true;
    }
}

DisaCustomRenderManager::RenderTask* DisaCustomRenderManager::findRenderTask(ELayer layer, const std::string& identifier) {
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
        return (*findIt);
    }
    return nullptr;
}

void DisaCustomRenderManager::eraseRenderTask(ELayer layer, const std::string& identifier) {
    std::vector<RenderTask*>* target = nullptr;
    switch (layer)
    {
    case DisaCustomRenderManager::Front: target = &m_frontLayer; break;
    case DisaCustomRenderManager::Back: target = &m_backLayer; break;
    default: return;
    }

    if (auto findIt = std::ranges::find(*target, identifier, &RenderTask::identifier);
        findIt != target->end()) {
        (*findIt)->removeRequested = true;
    }
}

void DisaCustomRenderManager::clear() {
    auto cleanTasks = [](std::vector<RenderTask*>& v) {
        for (RenderTask* t : v) {
            t->removeRequested = true;
        }
        };
    cleanTasks(m_frontLayer);
    cleanTasks(m_backLayer);
    m_fullClean = true;
}

DisaCustomRenderManager* DisaCustomRenderManager::get() {
    static DisaCustomRenderManager manager;
    return &manager;
}

