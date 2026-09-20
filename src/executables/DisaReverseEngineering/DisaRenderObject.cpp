#include "DisaRenderObject.h"

#include <gl/glew.h>
#include <cassert>
#include <cstdint>

#include <FunctionPatcher.h>

namespace {

    /**
    * @brief Maps DisaRenderObject::primitiveType to a GL primitive mode.
    * Entry 0 is -1 and no submit was found that uses it.
    */
    inline GLenum getPrimitiveMode(int16_t primitiveType) {
        return (&FP::getData<GLenum>(0x00596ea0))[primitiveType];
    }

    /**
    * @brief Shared body of the two blend list binders. The record holds two display lists and
    * picks between them with bit 29 of the word at its offset 0x8.
    * @param cacheValue What 0x005AC920 has to hold for the list to already be bound.
    */
    void callBlendList(int blendMode, unsigned int tableAddress, int cacheValue) {
        if (blendMode == -1) blendMode = FP::getData<int>(0x005bae6c);

        int& lastBlendList = FP::getData<int>(0x005ac920);
        if (lastBlendList == cacheValue + blendMode) return;

        const unsigned int record = tableAddress + blendMode * 0x30;
        const unsigned int listSelector = (FP::getData<unsigned int>(record + 8) >> 0x1d) & 1;
        glCallList(FP::getData<GLuint>(record + listSelector * 4));
        glGetError();
        lastBlendList = cacheValue + blendMode;
    }

    /**
    * @brief Hands the texture the sampler state the object captured when it was submitted,
    * then lets the game upload and bind it.
    */
    void applyTexture(DisaTextureSPtr* texture, const DisaRenderObjectTexParameter& textureInfo) {
        FUN_00428120_glActiveTexture(texture);

        texture->m_field0xc = textureInfo.paletteIndex;
        texture->m_field0xe = 0;
        if (texture->m_field0x4) {
            texture->m_field0x4[0xe] = (int8_t)textureInfo.magFilter;
            //a wrap mode of 0 means the texture keeps the one it already has
            if (const int wrapMode = (int8_t)textureInfo.wrapMode; wrapMode != 0)
                texture->m_field0x4[0xd] = wrapMode;
        }

        FP::callFastFarA<void>(0x00424480, texture); //upload if the palette changed
        FP::callFastFarA<void>(0x004245e0, texture); //bind and set glTexParameteri
    }

    /**
    * @brief Picks the program from what the object's flags say its surface is.
    * @return true when the program wants the light position, which the caller then uploads.
    */
    bool applyMaterialProgram(DisaRenderFlags renderFlags) {
        if (!renderFlags.useTexture) {
            FUN_00421a30_glUseAndInitProgram(1, 2); //flat vertex color
            return true;
        }
        if (!renderFlags.useLighting) {
            FUN_00421a30_glUseAndInitProgram(0, 0); //plain textured, the only case without a light
            return false;
        }
        FUN_00421a30_glUseAndInitProgram(3, 7);
        return true;
    }

    /// Everything the blend and texture unit caches hold is stale after an untextured draw.
    inline void invalidateStateCaches() {
        FP::getData<int>(0x005ac92c) = -1;
        FP::getData<int>(0x005ac928) = -1;
        FP::getData<int>(0x005bb024) = 0;
        FP::getData<int>(0x005bb020) = 0;
    }

    inline void setVertexColor(uint32_t rgba) {
        glColor4ubv((const GLubyte*)&rgba);
    }

} //namespace

DisaProjectionManager* DisaProjectionManager::get() {
    return FP::getData<DisaProjectionManager*>(0x005CBBC0);
}

void FUN_0041fee0_glCallBlendList(int blendMode) {
    callBlendList(blendMode, 0x006e8198, 0);
}

void FUN_0041ff30_glCallBlendListUntextured(int blendMode) {
    callBlendList(blendMode, 0x006e8258, 4);
}

