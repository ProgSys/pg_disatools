#include "PG_DisaTranslationModel.h"
#include <Util/PG_MD5.h>
#include <Util/PG_StringUtil.h>
#include <Resources/PG_ResourceManager.h>
#include <Resources/PG_ResourceSource.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <Disa/utils/PG_DisaModInfo.h>
#include <Disa/utils/PG_DisaModLoadInfo.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <algorithm>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
	/// Calls unary for every line. A '\r' right after a '\n' is skipped, one before it stays in the line.
	void forEachLine(const std::vector<char>& data, const std::function<void(const std::string&)>& unary) {
		auto begin = data.begin();
		while (begin != data.end()) {
			const auto end = std::find(begin, data.end(), '\n');
			const std::string line(begin, end);
			if (end == data.end()) {
				if (!line.empty()) unary(line);
				return;
			}
			unary(line);
			begin = std::next(end);
			if (begin != data.end() && *begin == '\r') ++begin;
		}
	}

	/// Parses a Shift-JIS code given as hex, e.g. "818F" or "0x818F".
	std::optional<unsigned short> parseShiftJisHex(const std::string& str) {
		if (str.empty()) return std::nullopt;
		try {
			size_t pos = 0;
			const unsigned long value = std::stoul(str, &pos, 16);
			if (pos != str.size() || value == 0 || value > 0xFFFF) return std::nullopt;
			return static_cast<unsigned short>(value);
		} catch (const std::exception&) {
			return std::nullopt;
		}
	}

	/// Packs a single UTF-8 encoded char little-endian (first byte lowest), matching utf8ToShiftJis/shiftJisToUtf8.
	uint32_t packUtf8Char(const std::string& str) {
		uint32_t value = 0;
		for (size_t i = 0; i < str.size(); ++i) {
			value |= static_cast<uint32_t>(static_cast<unsigned char>(str[i])) << (i * 8);
		}
		return value;
	}
} //namespace

void PG_DisaTranslationModel::loadTranslation(PG_DisaModInfo* modInfo, const PG_DisaModLoadInfo& metaInfo) {
	if (!modInfo || modInfo->id.empty()) return;

	auto findBlockIt = metaInfo.json.find("translation");
	if (findBlockIt == metaInfo.json.end()) return;
	auto& j = *findBlockIt;

	// Pre-added: U+2665 (♥), U+3010 (【), U+3011 (】)
	std::string specialChars = "\xE2\x99\xA5" "\xE3\x80\x90" "\xE3\x80\x91";
	if (auto findIt = j.find("files"); findIt != j.end()) {
		for (auto& jTranslations: *findIt) {
			std::string fileName;
			jTranslations.get_to(fileName);
			auto handle = modInfo->open("translation/" + fileName);
			if (!handle) throw std::runtime_error("Failed to find 'translation/" + fileName + "' in zip!");
			forEachLine(handle.readAll(), [this, &specialChars](const std::string& line) {
				auto splits = splitUtf8Simple(line, ';');
				if (splits.empty()) return;
				if (splits.back() == "\r") splits.pop_back();
				if (splits.size() < 2) return;
				if (splits[0].empty() || splits[1].empty()) return;
				if (splits[0] == "0") {
					specialChars.append(splits[1]);
				}
				m_translations[std::move(splits[0])] = std::move(splits[1]);
			});
		}
	}

	if (auto findIt = j.find("fontOffsetX"); findIt != j.end()) {
		findIt->get_to(m_fontOffset.x);
	}
	if (auto findIt = j.find("fontOffsetY"); findIt != j.end()) {
		findIt->get_to(m_fontOffset.y);
	}

	if (auto findIt = j.find("fontScaleX"); findIt != j.end()) {
		findIt->get_to(m_fontScale.x);
	}
	if (auto findIt = j.find("fontScaleY"); findIt != j.end()) {
		findIt->get_to(m_fontScale.y);
	}

	if (auto findIt = j.find("iconOffsetX"); findIt != j.end()) {
		findIt->get_to(m_iconOffset.x);
	}
	if (auto findIt = j.find("iconOffsetY"); findIt != j.end()) {
		findIt->get_to(m_iconOffset.y);
	}

	// Shift-JIS <-> UTF-8 replacements. The first entry for a UTF-8 char is used to map back.
	// Default: the game renders the yen signs as a heart, so map them to U+2665 (♥).
	if (auto findIt = j.find("replace"); findIt != j.end()) {
		for (auto& jReplace : *findIt) {
			std::string shiftJisStr, utf8Str;
			if (auto it = jReplace.find("shiftJis"); it != jReplace.end()) it->get_to(shiftJisStr);
			if (auto it = jReplace.find("utf8"); it != jReplace.end()) it->get_to(utf8Str);

			const auto shiftJis = parseShiftJisHex(shiftJisStr);
			const auto utf8Encoded = parseCodePointToUtf8(utf8Str);
			if (!shiftJis || !utf8Encoded) {
				OUT_ERROR("Invalid translation replace entry: shiftJis '" << shiftJisStr << "', utf8 '" << utf8Str << "'!");
				continue;
			}
			const uint32_t utf8 = packUtf8Char(*utf8Encoded);
			m_swiftJisToUtf8[*shiftJis] = utf8;
			m_utf8ToSwiftJis.emplace(utf8, *shiftJis);
			specialChars.append(*utf8Encoded);
		}
	} else {
		constexpr uint32_t utf8Heart = 0xA599E2; // E2 99 A5
		m_swiftJisToUtf8[0x818F] = utf8Heart; // U+FFE5 (￥)
		m_swiftJisToUtf8[0x005C] = utf8Heart; // U+00A5 (¥)
		m_utf8ToSwiftJis.emplace(utf8Heart, 0x818F);
	}

	if (auto findIt = j.find("font"); findIt != j.end()) {
		std::string fontPath;
		findIt->get_to(fontPath);
		//"::" in front, or a font the mod doesn't carry, is one of the fonts resource folder, like "Disa/DisaFont-Regular.ttf"
		const bool isGlobal = fontPath.starts_with("::");
		if (isGlobal) fontPath.erase(0, 2);
		auto handle = isGlobal ? PG_ResourceHandle() : modInfo->open("fonts/" + fontPath);
		std::vector<char> fontData = handle ? handle.readAll() : std::vector<char>();
		if (!fontData.empty()) {
			m_font.load(fontPath, std::move(fontData));
		} else if (PG_ResourceManager::requestOpenInFolder(PG_ResourceFolder_Fonts, fontPath)) {
			m_font.load(fontPath);
		} else {
			OUT_ERROR("Failed to find font: 'fonts/" << fontPath << "' in the mod or the resources!");
			return;
		}
		m_font.requestCharsUtf8(specialChars);
	}
}

