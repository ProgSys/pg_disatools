#include "PG_DisaSprite.h"

#include <Resources/PG_Material.h>
#include <Util/PG_StringUtil.h>
#include <glm/gtx/euler_angles.hpp>

struct PG_DisaSpriteKeyframe {
	unsigned int at;
	shfileLayers layers;
};

struct PG_DisaSpriteAnimation {
	unsigned short id; //basically the name 
	unsigned int length = 0.f;
	bool mirrorX = false;
	std::vector<PG_DisaSpriteKeyframe> keyframes;
};

class PG_DisaSpriteBridge: public PG_SpriteBridge {
public:

	void load(const PG_SH& sh) {
		spriteMat.load("M_DisaSprite");

		textures.clear();
		for (auto& sheet : sh.getSpriteSheets()) {
			textures.push_back(PG_Texture(PG_UNIQUE_RES("spriteSheet"), sheet, { false, PG_Texture::Nearest, PG_Texture::Repeat }));
		}

		colorTables.clear();
		for (auto& table : sh.getColortables()) {
			PG_RGBAImage image;
			image.setAll(table.size(), 1, (void*)table.data());
			colorTables.push_back(PG_Texture(PG_UNIQUE_RES("colorTable"), image, {false, PG_Texture::Nearest, PG_Texture::Repeat}));
		}

		cutouts = sh.getCutouts();

		
		for (auto it = sh.getAnimations().begin(); it != sh.getAnimations().end(); ++it) {
			PG_DisaSpriteAnimation animation;
			animation.id = it->id;

			const unsigned short endKeyframe = it != std::prev(sh.getAnimations().end()) ? std::next(it)->start_keyframe : sh.getKeyframes().size();
			for (unsigned short i = it->start_keyframe; i < endKeyframe; ++i) {
				const auto& keyframe = sh.getKeyframes()[i];
				if (keyframe.bundel_index >= sh.getLayers().size()) continue;
				animation.keyframes.emplace_back(animation.length, sh.getLayers()[keyframe.bundel_index]);
				animation.length += keyframe.duration;
			}

			if (animation.id == 1001 || animation.id == 1007 || animation.id == 1061 || animation.id == 1067) {
				animations.push_back(animation);

				animation.mirrorX = true;
				animation.id += 10000;
			}

			animations.push_back(std::move(animation));
		}
	}

	std::string getAnimationName(int index) const override { return std::to_string(animations[index].id); };
	int getAnimationIndex(const std::string& name) const override { 
		unsigned short id = PG_StringUtil::toInt(name);
		auto findIt = std::ranges::find(animations, id, &PG_DisaSpriteAnimation::id);
		if (findIt == animations.end()) return -1;
		return std::distance(animations.begin(), findIt);
	};
	int getEndFrame(int animationIndex) const override { return animationIndex < 0? 0: animations[animationIndex].length; }

	void clear() override {

	}

	void traverse(PG_Object* owner, const glm::mat4& modelMatrix, const PG_Geometry& geo,
		int animationIndex, float frame, const PG_SpriteTraverserser::TraverserseFunc& visitor) const override {

		if (textures.empty() || animations.empty() || animationIndex >= animations.size()) return;

		auto aniIt = std::next(animations.begin(), animationIndex);
		unsigned int frameI = unsigned int(std::floor(frame)) % aniIt->length;
		
		auto findKeyIt = std::ranges::find_if(aniIt->keyframes, [frameI](const PG_DisaSpriteKeyframe& k) { return k.at > frameI; });
		if (findKeyIt == aniIt->keyframes.end()) 
			findKeyIt = std::prev(aniIt->keyframes.end());
		else if(findKeyIt != aniIt->keyframes.begin())
			findKeyIt = std::prev(findKeyIt);
		
		//render all
		int colorTableOffset = 0;
		if (auto value = owner->getPropertyValue("colorTableOffset")) {
			colorTableOffset = value.toInt();
			colorTableOffset = glm::clamp(colorTableOffset, 0, (int)colorTables.size() - 1);
		}

		spriteMat.set("colorTable", colorTables[colorTableOffset]);
		float i = 0.f;
		for (auto it = cutouts.begin() + findKeyIt->layers.start_cutout; it != cutouts.begin() + (findKeyIt->layers.start_cutout + findKeyIt->layers.number_of_cutouts); ++it) {
			if (it->external_sheet > 0) continue;
			spriteMat.set("colortableStart", it->colortable * 16);

			glm::mat4 mat = modelMatrix;
			mat = glm::translate(mat, glm::vec3(0.f, 0.f, i));
			i -= 0.01f; //to combat at z-fighting
			if (aniIt->mirrorX) {
				mat = glm::rotate(mat, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			}
			mat = glm::translate(mat, glm::vec3(it->offsetx / 50.f, it->offsety / 50.f, 0.f));
			mat = mat * glm::eulerAngleYXZ(0.f, 0.f, glm::radians((float)it->rotation));

			mat = glm::scale(mat, glm::vec3(it->scalex / 100.f, it->scaley / 100.f, 0.f));
			mat = glm::translate(mat, glm::vec3(it->anchorx / 50.f, it->anchory / 50.f, 0.f));
			


			visitor(nullptr, geo, spriteMat, textures[it->sheet], glm::vec2(it->x, it->y), glm::vec2(it->width, it->height),
				it->transparency/128.f, PG_SpriteEnums::EStencilDrawMode::IgnoreStencil, false, mat, nullptr);
		}

		
	};

	mutable PG_Material spriteMat;
	std::vector<PG_Texture> textures;
	std::vector<PG_Texture> colorTables;

	std::vector<shfileCutout> cutouts;
	std::vector<PG_DisaSpriteAnimation> animations;
};

PG_DisaSprite::PG_DisaSprite(unsigned short spriteID, const PG_SH& sh) {
	load(spriteID, sh);
}

PG_DisaSprite::PG_DisaSprite(unsigned short spriteID, PG_SH&& sh) {
	load(spriteID, std::move(sh));
}

void PG_DisaSprite::load(unsigned short spriteID, const PG_SH& sh) {
	std::string name = "DisaSprite" + std::to_string(spriteID);

	PG_Sprite_LoadFunc loadFunc = [sh]() -> std::variant<PG_SpriteData, PG_SpriteBridge*> {
		auto* bridge = new PG_DisaSpriteBridge();
		bridge->load(sh);
		return bridge;
	};

	PG_Sprite::load(name, loadFunc);
}

void PG_DisaSprite::load(unsigned short spriteID, PG_SH&& sh) {
	std::string name = "DisaSprite" + std::to_string(spriteID);

	PG_Sprite_LoadFunc loadFunc = [spriteID, sh = std::move(sh)]() -> std::variant<PG_SpriteData, PG_SpriteBridge*> {
		auto* bridge = new PG_DisaSpriteBridge();
		bridge->load(sh);
		return bridge;
		};

	PG_Sprite::load(name, loadFunc);
}

