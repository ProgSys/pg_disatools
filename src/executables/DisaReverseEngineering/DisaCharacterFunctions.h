#pragma once

#include <DisaFunctions.h>
#include <nlohmann/json.hpp>
#include <Disa/dataStructs/PG_DisaStats.h>
#include <Disa/dataStructs/PG_DisaDataCharacter.h>
#include <Disa/dataStructs/PG_DisaPartyCharacter.h>
#include <Disa/dataStructs/PG_DisaDataMagic.h>



/**
* @brief Unit inside current player party
* The number of units in player party is stored in aUnitsOwned.
* Located at &FP::getData<uint>(0x0061a45c) - 0x171.
*/
struct DisaPartyCharacterInfo : public PG_DisaPartyCharacter { //1720 unsigned char
    static std::string playerPartyToString();
    static std::string playerPartyToJson();

    static DisaPartyCharacterInfo& addNewEntry();
    static unsigned int getSize();
    static DisaPartyCharacterInfo& getEntry(unsigned int index);

    static DisaPartyCharacterInfo* begin();
    static DisaPartyCharacterInfo* end();

};

/**
* @brief From Char.dat
*/
struct DisaDataCharacterInfo : public PG_DisaDataCharacter { //size 216
    /**
    * @brief Copies original data into our managed memory and swap all pointers to our data
    * Must be called before other functions
    */
    static void steal();
    static DisaDataCharacterInfo& addNewEntry();
    static unsigned int getSize();
    static DisaDataCharacterInfo& getEntry(unsigned int index);

    static std::vector<DisaDataCharacterInfo>::iterator begin();
    static std::vector<DisaDataCharacterInfo>::iterator end();

    /**
    * @brief Convertrs the curret characters table to a string
    */
    static std::string globalToString();
};

struct DisaDataMagicInfo : public PG_DisaDataMagic {

    /**
    * @brief Copies original data into our managed memory and swap all pointers to our data
    * Must be called before other functions
    */
    static void steal();
    static DisaDataMagicInfo& addNewEntry();
    static unsigned int getSize();
    static DisaDataMagicInfo& getEntry(unsigned int index);

    static std::vector<DisaDataMagicInfo>::iterator begin();
    static std::vector<DisaDataMagicInfo>::iterator end();

    /**
    * @brief Convertrs the curret magic table to a string
    */
    static std::string globalToString();
};

struct DisaAssemblyUnitInfo {
    unsigned short killCount; //used to scale mana cost applied to character ids over 2000. Inversed, staring from 29 to 0
    unsigned short characterId;
    unsigned int killCountLevelSum; //the level sum of all units killed

    void clear() {
        killCount = 0;
        characterId = 0;
        killCountLevelSum = 0;
    }
};

/**
* @brief A senator of the Dark Assembly.
* Located at 0x0064FA98, 0x200 entries, the number in use is DAT_0065db14. Filled by FUN_004d36b0.
*/
struct DisaSenatorEntry { //32 unsigned char
    unsigned short level; //0x00
    unsigned short characterId; //0x02
    unsigned short attendance; //0x04
    unsigned short killCount; //0x06 how often the player killed the senator
    uint8_t nameTextGroup; //0x08 text bank group of the name
    uint8_t nameTextIndex; //0x09 text bank index of the name
    uint8_t unknown0x0a; //0x0a
    uint8_t unknown0x0b; //0x0b random 0 to 255 from FUN_004d36b0
    uint8_t unknown0x0c; //0x0c set to 1 by FUN_004d36b0
    std::array<char, 17> name; //0x0d shiftJis encoded
    int8_t favor; //0x1e opinion of the player, shown from "Loathe" to "Love" by FUN_0049c600. Starts at -20 to 10
    uint8_t nameTextStyle; //0x1f the text style (DisaSettings::field_0x30) the name is written in
};
static_assert(sizeof(DisaSenatorEntry) == 0x20);

//render status screen
void __fastcall FUN_0047f080_ProccessStatusMenu(unsigned int i);

//to create new character Proccess: FUN_0048dfd0
//to create new character Render: FUN_0048dba0

void __fastcall FUN_004d63b0_EnemyUnitKilled(DisaPartyCharacterInfo* character);

void FUN_0048cd40_BuildAssemblyUnitInfosLookup();


void FUN_0048dfd0_ProccessAssemblyMenu();


void __fastcall FUN_004b5490(unsigned int portraitImageId, unsigned int portraitColor, int param_3, int param_4, int param_5, int param_6, int param_7);

void FUN_004b5490_NoneStd(int param_3, int param_4, int param_5, int param_6, int param_7);

//is called after characters where loaded
void __stdcall FUN_004cf3c0(); //called once the game has set up its data tables

/**
* @brief Writes the stored names and titles again in the current text style, so they match the loaded text banks.
* Covers the 128 party slots, the three units at 0x0065E468 and the senators. Called once at the end of loading a save.
* Original function: FUN_004cd1b0, called at 0x004CB5B3 by FUN_004cb380.
* @param textStyle DisaSettings::field_0x30, only stored into DisaSenatorEntry::nameTextStyle. The copy itself reads the setting.
* @warning Like the original, an unknown characterId reads the entry before the character table.
*/
void __fastcall FUN_004cd1b0_refreshNameTexts(uint32_t textStyle);