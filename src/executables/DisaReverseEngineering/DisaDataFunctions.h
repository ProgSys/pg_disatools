#pragma once

#include <DisaFunctions.h>

/**
* @brief Raw data contianer
* Has a size and two arrays.
* First array has the offsets to the raw data.
* Second array, right after the first array, holds the Ids of each data entry.
*/
struct DisaSpriteOffsetListArchiveData {
    unsigned int size;
    unsigned int firstAddressOffset;

    inline unsigned int* getAddressBegin() const { return (unsigned int*) (&firstAddressOffset); }
    inline unsigned short* getIdsBegin() const { return (unsigned short*) (&firstAddressOffset + size); }
    std::pair<unsigned short, const char*> getDataEntry(int index) const;
};

struct DisaSpriteLoaderInfo {
    int count;               //0x0 number of sprites inside anmData
    int initialized;        //0x4 true when processed
    char* data;              //0x8 processed data after FUN_004de780 was called, each having length of 0x1200
    DisaSpriteOffsetListArchiveData* anmData;   //0x0C double array: unit size, unit[size] addresses offset, ushort[size] ids, .sh files 
    unsigned short* spriteIds;  //0x10 points to the sprite ids inside anmData

    /**
    * @brief Will traverse over each sprite and write a procces result into data
    * data must be preallocated with the appropriate size (count * 0x1200)
    */
    void FUN_004de780_processANM(int param_2);
};

/**
* @brief Find sprite data for given id
*/
char* __fastcall FUN_004de880_getSpriteData(unsigned int spriteId);
/**
* @brief Comresses the given save data and save it with given filename.
*/
bool __stdcall FUN_00442260_Save(char* data, size_t fileSize, char* filename);
/**
* @brief Is runn during loading to decrypt and decompress save data
*/
bool __fastcall FUN_0043e600_decompressSave(char*& decompressedData, unsigned int& decompressedSize, char* data);
void FUN_0043e600_decompressSave_NoneStd(char* data); //do not clean stack

bool __stdcall FUN_004422c0_LoadSave(void* dst, size_t dstSize, char* filename, int* param_4, __time64_t* param_5);