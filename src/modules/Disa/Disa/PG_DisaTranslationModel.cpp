#include "PG_DisaTranslationModel.h"
#include <Util/PG_MD5.h>
#include <Util/PG_StringUtil.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <Disa/utils/PG_DisaModInfo.h>
#include <Disa/utils/PG_DisaModLoadInfo.h>
#include <Disa/fileformats/PG_ZIPUtil.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>

void PG_DisaTranslationModel::loadTranslation(PG_DisaModInfo* modInfo, const PG_DisaModLoadInfo& metaInfo) {
	if (!modInfo || modInfo->id.empty()) return;

	auto& j = metaInfo.json;

	if (auto findIt = j.find("translations"); findIt != j.end()) {
		if (!modInfo->archive) {
			modInfo->archive = PG_ZIPUtil::openZip(modInfo->zipFile);
		}

		for (auto& jTranslations: *findIt) {
			std::string fileName;
			jTranslations.get_to(fileName);
			PG_ZIPUtil::readStringLines(modInfo->archive.get(), "translation/" + fileName, [this](const std::string& line) {
				auto splits = splitUtf8Simple(line, ';');
				if (splits.empty()) return;
				if (splits.back() == "\r") splits.pop_back();
				if (splits.size() < 2) return;
				if(splits[0].empty() || splits[1].empty())  return;
				m_translations[std::move(splits[0])] = std::move(splits[1]);
			});
		}

	}

	if (auto findIt = j.find("font"); findIt != j.end()) {
		std::string fontPath;
		findIt->get_to(fontPath);
		auto fontData = PG_ZIPUtil::readData(modInfo->archive.get(), "fonts/" + fontPath);
		if (fontData.empty()) {
			OUT_ERROR("Failed to find font: 'fonts/" << fontPath << "'!");
			return;
		}
		m_font.load(fontPath, std::move(fontData));
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
	if (!m_enabled) return {};
	auto hash = PG_MD5::getZeroTerminated(shiftJis);

	auto findIt = m_translations.find(hash);
	if (findIt == m_translations.end() || findIt->second.empty()) return {};
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