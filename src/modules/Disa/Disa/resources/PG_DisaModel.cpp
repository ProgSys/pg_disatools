#include "PG_DisaModel.h"
#include <Disa/PG_DisaTransformation.h>
#include <ranges>

static inline std::vector<PG_Model::Surface> loadSurfaces(const geofileMesh& geoModel, const std::string& modelId, int disaModelId, bool isBillboard) {
	std::vector<PG_Model::Surface> surfaces;

	std::vector<glm::vec3>::const_iterator vBegin = geoModel.vertices.begin(), vEnd;
	std::vector<glm::vec2>::const_iterator uBegin = geoModel.uv.begin(), uEnd;

	struct GeoData {
		enum Type : int {
			Normal = 0,
			Additive,
			Billboard,
			BillboardAdditive,
		};
		int textureIndex;
		Type type;
		float alpha = 1.f;
		bool isUvAnimated = false;
		std::vector<glm::vec3> vert;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;
	};
	std::vector<GeoData> geoDatas;
	geoDatas.reserve(geoModel.header.number_of_surfaces);

	for (const auto& surface : geoModel.surfaces) {
		if (surface.length <= 0)
			continue;

		const float alpha = surface.aMult / 255.0f;
		typename GeoData::Type type = GeoData::Normal;
		if ((surface.additive) != 0) {//TODO somekind of blend mode
			if (isBillboard)
				type = GeoData::BillboardAdditive;
			else
				type = GeoData::Additive;
		}
		else {
			if (isBillboard)
				type = GeoData::Billboard;
			//else
				//type = GeoData::Normal;
		}

		const bool isUvAnimated = surface.animated != 0;

		auto findIt = std::ranges::find_if(geoDatas, [type, textureIndex = surface.index, alpha](const GeoData& data) {
			return data.type == type && data.textureIndex == textureIndex && data.alpha == alpha;
			});

		if (findIt == geoDatas.end()) {
			geoDatas.push_back({ surface.index , type, alpha, isUvAnimated });
			findIt = std::prev(geoDatas.end());
		}
		GeoData& data = *findIt;
		//TODO optimize, no need to lead geometry data multiple times
		if (surface.index == 0 || geoModel.uv.empty()) {

			// == Geometry ==
			vEnd = std::next(vBegin, surface.length);

			std::transform(std::reverse_iterator(vEnd), std::reverse_iterator(vBegin), std::back_insert_iterator(data.vert),
				[&data](const glm::vec3& p) {
					data.uvs.push_back(glm::vec2(-p.x, p.z) / 12.0f);
					return glm::vec3(-p.x, -p.y, p.z); });

			for (auto it = data.vert.begin(); it != data.vert.end(); it += 3) {
				const glm::vec3 normal = glm::normalize(glm::cross(*std::next(it) - *it, *std::next(it, 2) - *it));
				data.normals.insert(data.normals.end(), 3, normal);
			}

			vBegin = vEnd;
		}
		else {

			// == Geometry ==
			vEnd = std::next(vBegin, surface.length);
			uEnd = std::next(uBegin, surface.length);

			std::transform(std::reverse_iterator(vEnd), std::reverse_iterator(vBegin), std::back_insert_iterator(data.vert),
				[](const glm::vec3& p) {
					return glm::vec3(-p.x, -p.y, p.z); });
			if (data.vert.size() > 3) {
				for (auto it = data.vert.begin(); it != data.vert.end(); it += 3) {
					const glm::vec3 normal = glm::normalize(glm::cross(*std::next(it) - *it, *std::next(it, 2) - *it));
					data.normals.insert(data.normals.end(), 3, normal);
				}
			}
			else {
				data.normals.insert(data.normals.end(), data.vert.size(), glm::vec3(0.0f, 1.0f, 0.0f));
			}

			std::copy(std::reverse_iterator(uEnd), std::reverse_iterator(uBegin), std::back_insert_iterator(data.uvs));

			vBegin = vEnd;
			uBegin = uEnd;
		}
	}

	int i = 0;
	bool skipNext = false;
	std::vector<PG_Texture> textures;
	for (const auto& tex : geoModel.textures) {
		if (skipNext) {
			skipNext = false;
			continue;
		}
		skipNext = geoModel.header.isNew && tex.header.hasNormal;
		const std::string idetifier = std::to_string(disaModelId) + std::to_string(i) + (geoModel.header.isNew ? "_NEW" : "");
		textures.push_back(PG_Texture(modelId + "Texture" + idetifier, tex.toImageRGBA()));
		++i;
	}


	for (GeoData& data : geoDatas) {
		if (!data.vert.empty()) {
			PG_Mesh geometry(
				{
					PG_Mesh::DataWrapper("position", std::move(data.vert)),
					PG_Mesh::DataWrapper("normal", std::move(data.normals)),
					PG_Mesh::DataWrapper("uv", std::move(data.uvs)),
				}
			);

			const std::string idetifier = std::to_string(disaModelId) + std::to_string(i) + (geoModel.header.isNew ? "_NEW" : "");
			// == Material ==
			PG_Material mat;
			if (data.textureIndex > 0) {
				const unsigned int index = (data.textureIndex - 1);
				if (index < textures.size()) {
					if (const auto& tex = textures[index]; tex.isValid()) {
						static std::array<std::string, 4> mats{ "M_DisaModel", "M_DisaModelAdditive", "M_DisaModelBillboard", "M_DisaModelBillboardAdditive" };
						mat.load(mats[data.type]);
						mat.makeInstance();
						mat.setBinding("textureSlot0", tex);
						mat.setBinding("baseAlpha", data.alpha);
						if (!data.isUvAnimated)
							mat.setBinding("animationUV", glm::vec2(0.0f));
					}
				}
			}

			if (!mat.isSet())
				mat.load("M_MapDefaultTile");

			surfaces.push_back({
					std::move(mat),
					std::move(PG_Geometry(modelId + "Surface" + idetifier, std::move(geometry)))
				});
		}
		++i;
	}
	return surfaces;
};

