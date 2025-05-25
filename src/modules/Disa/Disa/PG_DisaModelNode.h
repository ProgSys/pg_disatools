#pragma once
#include <Scene/PG_DynamicModelNode.h>
#include <Disa/fileformats/PG_GEO.h>


class PG_DisaModelNode : public PG_DynamicModelNode {
	PG_OBJECT(PG_DisaModelNode);
	PG_PARENT(PG_DynamicModelNode);

	PG_PROPERTY(glm::vec2 animationUV GET getAnimationUV CHANGED textureScrollChanged);
	PG_PROPERTY(glm::vec2 textureScroll GET getTextureScroll SET setTextureScroll CHANGED textureScrollChanged);
	PG_PROPERTY(float globalAlpha GET getGlobalAlpha SET setGlobalAlpha CHANGED globalAlphaChanged);
signals:
	void textureScrollChanged();
	void globalAlphaChanged();
public:
	PG_DisaModelNode(const std::string& name);

	inline glm::vec2 getAnimationUV() const { return m_animationScroll; }

	inline glm::vec2 getTextureScroll() const { return m_textureScroll; }
	inline void setTextureScroll(const glm::vec2& scroll)  { 
		if (m_textureScroll != scroll) {
			m_textureScroll = scroll;
			emit textureScrollChanged();
		}
	}

	inline const PG_Transformation& getRootTransformation() const { return m_rootTransformation; }
	inline void setRootTransformation(PG_Transformation rootTransformation) {
		if (m_rootTransformation != rootTransformation) {
			m_rootTransformation = rootTransformation;
			m_aabb.clear();
			refreshTree();
			emit modelMatrixChanged(getModelMatrix());
		}
	}

	inline float getGlobalAlpha() const { return m_globalAlpha; }
	inline void setGlobalAlpha(const float& scroll) {
		if (m_globalAlpha != scroll) {
			m_globalAlpha = scroll;
			emit globalAlphaChanged();
		}
	}

	void load(const std::string& modelId, const PG_GEO& geo);
	void loadGlobal(const std::string& modelId);
	void loadGlobal(const std::string& modelId, const PG_GEO& geo);
	void traverseGeometry(const glm::mat4& modelMatrix, const TraverserseFunc& visitor, float distance) const override;

	void makeTransparent();
	void reverseTransparent();

	bool event(PG_Event* e) override;

	void updateVolume() override;
private:
	glm::vec2 m_animationScroll = glm::vec2(0.f);
	glm::vec2 m_textureScroll = glm::vec2(0.f);
	float m_globalAlpha = 1.f;
	PG_Transformation m_rootTransformation;
	PG_AABB m_aabb;
};