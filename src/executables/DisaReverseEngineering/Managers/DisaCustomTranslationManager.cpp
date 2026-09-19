#include "DisaCustomTranslationManager.h"
#include "DisaCustomBaseModManager.h"
#include <DisaCharacterFunctions.h>
#include <Util/PG_MD5.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <Disa/PG_DisaTalkCompiler.h>
#include <DisaScriptManager.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInByteArray.h>
#include <unordered_set>

void DisaCustomTranslationManager::replaceSpecial(char* str) const{
	if (!str) return;
	char* read = str;
	
	unsigned char c;
	while (c = *read) {
		if (c <= 0x7F || (c >= 0xA0 && c <= 0xDF)) {
			*str = *read;
			++str;
			++read;
			continue;
		}

		++read;
		unsigned short ch = c << 8 | *read;
		if (m_swiftJisToUtf8.contains(ch)) {
			*str = 0x61;
			++str;
			++read;
			continue;
		}

		str += 2;
		++read;
	}
	*str = 0;
}

bool DisaCustomTranslationManager::toUtf8(const char* shiftJis, char* dest, size_t* outLength) {
	if (!shiftJis) return false;
	std::string str(shiftJis);
	str = shiftJisToUtf8(str, [this](unsigned short shiftJisChar) -> uint32_t {
		auto findIt = m_swiftJisToUtf8.find(shiftJisChar);
		if (findIt == m_swiftJisToUtf8.end()) return 0;
		return findIt->second;
		});
	if (str.empty()) return false;
	memset(dest, 0, str.size() + 2);
	memcpy(dest, str.c_str(), std::min(str.size(), 254u));
	*outLength = std::min(getUtf8Length(str), 254u);
	return true;
}

void DisaCustomTranslationManager::createBaseTranslation() {
	PG::STREAM::OutByteFile writer(DisaCustomBaseModManager::get()->getCustomTranslationDir().toFilePath("baseTranslation.csv"));
	writer << "0;;Character cache;Add your special characters here;\n";
	writer << "39a7768ec09c7bf1d2197d8296678dee;;Start;Main menu;\n";
	writer << "5e422c271d6d1e950e5b6b0f4bd57b97;;Continue;Main menu;\n";
	writer << "52e0c986db07c4936675a2f80915afe7;;Settings;Main menu;\n";
	writer << "aec95bec8bf6db33d94d93e0c1cb2ab8;;Exit;Main menu;\n";
	std::unordered_set<std::string> dublicateFilter;

	{
		for (const auto& charInfo : std::ranges::subrange(DisaDataCharacterInfo::begin(), DisaDataCharacterInfo::end())) {
				auto hash = PG_MD5::get(toShiftJisString(charInfo.name));
				if (dublicateFilter.insert(hash).second) {
					writer << hash << ";;" << charInfo.getName() << ";Character name "<< std::to_string(charInfo.characterId) <<"; \n";
				}
				hash = PG_MD5::get(toShiftJisString(charInfo.title));
				if (dublicateFilter.insert(hash).second) {
					writer << hash << ";;" << charInfo.getTitle() << ";Character title " << std::to_string(charInfo.characterId) << "; \n";
				}
		}
	}

	{
		for (const auto& info : std::ranges::subrange(DisaDataMagicInfo::begin(), DisaDataMagicInfo::end())) {
			auto hash = PG_MD5::get(toShiftJisString(info.name));
			if (dublicateFilter.insert(hash).second) {
				writer << hash << ";;" << info.getName() << ";Magic name " << std::to_string(info.id) << "; \n";
			}
			hash = PG_MD5::get(toShiftJisString(info.description));
			if (dublicateFilter.insert(hash).second) {
				writer << hash << ";;" << info.getDescription() << ";Magic description " << std::to_string(info.id) << "; \n";
			}
		}
	}
	{
		
		auto& scriptManager = DisaScriptManager::get();
		const PG_DisaTalkDatScriptEntry* startIt = scriptManager.talkScriptsEntries;
		const PG_DisaTalkDatScriptEntry* endIt = startIt + scriptManager.sizeTalkScripts;
		int i = 0;
		for (const PG_DisaTalkDatScriptEntry& entry : std::ranges::subrange(startIt, endIt)) {
			unsigned char* code = scriptManager.scriptsData + entry.startAddress;
			PG::STREAM::InByteArray reader((const char*)code, 500000);
			try {
				auto strs = PG_DisaTalkScript::findStrings(&reader);
				for (const auto& str : strs) {
					auto hash = PG_MD5::get(str);
					if (!dublicateFilter.insert(hash).second) continue;
					writer << hash<<";;" << shiftJisToString(str) <<";Talk Script "<< std::to_string(i) <<" (" << std::to_string(entry.scriptId) << ", " << std::to_string(entry.dungeonId) << ", " << std::to_string(entry.searchFlag) << "); \n";
				}
			}
			catch (const PG_CompileException& ex) {
				OUT_ERROR("Failed to decompile talk script "<<std::dec<< i <<": (" << (int)entry.scriptId << ", " << (int)entry.dungeonId << ", " << (int)entry.searchFlag << ") Reason: " << ex.what() << " At: "<<ex.getLine());
			}
			++i;
		}
	}

	writer.close();
}

