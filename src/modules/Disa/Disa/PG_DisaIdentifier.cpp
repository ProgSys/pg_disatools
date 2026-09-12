#include "PG_DisaIdentifier.h"
#include <Util/PG_StringUtil.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>
#include <sstream>

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

PG_DisaIdentifier PG_DisaIdentifier::disaIdentifierFromIds(unsigned int ids) {
	return PG_DisaIdentifier(std::to_string(ids), "disa");
}

PG_DisaIdentifier PG_DisaIdentifier::disaIdentifierFromIds(unsigned int a, unsigned int b, unsigned int c) {
	std::stringstream stream;
	stream << a << "::" << b << "::" << c;
	return PG_DisaIdentifier(stream.str(), "disa");
}

std::pair<uint8_t, uint32_t> PG_DisaIdentifier::splitScriptId(unsigned int id) {
	constexpr uint32_t scriptTypeRange = 1000000;
	return { id / scriptTypeRange, id % scriptTypeRange };
}

bool PG_DisaIdentifier::isCustomScriptId(std::pair<uint8_t, uint32_t> value) {
	if (value.first > 9) return true;
	switch (value.first) {
	case ScriptType_Magic: return isCustomMagicId(value.second);
	}

	return false;
}

std::vector<int> PG_DisaIdentifier::getIds() const {
	std::vector<int> out;
	auto name = getName();
	if (name.empty()) return out;
	auto splits = PG_StringUtil::splitView(name, "::");
	if (splits.size() > 10) return out; //too big cant be a valid Identifier
	out.reserve(splits.size());
	int value;
	for (const std::string_view& split : splits) {
		try {
			value = PG_StringUtil::toIntView(split);
		}
		catch (...) {
			break;
		}
		out.push_back(value);
	}

	return out;
}

int PG_DisaIdentifier::toNumber() const {
	auto name = getName();
	if (name.empty()) return -1;
	try {
		return PG_StringUtil::toInt(name);
	}
	catch (...) {
		return -1;
	}
}