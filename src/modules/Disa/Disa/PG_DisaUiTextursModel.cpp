#include "PG_DisaUiTextursModel.h"
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_TX2.h>

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


void PG_DisaUiTextursModel::open(std::shared_ptr<PG_OLA> startDat) {
	auto entry = startDat->find(PG_File("ITEM.TX2"), false);

	PG_TX2 tx2;
	auto reader = entry.createReader();
	if (tx2.open(reader.get())) {
		m_iconsTexture.load("DisaIconsAtlas", [tex = std::move(tx2)]() -> PG_Texture::ImageFormats {
			PG_RGBAImage out;
			decompressTX2(tex, out);
			return out;
			});
	}
	else {
		m_iconsTexture.reset();
	}
}

PG_DisaUiTextursModel* PG_DisaUiTextursModel::get() {
	return g_global;
}