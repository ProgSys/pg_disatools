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