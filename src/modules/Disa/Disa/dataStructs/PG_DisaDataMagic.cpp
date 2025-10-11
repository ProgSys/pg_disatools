#include "PG_DisaDataMagic.h"
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <Disa/fileformats/PG_JsonUtil.h>
#include <Util/PG_StringUtil.h>
using namespace PG_JsonUtil;

std::string PG_DisaDataMagic::getName() const {
    return shiftJisToString(toShiftJisString(name));
}
void PG_DisaDataMagic::setName(const std::string& nameIn) {
    memset(name.data(), 0, name.size());
    if (nameIn.empty() || nameIn.size() > 100) return;

    const std::string shiftStr = stringToShiftJis(nameIn);
    memcpy(name.data(), shiftStr.data(), std::min(shiftStr.size(), name.size() - 1));
    name.back() = 0;
}

std::string PG_DisaDataMagic::getDescription() const {
    return shiftJisToString(toShiftJisString(description));
}

static std::map<std::string, unsigned int> gIconMapper = {
    {"fist", 0},
    {"swort", 1},
    {"spear", 2},
    {"bow", 3},
    {"pistol", 4},
    {"axe", 5},
    {"staff", 6},
    {"special", 7},
    {"poison", 8},
    {"sleep", 9},
    {"paralysis", 10},
    {"questionmark", 11},
    {"whirl", 12},
    {"special2", 22},
};

void PG_DisaDataMagic::setDescription(const std::string& str) {
    memset(description.data(), 0, description.size());
    if (str.empty() || str.size() > 256) return;

    //add special markers for icons

    std::string shiftStr;
    std::string specialChar;
    int counter = 0;
    std::vector<std::pair<int, int>> insertTable;
    for (char c : str) {
        if (c == '<') {
            if (counter != 0) {
                shiftStr.clear();
                break;
            }
            ++counter;
            insertTable.emplace_back(shiftStr.size(), -1);
        }
        else if (c == '>') {
            if (counter != 1) {
                shiftStr.clear();
                break;
            }
            --counter;
            if (!specialChar.empty() && !insertTable.empty()) {
                if (auto findIt = gIconMapper.find(specialChar); findIt != gIconMapper.end()) {
                    insertTable.back().second = findIt->second;
                }
                else {
                    try {
                        insertTable.back().second = PG_StringUtil::toInt(specialChar);
                    }
                    catch (...) {}
                }
            }
            specialChar.clear();
        }
        else if (counter != 0) {
            specialChar.push_back(c);
        }
        else {
            shiftStr.push_back(c);
        }
    }

    if (shiftStr.empty() && insertTable.empty()) return;
    shiftStr = stringToShiftJis(shiftStr);

    if (!insertTable.empty()) {
        unsigned int lastAt = 0;
        unsigned int offset = 0;
        for (auto [at, toInsert] : insertTable) {
            offset = offset + indexToShiftJisOffset(shiftStr, at - lastAt, offset);
            lastAt = at;
            if (toInsert < 0) toInsert = 11;
            unsigned short specialChar = 0x873f + glm::clamp(toInsert + 1, 1, 93);
            std::string insertStr = { (char)(specialChar >> 8) , (char)specialChar };
            shiftStr.insert(offset, insertStr);
            offset += 2;
        }
    }

    memcpy(description.data(), shiftStr.data(), std::min(shiftStr.size(), description.size() - 1));
    description.back() = 0;
}

void PG_DisaDataMagic::fromJson(const nlohmann::json& j) {
    readFromJson(j, "id", id);
    readFromJson(j, "level", level);
    readFromJson(j, "pow", pow);
    readFromJson(j, "sp", sp);

    {
        std::string str;
        readFromJson(j, "name", str, 32);
        setName(str);
    }
    {
        std::string str;
        readFromJson(j, "description", str, 128);
        setDescription(str);
    }

    readFromJson(j, "effectPrams", effectPrams);
    readFromJson(j, "wt", wt);
    readFromJson(j, "e", e);
    readFromJson(j, "st", st);
    readFromJson(j, "wt2", wt2);
    readFromJson(j, "rg", rg);
    readFromJson(j, "pat", pat);
    readFromJson(j, "up", up);
    readFromJson(j, "dn", dn);
    readFromJson(j, "effect", effect);
}

std::string PG_DisaDataMagic::toString() const {
    std::stringstream stream;
    stream << "(" << std::setw(4) << std::setfill(' ') << id << ") " << getName() << " - " << getDescription() << ";";
    return stream.str();
}


