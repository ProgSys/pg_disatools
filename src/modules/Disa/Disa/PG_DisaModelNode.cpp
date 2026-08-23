#include "PG_DisaModelNode.h"
#include <Scene/PG_NodeTypes.h>
#include <Util/Volume/PG_AABB.h>
#include <Base/Events/PG_TickEvent.h>
#include <Disa/resources/PG_DisaModel.h>


PG_DisaModelNode::PG_DisaModelNode(const std::string& name) :
	PG_DynamicModelNode(name, PG_TypeInfo::create<PG_DisaModelNode>(PG_Type::Node_DisaModel)) {
	setAnimationSpeed(0.06f);
	connect(this, &PG_DisaModelNode::loaded, this, [this] {m_aabb.clear(); });
}

void PG_DisaModelNode::load(const std::string& modelId, const PG_GEO& geo) {
	loadGlobal(PG_UNIQUE_RES("R_DisaModel") + modelId, geo);
}

void PG_DisaModelNode::loadGlobal(const std::string& modelId) {
	clearChildrenByType({ PG_Type::Node_Model,PG_Type::Node_DisaModel });
	PG_DisaModel model;
	model.PG_Model::load(modelId);
	setModel(model);
}

void PG_DisaModelNode::loadGlobal(const std::string& modelId, const PG_GEO& geo) {
	clearChildrenByType({ PG_Type::Node_Model,PG_Type::Node_DisaModel });
	if (geo.getTransformations().empty() || geo.getModels().empty() || geo.getTreeNodes().empty()) return;

	PG_DisaModel model;
	model.load(modelId, geo);
	setModel(model);
}

void PG_DisaModelNode::traverseGeometry(const glm::mat4& modelMatrix, const TraverserseFunc& visitor, float distance) const {
	if (!m_model.isLoaded() || m_model.getMeshSize() == 0) return;
	const auto properties = buildMatricesDefault(m_model, m_currentAnimation, m_frame);
	std::vector<glm::mat4> matrices = toMat4(properties);
	glm::mat4 rootTransformation = m_rootTransformation.getMatrix();
	for (const auto& node : m_model.getNodes() | std::views::reverse) {
		if (node.parentIndex >= 0) {
			matrices[node.index] = glm::inverse(matrices[node.parentIndex]) * matrices[node.index];
		}
	}
	for (const auto& node : m_model.getNodes()) {
		if (node.parentIndex < 0) {
			matrices[node.index] = matrices[node.index] * rootTransformation;
		}
		else {
			matrices[node.index] = matrices[node.parentIndex] * matrices[node.index];
		}
	}
	m_model.traverseGeometry(modelMatrix, matrices, properties, visitor, distance);
}

void PG_DisaModelNode::makeTransparent() {
	if (!m_model.isLoaded() || m_model.getMeshSize() == 0) return;
	setGlobalAlpha(0.5f);
	for (auto& m : m_model.getMaterials()) {
		m.addRenderFlag(PG_Material::RenderLast);
	}
}
void PG_DisaModelNode::reverseTransparent() {
	if (!m_model.isLoaded() || m_model.getMeshSize() == 0) return;
	setGlobalAlpha(1.0f);
	for (auto& m : m_model.getMaterials()) {
		m.removeRenderFlag(PG_Material::RenderLast);
	}
}

bool PG_DisaModelNode::event(PG_Event* e) {
	if (e->getType() == EEventTypes::PG_Event_Tick) {
		PG_TickEvent* tEvent = static_cast<PG_TickEvent*>(e);
		m_animationScroll = glm::mod(m_animationScroll + m_textureScroll * 0.5f * (float)tEvent->getDeltaTime(), 1.0f);

		//if model node has animations then refresh volume
		if (getModel() && !getModel().getAnimations().empty() 
			&& getAnimationIndex() >= 0 && getAnimationIndex() < getModel().getAnimations().size() 
			&& !getModel().getAnimations()[getAnimationIndex()].transformationKeyframes.empty()) {
			refreshTree();
		}
	}
	return PG_DynamicModelNode::event(e);
}

void PG_DisaModelNode::updateVolume() {
	m_aabb.clear();
	traverseGeometry(glm::mat4(1.f), [&aabb = m_aabb](const PG_Geometry& geo, const PG_Material&, const glm::mat4& mat, const std::map<std::string, PG_Any>&, bool, const std::vector<glm::mat4>&) {
		if (!geo) return;
		geo.addAABB(aabb, mat);
		}, 0.f);
	m_aabb.makeThick();
	if (m_aabb.isValid()) {
		getVolumeVisibility().setOBB(m_aabb.min, m_aabb.max);
		getVolumeCollision().setOBB(m_aabb.min, m_aabb.max);
	}

	PG_ActorNode::updateVolume();
}
