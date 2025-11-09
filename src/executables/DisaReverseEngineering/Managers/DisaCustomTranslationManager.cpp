#include "DisaCustomTranslationManager.h"
#include "DisaCustomBaseModManager.h"
#include <Util/PG_MD5.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>

void DisaCustomTranslationManager::test() {
	//m_translations["39a7768ec09c7bf1d2197d8296678dee"] = "\x41\xC3\x87\xC3\x9A\x43\x41\x52"; //sugar
	m_translations["39a7768ec09c7bf1d2197d8296678dee"] = "\x41\xC3\xA7\xC3\xBA\x63\x61\x72"; //sugar

	m_translations["39a7768ec09c7bf1d2197d8296678dee"] = "\x43\x6F\x6D\x65\xC3\xA7\x61\x72";// Start
	m_translations["5e422c271d6d1e950e5b6b0f4bd57b97"] = "\x43\x6F\x6E\x74\x69\x6E\x75\x61\x72";// Continue
	m_translations["52e0c986db07c4936675a2f80915afe7"] = "\x43\x6F\x6E\x66\x69\x67\x75\x72\x61\xC3\xA7\xC3\xB5\x65\x73";// Settings
	m_translations["aec95bec8bf6db33d94d93e0c1cb2ab8"] = "\x53\x61\x69\x72";// Exit
}

bool DisaCustomTranslationManager::translateMark(char*& shiftJis) {
	if (m_translations.empty() || !shiftJis || isTranslationMarked(shiftJis)) return false;
	auto hash = PG_MD5::getZeroTerminated(shiftJis);
	//search string
	auto findIt = m_translations.find(hash);
	if (findIt == m_translations.end()) return false;


	bool markerSingle = *((unsigned short*)shiftJis) <= 0x7F;
	if (markerSingle) {
		//single byte
		m_last = m_currentMarkSingle;
		if (m_currentMarkSingle >= 0x20)  m_currentMarkSingle = 0x01;
		++m_currentMarkSingle;
	}
	else {
		m_last = m_currentMark;
		++m_currentMark;
		if (m_currentMark > 0x86FF) m_currentMark = 0x8600;
	}
	auto& entry = m_markers[m_last];
	entry.originalLength = static_cast<unsigned short>(getShiftJisLength(shiftJis));
	entry.it = findIt;
	entry.copy.resize(entry.originalLength * 2 + 2);

	//copy
	auto at = shiftJis;
	auto it = entry.copy.begin();
	while (*at) {
		*it = *at;
		++it;
		++at;
	}
	//add zero termination
	*it = '\0';
	++it;
	*it = '\0';

	if (markerSingle) {
		entry.copy.front() = m_last;
	}
	else {
		entry.copy[0] = static_cast<uint8_t>((m_last >> 8) & 0xFF);
		entry.copy[1] = static_cast<uint8_t>(m_last & 0xFF);
	}
	shiftJis = entry.copy.data();

	return true;
}

bool DisaCustomTranslationManager::isTranslationMarked(char* shiftJis) {
	return ((unsigned char)*shiftJis) < 0x20 || static_cast<uint8_t>(*shiftJis) == 0x86;
}

std::string& DisaCustomTranslationManager::getUtf8FromMark(char* shiftJis) {
	m_stringBuffer.clear();
	if (m_translations.empty() || m_last == 0) return m_stringBuffer;
	/*
	unsigned short marker;
	if (*((unsigned char*)shiftJis) < 0x20) {
		marker = *shiftJis;
	}
	else if (static_cast<uint8_t>(*shiftJis) == 0x86) {
		marker = *((unsigned short*)shiftJis);
		marker = (marker >> 8) | (marker << 8);
	}
	else {
		return m_stringBuffer;
	}
	*/

	auto findIt = m_markers.find(m_last);
	m_last = 0;
	if (findIt == m_markers.end()) return m_stringBuffer;

	size_t stringLength = getShiftJisLength(shiftJis);
	auto& [originalLength, it, strBuffer] = findIt->second;
	m_stringBuffer = it->second;
	if (stringLength == originalLength) return m_stringBuffer;
	const size_t at = std::round((stringLength / (float)originalLength) * (float)originalLength);
	resizeUtf8(m_stringBuffer, at);
	return m_stringBuffer;
}

char* DisaCustomTranslationManager::translate(char* shiftJis) {
	if (!shiftJis) return nullptr;
	auto hash = PG_MD5::getZeroTerminated(shiftJis);
	if (auto* translation = m_translationsMemory.at(hash)) {
		return translation->copy.data();
	}
	//search string
	auto findIt = m_translations.find(hash);
	if (findIt == m_translations.end() || findIt->second.empty()) return nullptr;

	auto translation = utf8ToShiftJis(findIt->second, [this](uint32_t utf8Char) -> unsigned short {
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
	mem.copy.resize(translation.size());
	memcpy(mem.copy.data(), translation.c_str(), translation.size());
	mem.newLength = getShiftJisLength(mem.copy.data());

	return mem.copy.data();
}

DisaCustomTranslationManager* DisaCustomTranslationManager::get() {
	return reinterpret_cast<DisaCustomTranslationManager*>(DisaCustomBaseModManager::get()->getTranslationModel());
}