class TranslationRecorder : public PG_Object {
public:
	using PG_Object::PG_Object;

	void init() {
		PG::STREAM::InByteFile reader(DisaCustomBaseModManager::get()->getCustomTranslationDir().toFilePath("recordTranslation.csv"));
		if (!reader.isOpen()) return;
		std::string line;
		while (!reader.eof()) {
			char c = reader.readChar();
			if (c == '\n' && !line.empty()) {
				auto splits = PG_StringUtil::split(line, ';');
				if (splits.size() > 3 && !splits[0].empty() && !splits[2].empty()) {
					recordsOld[std::move(splits[0])] = { std::move(splits[1]), std::move(splits[2]) };
				}
				line.clear();
			}
			else {
				line.push_back(c);
			}
		}
		reader.close();
	}

	void done() {
		if (records.empty()) return;
		auto file = DisaCustomBaseModManager::get()->getCustomTranslationDir().toFilePath("recordTranslation.csv");
		PG::STREAM::OutByteFile writer(file, PG::STREAM::OutByteFile::Append);
		for (const auto& [hash, strPair] : records)
			writer << hash << ";" << strPair.first << ";" << strPair.second << ";Record;\n";
		OUT_INFO("Record written to: "<< file);
	}

	void record(char* shiftJis) {
		auto hash = PG_MD5::getZeroTerminated(shiftJis);
		if (recordsOld.contains(hash) || records.contains(hash)) return;
		auto str = shiftJisToString(shiftJis);
		OUT_INFO("Recorded: \"" << str << "\" (" << hash << ")");
		records[hash] = { std::string(), std::move(str)};
	}

	std::unordered_map<std::string, std::pair<std::string, std::string>> recordsOld;
	std::unordered_map<std::string, std::pair<std::string, std::string>> records;
};


void DisaCustomTranslationManager::toggleTranslationRecording() {
	auto* recorder = findFirstChild<TranslationRecorder>("TranslationRecorder");
	if (recorder) {
		m_enabled = true;
		recorder->done();
		delete recorder;
		return;
	}
	recorder = PG_new<TranslationRecorder>("TranslationRecorder", this);
	connect(this, &DisaCustomTranslationManager::stringIn, recorder, &TranslationRecorder::record);
	m_enabled = false;
	OUT_INFO("=== Record started! ===");
}



DisaCustomTranslationManager* DisaCustomTranslationManager::get() {
	return reinterpret_cast<DisaCustomTranslationManager*>(DisaCustomBaseModManager::get()->getTranslationModel());
}
