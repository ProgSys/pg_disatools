#include "PG_DisaUiTextursModel.h"

static PG_DisaUiTextursModel* g_global = nullptr;

void PG_DisaUiTextursModel::init() {
	g_global = this;
}

glm::vec4 PG_DisaUiTextursModel::getIconLocation(unsigned short iconIndex, unsigned short iconType) {
	glm::vec4 out;
	out.x = (iconIndex % 16) * 16.f;
	out.y = (iconIndex / 16) * 16.f;
	if (iconIndex == 78 && iconType == 2) {
		out.z = 32.f;
		out.x -= out.z;
		out.w = 16.f;
	}else if (iconIndex == 76 && iconType == 3) {
		out.z = 32.f;
		out.w = 32.f;
		out.x += out.z;
	}
	else{
		out.z = 16.f;
		out.w = 16.f;
	}

	return out;
}


void PG_DisaUiTextursModel::open() {
	m_iconsTexture.load(std::string(iconsAtlasPath));
}

PG_DisaUiTextursModel* PG_DisaUiTextursModel::get() {
	return g_global;
}