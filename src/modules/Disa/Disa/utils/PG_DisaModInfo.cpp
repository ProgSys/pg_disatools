#include <Disa/utils/PG_DisaModInfo.h>

#include <nlohmann/json.hpp>
#include <Disa/fileformats/PG_JsonUtil.h>
#include <Resources/PG_ResourceManager.h>
#include <Resources/PG_ResourceName.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>
#include <Util/PG_StringUtil.h>

bool PG_DisaModInfo::acquire() {
	if (resource) return true;
	if (namespaceMod.empty() || !PG_ResourceManager::exists()) return false;

	resource = PG_ResourceManager::registerSource<PG_ZIPResourceSource>(zipFile, namespaceMod);
	if (resource && !resource->isValid()) resource.expire();
	return static_cast<bool>(resource);
}

PG_ResourceHandle PG_DisaModInfo::open(const std::string& pathInZip) const {
	if (resource) return PG_ResourceManager::requestOpen(PG_ResourceName::join(namespaceMod, pathInZip));

	//built only for this read, it closes the zip right after
	const auto source = std::make_shared<PG_ZIPResourceSource>(zipFile, namespaceMod);
	return source->open(pathInZip);
}

std::string PG_DisaModInfo::resolveNameSpace(const nlohmann::json& j, const PG_File& zipFile) {
	std::string nameSpace;
	PG_JsonUtil::readFromJson(j, "namespace", nameSpace);
	if (nameSpace.empty()) PG_JsonUtil::readFromJson(j, "namesspace", nameSpace);
	return resolveNameSpace(std::move(nameSpace), zipFile);
}

std::string PG_DisaModInfo::resolveNameSpace(std::string declared, const PG_File& zipFile) {
	std::string nameSpace = declared.empty() ? zipFile.getName() : std::move(declared);
	nameSpace = PG_StringUtil::toLowerCopy(nameSpace);
	if (nameSpace.empty() || !PG_SimpleWordValidator()(nameSpace)) return {};
	return nameSpace;
}