void FUN_00428120_glActiveTexture(const DisaTextureSPtr* texture) {
    if (texture->m_field0x18 > 3) return;
    glActiveTexture(GL_TEXTURE0 + texture->m_field0x18);
}

void FUN_00421bf0_glSetLightPosUniform(const glm::vec3& lightPos) {
    if (FP::getData<bool>(0x006e8cfc)) return;
    const GLuint programId = (&FP::getData<GLuint>(0x006e8878))[FP::getData<int>(0x006e8cf8)];
    if (const GLint location = glGetUniformLocation(programId, "lightPos"); location >= 0)
        glUniform3f(location, lightPos.x, lightPos.y, lightPos.z);
}

void FUN_00421f40_glSetInvViewMtxUniform(const glm::mat4& invViewMatrix) {
    if (FP::getData<bool>(0x006e8cfc)) return;
    const GLuint programId = (&FP::getData<GLuint>(0x006e8878))[FP::getData<int>(0x006e8cf8)];
    if (const GLint location = glGetUniformLocation(programId, "invViewMtx"); location >= 0)
        glUniformMatrix4fv(location, 1, GL_FALSE, (const GLfloat*)&invViewMatrix);
}

void FUN_00421ba0_glSetImageSizeUniform(float width, float height) {
    const GLuint programId = FP::getData<GLuint>(0x006e88b8);
    if (const GLint location = glGetUniformLocation(programId, "img_Width"); location >= 0)
        glUniform1f(location, width);
    if (const GLint location = glGetUniformLocation(programId, "img_Height"); location >= 0)
        glUniform1f(location, height);
}

