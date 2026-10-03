#include "DisaCustomRenderManager.h"

#include <GL/glew.h>
#include <Base/PG_EventLoop.h>
#include <Base/Events/PG_TickEvent.h>
#include <Scene/PG_Scene.h>
#include <Resources/PG_Material.h>
#include <Resources/PG_ResourceManager.h>
#include <Managers/DisaCustomBaseModManager.h>
#include <DisaRendering.h>

void DisaCustomRenderManager::resetDisaOpenGLStates() {
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
    if (m_isInit) return;
    m_isInit = true;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        OUT_ERROR("glewInit failed, something is seriously wrong: " << glewGetErrorString(err));
        exit(1);
        return;
    }
    PG_EventLoop::start();
    PG_ResourceManager::create(DisaCustomBaseModManager::get()->exePath().getPath() + "/resources");
    PG_Context::create();
    m_cachedResources.push_back(PG_Any::from(PG_Material("M_DisaFontTest")));
#ifdef _DEBUG
    m_errorTexure.load("nsfw/face.png");
#else
    m_errorTexure.load("gui/disa/pg_img_missing_thumbnail.png");
#endif // _DEBUG
    m_loadTexture.load("gui/disa/disaMissingno.png");
    m_fullImageScreenModel.load("T_DisaFullScreenImg", { { PG_Material("M_DisaFullScreenImg"), PG_Geometry::sfqUVyFlipped()  } });
    m_imageScreenModel.load("T_DisaScreenImg", { { PG_Material("M_DisaScreenImg"), PG_Geometry::sfq()  } });

    m_scene = PG_new<PG_Scene>("DisaCustomScene");
    m_scene->init();
    //both stay visible, a hidden actor stops the ticks of everything below it
    m_worldGroup = PG_new<PG_ActorNode>("DisaWorldGroup", m_scene->getRoot());
    m_screenGroup = PG_new<PG_ActorNode>("DisaScreenGroup", m_scene->getRoot());
}

void DisaCustomRenderManager::onFrameBegin() {
    m_deltaTimer.tick();

    init();

    /* The index a custom render object carries points into m_frameNodes, and the game wipes its own
     * render list in the same frame step, so both have to be emptied at the same moment. */
    m_frameNodes.clear();
    m_frameKeyUses.clear();
    releaseUnusedNodes();

    //the scene animates on ticks, which are in milliseconds while the delta timer counts seconds
    PG_EventLoop::postEvent(PG_new<PG_TickEvent>(m_deltaTimer.getDeltaTime() * 1000.0), m_scene);
    PG_EventLoop::doRun();
    {
        PG_Context::runCreate();
        PG_Context::destroyGarbage();
    }
}

void DisaCustomRenderManager::glViewportReplace(int x, int y, int width, int height) {
    m_viewportPos = glm::ivec2(x, y);
    m_viewportSize = glm::ivec2(width, height);
    glViewport(x, y, width, height);
}

void DisaCustomRenderManager::glDrawArraysReplace(unsigned int mode, int first, int count) {
    glDrawArrays(mode, first, count);
}



DisaCustomRenderManager::NodeId DisaCustomRenderManager::makeNodeId(std::string_view key) {
    return PG_Hash::fnv1a(key) & ~g_uniqueIdBit;
}

DisaCustomRenderManager::NodeId DisaCustomRenderManager::makeFrameNodeId(std::string_view key) {
    const NodeId base = makeNodeId(key);
    const int use = m_frameKeyUses[base]++;
    return use == 0 ? base : PG_Hash::combine(base, use) & ~g_uniqueIdBit;
}

void DisaCustomRenderManager::submit(float depth, PG_ActorNode* node, int blendMode, bool isPerspective, bool depthMaskEnabled, float polygonOffset, int stencilMode, unsigned char stencilRef, bool useColorTint) {
    assert(node);
    if (!node) return;

    const int index = static_cast<int>(m_frameNodes.size());
    m_frameNodes.push_back(node);
    //the node carries its own matrix, the render object needs none
    DisaRenderObjectManager::get()->submitCustomModel(depth, index, blendMode, nullptr, isPerspective, depthMaskEnabled, polygonOffset, stencilMode, stencilRef, useColorTint);
}

PG_ActorNode* DisaCustomRenderManager::getFrameNode(int index) const {
    if (index < 0 || index >= static_cast<int>(m_frameNodes.size())) return nullptr;
    return m_frameNodes[index];
}

void DisaCustomRenderManager::releaseUnusedNodes() {
    std::erase_if(m_nodes, [this](const auto& pair) {
        const auto& [id, node] = pair;
        if (m_usedIds.contains(id) || m_usedIdsLast.contains(id)) return false;
        if (node) node->deleteLater();
        return true;
    });
    m_usedIdsLast = std::move(m_usedIds);
    m_usedIds.clear();
}

DisaCustomRenderManager* DisaCustomRenderManager::get() {
    static DisaCustomRenderManager manager;
    return &manager;
}

