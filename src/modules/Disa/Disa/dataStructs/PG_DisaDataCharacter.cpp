#include "PG_DisaDataCharacter.h"
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <Disa/fileformats/PG_JsonUtil.h>
using namespace PG_JsonUtil;

std::string PG_DisaDataCharacter::getName() const {
    return shiftJisToString(toShiftJisString(name));
}

void PG_DisaDataCharacter::setName(const std::string& str) {
    memset(name.data(), 0, name.size());
    if (str.empty() || str.size() > 100) return;

    const std::string shiftStr = stringToShiftJis(str);
    memcpy(name.data(), shiftStr.data(), std::min(shiftStr.size(), name.size() - 1));
}

std::string PG_DisaDataCharacter::getTitle() const {
    return shiftJisToString(toShiftJisString(title));
}

void PG_DisaDataCharacter::setTitle(const std::string& str) {
    memset(title.data(), 0, title.size());
    if (str.empty() || str.size() > 100) return;

    const std::string shiftStr = stringToShiftJis(str);
    memcpy(title.data(), shiftStr.data(), std::min(shiftStr.size(), title.size() - 1));
}

void PG_DisaDataCharacter::clear() {
    memset(this, 0, sizeof(PG_DisaDataCharacter));
}

void PG_DisaDataCharacter::fromJson(const nlohmann::json& j) {
    {
        std::string str;
        readFromJson(j, "name", str, 32);
        if (!str.empty()) setName(str);
    }
    {
        std::string str;
        readFromJson(j, "title", str, 32);
        if(!str.empty()) setTitle(str);
    }

    readFromJson(j, "type", type);
    readFromJson(j, "unknownValue0x3d", unknownValue0x3d);
    readFromJson(j, "unknownValue0x3e", unknownValue0x3e);
    readFromJson(j, "gender", gender);
    readFromJson(j, "unknownValue0x40", unknownValue0x40);
    readFromJson(j, "paletteIndex", paletteIndex);
    readFromJson(j, "learnSpeed", learnSpeed);
    readFromJson(j, "jumpPower", jumpPower);
    readFromJson(j, "weaponMasteryBaseStats", weaponMasteryBaseStats);

    if (auto findIt = j.find("stats"); findIt != j.end()) {
        stats.fromJson(*findIt);
    }

    readFromJson(j, "immunity", immunity);
    readFromJson(j, "characterId", characterId);
    readFromJson(j, "spriteId", spriteId);
    readFromJson(j, "unknownValue0x58", unknownValue0x58);
    readFromJson(j, "unknownValue0x60", unknownValue0x60);
    readFromJson(j, "throwingRange", throwingRange);
    readFromJson(j, "helpID", helpID);

    if (auto findIt = j.find("baseStats"); findIt != j.end()) {
        baseStats.fromJson(*findIt);
    }

    readFromJson(j, "experienceScale", experienceScale);
    if(experienceScale == 0){
        OUT_WARNING("\"experienceScale\" of " << getName() << " is zero! Level computation will produce wrong result!");
    }
    readFromJson(j, "movement", movement);
    readFromJson(j, "movementType", movementType);
    readFromJson(j, "unknownValue0x77", unknownValue0x77);
    readFromJson(j, "unknownValue0x79", unknownValue0x79);
    readFromJson(j, "unknownValue0x7b", unknownValue0x7b);
    readFromJson(j, "unknownValue0x7d", unknownValue0x7d);
    readFromJson(j, "unknownValue0x7f", unknownValue0x7f);
    readFromJson(j, "unknownValue0x81", unknownValue0x81);
    readFromJson(j, "unknownValue0x83", unknownValue0x83);
    readFromJson(j, "counter", counter);

    readFromJson(j, "skillIds", skillIds);
    readFromJson(j, "skillLevels", skillLevels);

}