void FUN_00426fb0_RenderObject(DisaRenderObject* object, bool skipGlSetup, EDisaRenderPass pass) {
    assert(object);
    const DisaRenderObjectManager* roManager = DisaRenderObjectManager::get();
    const glm::vec3 lightPos = -roManager->m_lookDir;

    /* The caller of the sorted list has already applied all of this and knows it is still
     * current, so only an immediate draw sets the state up here. */
    if (skipGlSetup) {
        switch (object->renderSettingsBitMask.stencilMode) {
        case 0:
            glDisable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 0, 0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            break;
        case 1:
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, object->renderSettingsBitMask.stencilRef, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            break;
        case 2:
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_NOTEQUAL, object->renderSettingsBitMask.stencilRef, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            break;
        case 3:
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_NOTEQUAL, object->renderSettingsBitMask.stencilRef, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            break;
        default: break; //a negative mode leaves the stencil alone
        }

        if (object->depthTestEnabled) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);
        glDepthMask(object->depthMaskEnabled);
        glPolygonOffset(0.0f, object->polygonOffset * FP::getData<float>(0x005ac924));
    }

    //a display list carries its own geometry and color, meshData is the list id
    if (object->vertexCount == 0) {
        setVertexColor(object->textureInfo.color);
        FUN_00421a30_glUseAndInitProgram(10, 17);

        if (!skipGlSetup) {
            if (pass == RenderPass_SSAO) {
                FUN_00421a30_glUseAndInitProgram(8, 15);
            }
            else if (pass == RenderPass_SSAOComposite) {
                FUN_00421a30_glUseAndInitProgram(9, 16);
            }
            else if (pass == RenderPass_Scene && object->renderFlags.isFullyLit) {
                FUN_00421a30_glUseAndInitProgram(5, 9);
                FUN_00421bf0_glSetLightPosUniform(lightPos);
            }
            else if (applyMaterialProgram(object->renderFlags)) {
                FUN_00421bf0_glSetLightPosUniform(lightPos);
            }
        }

        glCallList((GLuint)(uintptr_t)object->meshData);
        glGetError();
        if (!skipGlSetup) FUN_00421a30_glUseAndInitProgram(10, 17);

        setVertexColor(0);
        FP::getData<int>(0x005ac920) = -1;
        invalidateStateCaches();
        return;
    }

    if (!object->texture) {
        glDisable(GL_TEXTURE_2D);
        for (const GLenum unit : { GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2 }) {
            glActiveTexture(unit);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        if (!skipGlSetup) FUN_00421a30_glUseAndInitProgram(1, 2);
        FUN_0041ff30_glCallBlendListUntextured(object->renderSettingsBitMask.blendMode);
        FP::callFastFarA<void>(0x00420ed0, getPrimitiveMode(object->primitiveType), 0x8004u, object->vertexCount, object->meshData);
        if (!skipGlSetup) FUN_00421a30_glUseAndInitProgram(10, 17);

        invalidateStateCaches();
        return;
    }

    setVertexColor(0xffffffff);
    glEnable(GL_TEXTURE_2D);
    FUN_0041fee0_glCallBlendList(object->renderSettingsBitMask.blendMode);

    //a render target lives on unit 3 and is bound by whoever filled it, not here
    if (object->texture->m_field0x18 != 3) applyTexture(object->texture, object->textureInfo);

    const bool hasNormalMap = object->normalMap != nullptr;
    if (hasNormalMap) {
        applyTexture(object->normalMap, object->textureInfo);
        FUN_00428120_glActiveTexture(object->texture);
    }

    if (!skipGlSetup) FUN_00421a30_glUseAndInitProgram(10, 17);

    if (pass == RenderPass_ShadowDepth) {
        if (!skipGlSetup) {
            FUN_00421a30_glUseAndInitProgram(4, 8);
            FUN_00421bf0_glSetLightPosUniform(lightPos);
        }
    }
    //the two SSAO passes bind their program even for an immediate draw
    else if (pass == RenderPass_SSAO) {
        FUN_00421a30_glUseAndInitProgram(8, 15);
    }
    else if (pass == RenderPass_SSAOComposite) {
        FUN_00421a30_glUseAndInitProgram(9, 16);
    }
    else if (hasNormalMap) {
        if (!skipGlSetup) {
            if (pass == RenderPass_Scene && object->renderFlags.isFullyLit)
                FUN_00421a30_glUseAndInitProgram(5, 9);
            else
                FUN_00421a30_glUseAndInitProgram(3, 7);
            FUN_00421bf0_glSetLightPosUniform(lightPos);
        }
    }
    else if (object->useColorTint) {
        if (!skipGlSetup) FUN_00421a30_glUseAndInitProgram(0, 4);
    }
    else if (!skipGlSetup) {
        const bool isBlurPass = pass >= RenderPass_Blur && pass <= RenderPass_BlurAndDof;
        if (!isBlurPass || object->texture->m_field0x18 != 3) {
            FUN_00421a30_glUseAndInitProgram(0, 0);
        }
        else {
            FUN_00421a30_glUseAndInitProgram(7, 0xb + (pass - RenderPass_Blur));
            FP::getData<unsigned int>(0x008c83a0) |= 1;

            glm::mat4& invViewMatrix = FP::getData<glm::mat4>(0x008c83a8);
            invViewMatrix = glm::inverse(roManager->m_field0x7605c); //original uses FUN_0042D4C0
            FUN_00421f40_glSetInvViewMtxUniform(invViewMatrix);
        }
    }

    if (const uint32_t* imageData = object->texture->m_field0x4; imageData && imageData[0xe] == 2) {
        FUN_00421a30_glUseAndInitProgram(0, 1);
        const char* image = (const char*)object->texture->m_field0x4;
        FUN_00421ba0_glSetImageSizeUniform((float)*(const uint16_t*)(image + 0xbe), (float)*(const uint16_t*)(image + 0xc0));
    }

    //0x1000400 adds the tangent and bitangent arrays that a normal mapped program reads
    const unsigned int dataTypeBitMask = 0x8044u | (hasNormalMap ? 0x1000400u : 0u);
    FP::callFastFarA<void>(0x00420ed0, getPrimitiveMode(object->primitiveType), dataTypeBitMask, object->vertexCount, object->meshData);

    object->texture->m_field0xc = 0;
    object->texture->m_field0xe = 0;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
}
