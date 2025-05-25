#include "PG_DisaPartyCharacter.h"
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <Disa/fileformats/PG_JsonUtil.h>
using namespace PG_JsonUtil;

nlohmann::json  PG_DisaEquippedItem::toJson() const {
    nlohmann::json j;
    {
        std::vector<nlohmann::json> jResidentSlots;
        for (const DisaItemResident& resident : residentSlots) {
            if (resident.jobIdentifier == 0) break;
            nlohmann::json jSlot;
            jSlot["level"] = resident.level;
            jSlot["jobIdentifier"] = resident.jobIdentifier;
            jSlot["characterClass"] = resident.characterClass;
            jResidentSlots.push_back(std::move(jSlot));
        }
        if (!jResidentSlots.empty()) {
            j["residentSlots"] = std::move(jResidentSlots);
        }
    }


    j["unknownValue0x40"] = unknownValue0x40;
    j["unknownValue0x44"] = unknownValue0x44;
    j["stats"] = stats.toJson();
    j["baseStats"] = baseStats.toJson();

    j["id"] = id;
    j["level"] = level;
    j["unknownFlag1"] = unknownFlag1;
    j["rarety"] = rarety;
    j["unknownFlag2"] = unknownFlag2;
    j["unknownFlag3"] = unknownFlag3;

    j["iconIndex"] = iconIndex;
    j["maxResidents"] = maxResidents;
    j["unknownFlag5"] = unknownFlag5;
    j["unknownFlag6"] = unknownFlag6;
    j["rank"] = rank;
    j["range"] = range;
    j["unknownFlag7"] = unknownFlag7;
    j["unknownFlag8"] = unknownFlag8;

    j["unknownValue0x88"] = unknownValue0x88;
    j["unknownValue0x8C"] = unknownValue0x8C;

    return j;
}


std::string PG_DisaPartyCharacter::getName() const {
    return shiftJisToString(toShiftJisString(charName));
}

std::string PG_DisaPartyCharacter::getTitle() const {
    return shiftJisToString(toShiftJisString(charTitle));
}

void PG_DisaPartyCharacter::setCustomPortraitId(unsigned short id) {
    if (id & 0x0080) {
        portraitImageId = 0;
        portraitColor = 0;
        return; //too big
    }
    memcpy(&portraitImageId, &id, sizeof(short));
    //memcpy(&portraitColor, ((char*)&id) + 1, sizeof(char));
    portraitColor |= 0x80; //add flag bit
}

std::string PG_DisaPartyCharacter::toString() const {
    std::stringstream stream;
    stream << "(" << std::setw(4) << std::setfill(' ') << characterId << ") " << getName() << " - " << getTitle() << ";";
    return stream.str();
}


nlohmann::json PG_DisaPartyCharacter::toJson() const {
    nlohmann::json j;

    j["experience"] = experience;
    j["weapon"] = weapon.toJson();
    j["item1"] = item1.toJson();
    j["item2"] = item2.toJson();
    j["item3"] = item3.toJson();

     j["name"] = getName();
     j["title"] = getTitle();

    unsigned int baseOffset = (unsigned int)this;
    for (auto& v : unknownData1) {
        j[std::string("unknownValue") + std::to_string((unsigned int)&v - baseOffset)] = v;
    }

    j["resPoison"] = resPoison;
    j["resSleep"] = resSleep;
    j["resParalyze"] = resParalyze;
    j["resForget"] = resForget;
    j["resDeprave"] = resDeprave;

    j["resFire"] = resFire;
    j["resWind"] = resWind;
    j["resIce"] = resIce;

    for (auto& v : unknownData2) {
        j[std::string("unknownValue") + std::to_string((unsigned int)&v - baseOffset) ]= v;
    }

    j["skillEXP"] = skillEXP;
    j["skillId"] = skillId;
    j["skillLevel"] = skillLevel;

    j["currentHP"] = currentHP;
    j["currentSP"] = currentSP;

    j["activeStats"] = activeStats.toJson();
    j["stats"] = stats.toJson();
    j["weaponMasteryEXP"] = weaponMasteryEXP;

    j["mana"] = mana;

    for (auto& v : unknownData3) {
        j[std::string("unknownValue") + std::to_string((unsigned int)&v - baseOffset)] = v;
    }

    j["weaponMasteryLevels"] = weaponMasteryLevels;
    j["weaponMasteryBaseStats"] = weaponMasteryBaseStats;

    for (auto& v : unknownData4) {
        j[std::string("unknownValue") + std::to_string((unsigned int)&v - baseOffset)] = v;
    }

    j["level"] = level;
    j["spriteId"] = spriteId;
    j["characterId"] = characterId;

    for (auto& v : unknownData5) {
        j[std::string("unknownValue") + std::to_string((unsigned int)&v - baseOffset)] = v;
    }

    j["unknownStat1"] = unknownStat1;
    j["activeResFire"] = activeResFire;
    j["activeResWind"] = activeResWind;
    j["activeResIce"] = activeResIce;

    j["jumpingPowerBase"] = jumpingPowerBase;
    j["jumpingPower"] = jumpingPower;
    j["movementBase"] = movementBase;
    j["movement"] = movement;
    j["counterBase"] = counterBase;
    j["counter"] = counter;
    j["unknownStat2"] = unknownStat2;
    j["unknownStat3"] = unknownStat3;

    j["paletteIndex"] = paletteIndex;
    j["learnSpeed"] = learnSpeed;

    for (auto& v : unknownData6) {
        j[std::string("unknownValue") + std::to_string((unsigned int)&v - baseOffset)] = v;
    }
    j["field_0x68b"] = field_0x68b;
    j["portraitImageId"] = portraitImageId;
    j["portraitColor"] = portraitColor;
    j["field_0x68d"] = field_0x68d;
    j["field_0x68e"] = field_0x68e;
    j["field_0x690"] = field_0x690;
    j["field_0x691"] = field_0x691;
    j["field_0x692"] = field_0x692;
    j["field_0x693"] = field_0x693;
    j["field_0x694"] = field_0x694;
    j["field_0x698"] = field_0x698;
    j["field_0x69a"] = field_0x69a;
    j["field_0x69c"] = field_0x69c;
    j["field_0x6a0"] = field_0x6a0;

    for (auto& v : field_0x6a4) {
        j[std::string("unknownValue") + std::to_string((unsigned int)&v - baseOffset)] = v;
    }

    j["field_0x6ac"] = field_0x6ac;
    j["field_0x6b0"] = field_0x6b0;
    j["field_0x6b4"] = field_0x6b4;
    j["field_0x6b6"] = field_0x6b6;

    return j;
}