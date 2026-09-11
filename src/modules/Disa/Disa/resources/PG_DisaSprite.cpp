#include "PG_DisaSprite.h"

#include <Resources/PG_Material.h>
#include <Util/Debug/PG_DebugOut.h>
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

	std::string getAnimationName(int index) const override {
		if (index < 0 || index >= animations.size()) return {};
		return std::to_string(animations[index].id); 
	};
	int getAnimationIndex(const std::string& name) const override { 
		unsigned short id = PG_StringUtil::toInt(name);
		auto findIt = std::ranges::find(animations, id, &PG_DisaSpriteAnimation::id);
		if (findIt == animations.end()) return -1;
		return std::distance(animations.begin(), findIt);
	};
	int getEndFrame(int animationIndex) const override {
		if (animationIndex < 0 || animationIndex >= (int)animations.size()) return 0;
		return animations[animationIndex].length;
	}
	int getAnimationSize() const override { return (int)animations.size(); }

	void clear() override {

	}

	void traverse(PG_Object* owner, const glm::mat4& modelMatrix, const PG_Geometry& geo,
		int animationIndex, float frame, const PG_SpriteTraverserser::TraverserseFunc& visitor) const override {

		//a sheet the preview is pointed at may be anything, and a missing colortable is read below
		if (textures.empty() || colorTables.empty() || animations.empty() || animationIndex >= animations.size()) return;

		auto aniIt = std::next(animations.begin(), animationIndex);
		if (aniIt->keyframes.empty()) return;
		unsigned int frameI = aniIt->length != 0? unsigned int(std::floor(frame)) % aniIt->length: 0;
		
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

void PG_DisaSprite::load(const std::string& name, std::shared_ptr<PG::STREAM::In> reader) {
	PG_Sprite_LoadFunc loadFunc = [name, reader = std::move(reader)]() mutable -> std::variant<PG_SpriteData, PG_SpriteBridge*> {
		PG_SH sh;
		const bool result = sh.open(reader.get());
		reader.reset();
		if (!result) {
			OUT_ERROR("Failed to read the sprite sheet \"" << name << "\"!");
			return PG_SpriteData();
		}

		auto* bridge = new PG_DisaSpriteBridge();
		bridge->load(sh);
		return bridge;
		};

	PG_Sprite::load(name, loadFunc);
}

namespace {

/// Resolves one sheet index the way the fragment shader of M_DisaSprite does, but without reading off the end
glm::rgba colourAt(const PG_RGBAImage& table, int block, unsigned char id) {
	const unsigned int at = (unsigned int)block * 16 + id;
	return at < table.size() ? table.get(at, 0) : glm::rgba(0);
}

/// The bridge behind the sprite, null when it holds something else or has not loaded yet
const PG_DisaSpriteBridge* getBridge(const PG_Sprite& sprite) {
	if (!sprite.isLoaded()) return nullptr; //getData has nothing to hand out before then
	auto* held = std::get_if<PG_SpriteBridge*>(&sprite.getData());
	return held ? dynamic_cast<const PG_DisaSpriteBridge*>(*held) : nullptr;
}

/**
* @brief Reads a texture back out of graphics memory.
* @details Blocks until the render thread has downloaded it, so never on the render thread and never per
* frame. The format is asked for rather than guessed, a sheet of indices must not come back as colours.
*/
template<typename T>
T download(const PG_Texture& texture) {
	PG_Texture::ExportSettings settings;
	settings.forceFormat = typeid(typename T::Type);

	PG_Texture::ImageFormats image = texture.exportImage(settings).get();
	T* downloaded = std::get_if<T>(&image);
	return downloaded ? std::move(*downloaded) : T();
}

/// The colour table a sprite resolves through, empty when it has none or it could not be read back
PG_RGBAImage downloadColorTable(const PG_DisaSpriteBridge& bridge, int activeColorTable) {
	if (bridge.colorTables.empty()) return {};
	return download<PG_RGBAImage>(bridge.colorTables[glm::clamp(activeColorTable, 0, (int)bridge.colorTables.size() - 1)]);
}

/// One sheet resolved against an already downloaded colour table
PG_RGBAImage composeSheet(const PG_DisaSpriteBridge& bridge, int sheetIndex, const PG_RGBAImage& table) {
	const PG_UCharImage ids = download<PG_UCharImage>(bridge.textures[sheetIndex]);
	if (ids.empty()) {
		OUT_ERROR("Failed to read sprite sheet " << sheetIndex << " back!");
		return {};
	}

	//block 0 is what a pixel outside every cutout gets, and what a cutout on block 0 wants anyway
	PG_RGBAImage out(ids.getWidth(), ids.getHeight());
	for (unsigned int y = 0; y < ids.getHeight(); ++y)
		for (unsigned int x = 0; x < ids.getWidth(); ++x)
			out.get(x, y) = colourAt(table, 0, ids.get(x, y));

	//where two cutouts overlap the later one wins, the format has no way to say what should happen there
	for (const shfileCutout& cutout : bridge.cutouts) {
		if (cutout.external_sheet > 0 || (int)cutout.sheet != sheetIndex || cutout.colortable == 0) continue;

		const unsigned int endX = std::min<unsigned int>(cutout.x + cutout.width, ids.getWidth());
		const unsigned int endY = std::min<unsigned int>(cutout.y + cutout.height, ids.getHeight());
		for (unsigned int y = cutout.y; y < endY; ++y)
			for (unsigned int x = cutout.x; x < endX; ++x)
				out.get(x, y) = colourAt(table, cutout.colortable, ids.get(x, y));
	}

	return out;
}

}

int PG_DisaSpriteSheets::getSize(const PG_Sprite& sprite) {
	const PG_DisaSpriteBridge* bridge = getBridge(sprite);
	return bridge ? (int)bridge->textures.size() : 0;
}

PG_RGBAImage PG_DisaSpriteSheets::buildImage(const PG_Sprite& sprite, int sheetIndex, int activeColorTable) {
	const PG_DisaSpriteBridge* bridge = getBridge(sprite);
	if (!bridge || sheetIndex < 0 || sheetIndex >= (int)bridge->textures.size()) return {};

	const PG_RGBAImage table = downloadColorTable(*bridge, activeColorTable);
	if (table.empty()) {
		OUT_ERROR("Failed to read the colortable of \"" << sprite.getName() << "\"!");
		return {};
	}

	return composeSheet(*bridge, sheetIndex, table);
}

std::vector<PG_RGBAImage> PG_DisaSpriteSheets::buildImages(const PG_Sprite& sprite, int activeColorTable) {
	const PG_DisaSpriteBridge* bridge = getBridge(sprite);
	if (!bridge) return {};

	//read back once and resolve every sheet through it, rather than once per sheet
	const PG_RGBAImage table = downloadColorTable(*bridge, activeColorTable);
	if (table.empty()) {
		OUT_ERROR("Failed to read the colortable of \"" << sprite.getName() << "\"!");
		return {};
	}

	std::vector<PG_RGBAImage> out;
	out.reserve(bridge->textures.size());
	for (int i = 0; i < (int)bridge->textures.size(); ++i) out.push_back(composeSheet(*bridge, i, table));
	return out;
}