PG_Model::BasicDynamicModel PG_DisaModel::parse(const std::string& modelName, const PG_GEO& geo) {
	BasicDynamicModel dynamicModel;
	Animation animation;
	animation.name = "idle";
	animation.isLoop = true;
	animation.properties.emplace_back(PG_KeyframeType_Transformation, "transformation");
	animation.properties.emplace_back(PG_KeyframeType_Float, "animationAlpha");
	animation.properties.emplace_back(PG_KeyframeType_Vec3EulerAngles, "eulerAngels");


	//load geometry
	int i = 0;
	for (const geofileTreeNode& disaNode : geo.getTreeNodes()) {
		const bool isBillboard = disaNode.transformation_mode == 2 || disaNode.transformation_mode == 3;

		dynamicModel.nodes.emplace_back();
		PG_Model::Node& node = dynamicModel.nodes.back();
		node.index = i++;

		node.parentIndex = disaNode.parent_index - 1;

		if (disaNode.model_index >= 0) {
			node.meshIndex = dynamicModel.surfaces.size();
			const int modelIndex = glm::clamp((int)disaNode.model_index, 0, (int)geo.getModels().size() - 1);
			dynamicModel.surfaces.push_back(loadSurfaces(geo.getModels()[modelIndex], modelName, modelIndex, isBillboard));
		}

		if (disaNode.number_of_transformations <= 0 && disaNode.number_of_keyframes <= 0) continue;

		animation.channels.emplace_back(node.index);

		if (disaNode.number_of_transformations == 1) {
			const auto& trans = geo.getTransformations()[disaNode.transformation_index];
			node.transformation = PG_Transformation(
				fromDisaPosition(trans.position),
				fromDisaRotation(fromDisaRotationEuler(trans.rotation)),
				fromDisaScale(trans.scale)
			);
			if (disaNode.number_of_keyframes <= 0) {
				animation.channels.pop_back();
				continue;
			}
		}
		else {
			PG_Model::KeyframeWindow window{
					0, static_cast<int>(animation.transformationKeyframes.size()), static_cast<int>(animation.transformationKeyframes.size() + disaNode.number_of_transformations)
			};
			animation.channels.back().windows.emplace_back(window);
			window.propertyIndex = 2;
			animation.channels.back().windows.emplace_back(window);

			for (int i2 = (int)disaNode.transformation_index; i2 < (int)disaNode.transformation_index + disaNode.number_of_transformations; ++i2) {
				const auto& trans = geo.getTransformations()[glm::clamp(i2, 0, (int)geo.getTransformations().size() - 1)];
				animation.duration = std::max(animation.duration, (float)trans.times.x);
				animation.transformationKeyframes.emplace_back(
					(float)trans.times.x,
					PG_Transformation(
						fromDisaPosition(trans.position),
						fromDisaRotation(fromDisaRotationEuler(trans.rotation)),
						fromDisaScale(trans.scale)
					)
				);
				animation.vec3Keyframes.emplace_back((float)trans.times.x, fromDisaRotationEuler(trans.rotation));
			}

			std::ranges::sort(std::next(animation.transformationKeyframes.begin(), window.start), std::next(animation.transformationKeyframes.begin(), window.end), {}, &Keyframe<PG_Transformation>::frame);
		}

		if (disaNode.number_of_keyframes > 0) {
			PG_Model::KeyframeWindow window{
					1, static_cast<int>(animation.floatKeyframes.size()), static_cast<int>(animation.floatKeyframes.size() + disaNode.number_of_keyframes)
			};
			animation.channels.back().windows.emplace_back(window);
			for (int i2 = (int)disaNode.keyframe_index; i2 < (int)disaNode.keyframe_index + disaNode.number_of_keyframes; ++i2) {
				const auto& trans = geo.getKeyframes()[glm::clamp(i2, 0, (int)geo.getKeyframes().size() - 1)];
				animation.duration = std::max(animation.duration, (float)trans.time);
				animation.floatKeyframes.emplace_back(
					(float)trans.time, trans.value / 10000.0f
				);
			}

			std::ranges::sort(std::next(animation.floatKeyframes.begin(), window.start), std::next(animation.floatKeyframes.begin(), window.end), {}, &Keyframe<float>::frame);
		}
	}


	if (!animation.channels.empty()) {
		dynamicModel.animations.push_back(std::move(animation));
	}

	{ //create collision
		std::vector<glm::mat4> matrices(geo.getTreeNodes().size(), glm::mat4(1.f));
		for (auto& n : dynamicModel.nodes) {
			glm::mat4 mat = matrices[n.index];
			if (n.parentIndex >= 0) {
				mat = matrices[n.parentIndex] * n.transformation.getMatrix();
			}
			else {
				mat = n.transformation.getMatrix();
			}
			if (n.meshIndex >= 0) {
				for (const auto& surface : dynamicModel.surfaces[n.meshIndex]) {
					auto& geo = surface.geometry.getGeometry();
					geo.traverse<glm::vec3>("position", [&aabb = dynamicModel.collision, mat](const glm::vec3& p) {
						aabb.add(mat, p);
						});
				}
			}
		}
	}
	return dynamicModel;
}

void PG_DisaModel::load(const std::string& modelName, const PG_GEO& geo) {
	if (geo.getTransformations().empty() || geo.getModels().empty() || geo.getTreeNodes().empty()) {
		clear();
		return;
	}

	BasicDynamicModel dynamicModel = parse(modelName, geo);

	PG_Model::load(modelName, [dynamicModel = std::move(dynamicModel)] { return dynamicModel; });
}