#pragma once

#include <Util/Volume/PG_AABB.h>

struct PG_DisaTileGeometry {
	struct Surface {
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> uvs;
		std::vector<glm::vec3> uvsDecal;
		std::vector<glm::vec3> uvsShadowMap;
		std::vector<unsigned int> indices;
	};
	std::vector<std::pair<glm::ivec3, Surface>> surfaces;
	PG_AABB aabb;
};

struct PG_DisaGeoPanelGeometry {
	struct Surface {
		std::vector<glm::vec4> vertices;
		std::vector<unsigned int> indices;
	};
	std::vector<Surface> surfaces;
};

struct PG_DisaTileTopGeometry {
	struct Surface {
		std::vector<glm::vec4> vertices;
		std::vector<unsigned int> indices;
		std::vector<glm::vec2> uv;
	};
	std::vector<Surface> surfaces;
};