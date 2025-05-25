#include "PG_DisaActorNode.h"
#include <Scene/PG_NodeTypes.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Disa/PG_DisaMapGeometryHolder.h>

#include <optional>



PG_DisaActorNode::PG_DisaActorNode(const std::string& name) :
	PG_ActorNode(name, PG_TypeInfo::create<PG_DisaActorNode>(PG_Type::Node_DisaActor)) {}

PG_DisaActorNode::PG_DisaActorNode(const std::string& name, short actorId) :
	PG_ActorNode(name, PG_TypeInfo::create<PG_DisaActorNode>(PG_Type::Node_DisaActor)), m_actorId(actorId) {}


void PG_DisaActorNode::init() {
	PG_ActorNode::init();

	auto model = PG_new<PG_ModelNode>("N_ActorBillboard", "G_ActorBillboard", std::vector<PG_Model::Surface>{
					{
						PG_Material("M_DisaActorBillboard"),
						PG_Geometry("Geo_ActorBillboard", GL::Geometry::planeXY(1.f, 1.0))
					}
		}, this);
	model->setTranslationY(1.f);
	model->setEulerAngleZ(180.0f);

	auto text = PG_new<PG_GuiText>("N_ActorIdText", glm::vec2(0.f), glm::vec2(0.f), this);
	text->setText(std::to_string(m_actorId));
	text->setFontSize(3.f);
	text->setHorizontalAlignment(PG_GuiText::ETextAlignment::HorizontalCenter);
	text->setVerticalAlignment(PG_GuiText::ETextAlignment::VerticalCenter);
	text->setWrapMode(PG_GuiText::EWrapMode::None);
	text->setMaterial(PG_Material("M_DisaActorTextSDF"));
	text->setColor(glm::vec3(1.0f));
	text->setBorderColor(glm::vec3(0.1f));
	text->setEulerAngleZ(180.0f);
	text->doUpdate();
	text->setTranslationY(0.85f);
	connect(this, &PG_Object::parentChanged, this, &PG_DisaActorNode::onParentChanged);
}

void PG_DisaActorNode::onParentChanged(PG_Object* oldManager, PG_Object* newManager)  {
	/*
	if (auto map = dynamic_cast<PG_DisaMapNode*>(newManager)) {

		glm::vec3 offset(glm::floor(getTranslation().x), 0.f, glm::floor(getTranslation().z));
		if (auto tile = map->findTile(offset.x, offset.z)) {
			const std::string name = getName() + "(" + std::to_string(int(offset.x)) + "," + std::to_string(int(offset.z)) + ")";

			PG_DisaTileTopGeometry geo;
			tile->extractTopGeometry(geo);
			if (geo.surfaces.empty()) 
				return;
	
			float yHeight = std::numeric_limits<float>::lowest();
			for (glm::vec4& v : geo.surfaces.front().vertices) {
				v.x -= offset.x + 0.5f;
				v.z -= offset.z + 0.5f;
				if (v.y > yHeight) yHeight = v.y;
			}
			for (glm::vec4& v : geo.surfaces.front().vertices) {
				v.y -= yHeight;
			}
			setTranslationY(yHeight);

			std::vector<glm::vec3> normals(geo.surfaces.front().vertices.size(), glm::vec3(0.0f, 1.0f, 0.0f));

			auto model = PG_new<PG_ModelNode>("N_ActorGround", "G_"+ name, std::vector<PG_Model::Surface>{
			{
				PG_Material("M_DisaActorGround"),
				PG_Geometry("Geo_" + name, 
				GL::Geometry::basic(std::move(geo.surfaces.front().indices), std::move(geo.surfaces.front().vertices), std::move(normals), std::move(geo.surfaces.front().uv))
				)
			}
				}, this);
			model->setTranslationY(0.02f);
			model->moveToBack();
		}

	}
	*/
}