char* PG_DisaTranslationModel::translate(char* shiftJis, bool fullSize) {
	if (!shiftJis) return nullptr;
	emit stringIn(shiftJis);
	if (!m_enabled) return nullptr;
	auto hash = PG_MD5::getZeroTerminated(shiftJis);
	if (auto* translation = m_translationsMemory.at(hash)) {
		return translation->copy.data();
	}
	//search string
	auto findIt = m_translations.find(hash);
	if (findIt == m_translations.end() || findIt->second.empty()) return nullptr;

	auto translation = utf8ToShiftJis(findIt->second, fullSize, [this](uint32_t utf8Char) -> unsigned short {
		auto findIt = m_utf8ToSwiftJis.find(utf8Char);
		if (findIt != m_utf8ToSwiftJis.end()) return findIt->second;
		//add new entry
		++m_currSwiftJis;
		m_utf8ToSwiftJis[utf8Char] = m_currSwiftJis;
		m_swiftJisToUtf8[m_currSwiftJis] = utf8Char;
		return m_currSwiftJis;
		});
	if (translation.empty()) return nullptr;


	auto& mem = m_translationsMemory.emplace(hash);
	mem.originalLength = getShiftJisLength(shiftJis);
	mem.it = findIt;
	mem.copy.resize(translation.size() + 2, 0);
	memcpy(mem.copy.data(), translation.c_str(), translation.size());
	mem.newLength = getShiftJisLength(mem.copy.data());

	return mem.copy.data();
}

std::pair<std::string, std::string> PG_DisaTranslationModel::tranlatedUtf8(char* shiftJis) {
	if (!shiftJis) return {};
	emit stringIn(shiftJis);
	//if (!m_enabled) return {};
	auto hash = PG_MD5::getZeroTerminated(shiftJis);

	auto findIt = m_translations.find(hash);
	if (findIt == m_translations.end() || findIt->second.empty()) 
		return { std::move(hash), shiftJisToUtf8(shiftJis, [this](unsigned short shiftJisChar) -> uint32_t {
				auto findIt = m_swiftJisToUtf8.find(shiftJisChar);
				return findIt == m_swiftJisToUtf8.end()? 0 : findIt->second;
				}) };
	return { std::move(hash), findIt->second };
}

bool PG_DisaTranslationModel::hasMarkes(char* shiftJis) const {
	if (!shiftJis) return false;
	unsigned char c;
	while (c = *shiftJis) {
		if (c <= 0x7F || (c >= 0xA0 && c <= 0xDF) ) {
			++shiftJis; //single
			continue;
		} 

		unsigned short charShift = (c << 8) | (*(shiftJis + 1) & 0xFF);
		if (m_swiftJisToUtf8.contains(charShift)) return true;
		shiftJis += 2;
	}
	return false;
}