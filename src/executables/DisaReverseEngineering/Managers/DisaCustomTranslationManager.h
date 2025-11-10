#pragma once

#include <Disa/PG_DisaTranslationModel.h>
#include <unordered_map>
#include <list>

struct PG_DisaModLoadInfo;
struct PG_DisaModInfo;




class DisaCustomTranslationManager: public PG_DisaTranslationModel {
public:
	using PG_DisaTranslationModel::PG_DisaTranslationModel;

	bool toUtf8(const char* shiftJis, char* dest, size_t* outLength);

    void replaceSpecial(char*) const;

	void createBaseTranslation();
	void toggleTranslationRecording();

	static DisaCustomTranslationManager* get();

protected:
	std::string m_stringBuffer;
};