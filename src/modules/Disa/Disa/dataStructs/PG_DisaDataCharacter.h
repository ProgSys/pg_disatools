#pragma once

#include <Disa/dataStructs/PG_DisaStats.h>

/**
* @brief From Char.dat
*/
struct PG_DisaDataCharacter { //size 216
    std::array<char, 33> name; //shiftJis encoded
    std::array<char, 27> title; //shiftJis encoded

    unsigned char type; //0x3c
    unsigned char unknownValue0x3d; //0x3d
    unsigned char unknownValue0x3e; //0x3e
    unsigned char gender; //0x3f
    unsigned char numberOfSkills; //0x40
    unsigned char paletteIndex; //0x41
    unsigned char learnSpeed; //0x42
    unsigned char jumpPower; //0x43
    std::array<unsigned char, 8> weaponMasteryBaseStats; //0x44 - 0x4c fists, swort, spears, bows, guns, axes, staves, unknown (always zero)
    PG_DisaStartingStats stats; // 0x4c - 0x53
    unsigned short immunity; //0x54

    unsigned short characterId; //0x55
    unsigned short spriteId; //0x57
    unsigned short unknownValue0x58; //0x58
    unsigned short unknownValue0x60; //0x60
    unsigned short throwingRange;//0x6b
    unsigned short helpID;//0x5f
    PG_DisaBaseStats baseStats;//0x61
    unsigned short experienceScale; //scales how much experience the character needs to level up
    unsigned short movement;
    unsigned short movementType;
    unsigned short unknownValue0x77;
    unsigned short unknownValue0x79;
    unsigned short unknownValue0x7b;
    unsigned short unknownValue0x7d;
    unsigned short unknownValue0x7f;
    unsigned short unknownValue0x81;
    unsigned short manaCostScale; // (baseRankCost * manaCostScale) / 10
    unsigned short counter;

    std::array<unsigned short, 20> skillIds;
    std::array<unsigned short, 20> skillLevels; //when the skill unlockes

    std::string getName() const;
    void setName(const std::string& name);

    std::string getTitle() const;
    void setTitle(const std::string& title);

    void clear();

    void fromJson(const nlohmann::json& j);

    std::string toString() const;
};