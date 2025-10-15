#include "PG_DisaIdentifier.h"
#include <Util/PG_StringUtil.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>



unsigned int PG_DisaIdentifier::getNameAsNumber() const {
	try
	{
		return PG_StringUtil::toInt(getName());
	}
	catch (...)
	{
		return 0;
	}
}

bool PG_DisaIdentifier::build(const std::string& baseNamespace) {
	splitAt = 0;
	PG_SimpleWordNamespaceValidator()(value);
	if (value.empty()) return false;

	std::size_t pos = 0;
	PG_StringUtil::toLower(value);

	while ((pos = value.find("::", pos)) != std::string::npos) {
		if (splitAt != 0) {
			splitAt = 0; //is ininvalid
			return false;
		}
		splitAt = pos + 2;
		pos = splitAt;
	}

	if (splitAt == 0 && !baseNamespace.empty()) {
		value = baseNamespace + "::" + value;
		splitAt = baseNamespace.size() + 2;
		return true;
	}
	return isValid();
}

std::pair<uint8_t, uint32_t> PG_DisaIdentifier::splitScriptId(unsigned int id) {
	constexpr uint32_t scriptTypeRange = 1000000;
	return { id / scriptTypeRange, id % scriptTypeRange };
}

bool PG_DisaIdentifier::isCustomScriptId(std::pair<uint8_t, uint32_t> value) {
	switch (value.first) {
	case ScriptType_Magic: return isCustomMagicId(value.second);
	}

	return false;
}
