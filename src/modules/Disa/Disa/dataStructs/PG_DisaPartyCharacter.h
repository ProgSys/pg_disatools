#pragma once

#include <Disa/dataStructs/PG_DisaStats.h>

/**
* @brief An equippable item on a character or backpack
*/
struct PG_DisaEquippedItem { //144 unsigned char
    /**
    * @brief Info about a resident inside the item
    */
    struct DisaItemResident {
        unsigned short level;
        unsigned char jobIdentifier; // example 9 == Gladiator
        unsigned char characterClass; //random monster name and character class.
    };


    std::array<DisaItemResident, 16> residentSlots;
    unsigned int unknownValue0x40;
    unsigned int unknownValue0x44;

    PG_DisaStats stats; // are recalculateed
    PG_DisaBaseStats baseStats; //then base stats of the item

    unsigned short id;
    unsigned short level;
    unsigned char unknownFlag1;
    unsigned char rarety;
    unsigned char unknownFlag2;
    unsigned char unknownFlag3;

    unsigned char iconIndex;
    unsigned char maxResidents;
    unsigned char unknownFlag5;
    unsigned char unknownFlag6;
    unsigned char rank;
    unsigned char range;
    unsigned char unknownFlag7;
    unsigned char unknownFlag8;

    unsigned int unknownValue0x88;
    unsigned int unknownValue0x8C;

    nlohmann::json toJson() const;
};

/**
* @brief Unit inside current player party
* The number of units in player party is stored in aUnitsOwned.
* Located at &FP::getData<uint>(0x0061a45c) - 0x171.
*/
struct PG_DisaPartyCharacter { //1720 unsigned char
    constexpr static size_t byteSize = 1720;

    unsigned long long experience; //total number of experiance points the character has
    PG_DisaEquippedItem weapon;
    PG_DisaEquippedItem item1;
    PG_DisaEquippedItem item2;
    PG_DisaEquippedItem item3;

    std::array<char, 33> charName; //shiftJis encoded
    std::array<char, 27> charTitle; //shiftJis encoded

    std::array<unsigned int, 10> unknownData1;

    //ailment resistance
    unsigned short resPoison; //(PSN) character loses hp every turn
    unsigned short resSleep; //(SLP) character loses actions for a set number of turns based off of specialist strength 
    unsigned short resParalyze; //(PAR) character can not move 
    unsigned short resForget; //(F.G.T) character can not use special abilities
    unsigned short resDeprave; //(DPR) character can not gain EXP

    unsigned short resFire; //not fully confirmed, see currentResFire
    unsigned short resWind; //not fully confirmed, see currentResWind
    unsigned short resIce; //not fully confirmed, see currentResIce

    std::array<unsigned int, 26> unknownData2;

    std::array<unsigned int, 96> skillEXP; //total accumulated EXP throwout all levels
    std::array<unsigned short, 96> skillId; //from MAGIC.DAT
    std::array<unsigned char, 96> skillLevel;

    unsigned int currentHP;
    unsigned int currentSP;

    PG_DisaStats activeStats;  //active stats (they will recalculateed, based on equipped items and so on)
    PG_DisaStats stats;  //fixed character stats used to calculate active stats

    //weapon mastery (levels are saved seperatly, see weaponMasteryLevels)
    std::array<unsigned int, 8> weaponMasteryEXP; //fists, swort, spears, bows, guns, axes, staves, unknown (always zero)

    unsigned int mana;
    PG_DisaBaseStats bonusStats; //added to the stats to compute final stats (FUN_004db610)
    std::array<unsigned short, 4> unknownData3; //0x640
    std::array<unsigned char, 8> weaponMasteryLevels; //fists,swort, spears, bows, guns, axes, staves, unknown (always zero)
    std::array<unsigned char, 8> weaponMasteryBaseStats; //fists,swort, spears, bows, guns, axes, staves, unknown (always zero)
    std::array<unsigned short, 2 * 2> unknownData4;

    unsigned int level;
    unsigned short characterId; // found in CHAR_E.DAT
    unsigned short spriteId; // found in CHAR_E.DAT
    unsigned short charDatIndex; // index of character data
    std::array<unsigned short, 6> unknownData5;

    unsigned char unknownStat1;
    unsigned char activeResFire; //are recalculateed 
    unsigned char activeResWind; //are recalculateed 
    unsigned char activeResIce; //are recalculateed 

    unsigned char jumpingPowerBase; //(Jm) maximum height the character can move to an adjacent panel
    unsigned char jumpingPower;
    unsigned char movementBase; // (MV) determines the number of panels the character can traverse
    unsigned char movement;
    unsigned char counterBase;
    unsigned char counter;
    unsigned char unknownStat2;
    unsigned char unknownStat3;

    unsigned char paletteIndex; // found in CHAR_E.DAT (the color of character)
    unsigned char learnSpeed; // found in CHAR_E.DAT
    std::array<unsigned char, 6> unknownData6;

    unsigned char field_0x68b;
    unsigned char portraitImageId; //index found at FUN_0046dc60
    unsigned char portraitColor;
    unsigned char field_0x68d;
    unsigned short field_0x68e;
    unsigned char field_0x690;
    unsigned char field_0x691;
    unsigned char field_0x692;
    unsigned char field_0x693;
    unsigned int field_0x694;
    unsigned short field_0x698;
    unsigned short field_0x69a;
    unsigned int field_0x69c;
    unsigned int field_0x6a0;
    std::array<unsigned short, 4> field_0x6a4;
    unsigned int field_0x6ac; //string type (the game seems to store what kind of string is in name and title)
    unsigned int field_0x6b0; //use string type? 
    unsigned short field_0x6b4;
    unsigned short field_0x6b6;

    std::string getName() const;
    std::string getTitle() const;

    void setCustomPortraitId(unsigned short id);

    std::string toString() const;
    nlohmann::json toJson() const;
};