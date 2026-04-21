#pragma once

#include <Disa/dataStructs/PG_DisaStats.h>

//the first 3 bits is the mask the others are the index
struct PG_DisaGameStateFilter {
	unsigned short bitIndex : 3; //the bit that should be tested
	unsigned short index : 13; //the index of a gamestate at 0x00619330
	inline unsigned short getIndex() const noexcept { return index; }
	inline unsigned char getMask() const noexcept { return 1 << bitIndex; }

	inline bool isSet(unsigned char* globalStateIt) const {
		return ((globalStateIt)[getIndex()] & getMask()) == 0;
	}

	inline void set(unsigned char* globalStateIt) const {
		(globalStateIt)[getIndex()] = (globalStateIt)[getIndex()] | getMask();
	}

	inline void clear(unsigned char* globalStateIt) const {
		(globalStateIt)[getIndex()] = (globalStateIt)[getIndex()] & ~getMask();
	}

	inline bool isEmpty() const { return bitIndex == 0 && index == 0; }
};

struct PG_DisaTalkDatScriptEntry {
	unsigned int address; //0
	unsigned int scriptId; //4
	unsigned short dungeonId; //8
	unsigned short stateFlag; //10
	unsigned short gameStateFilterSizePositive; //12
	unsigned short gameStateFilterSizeNegative; //14-16
	std::array<PG_DisaGameStateFilter, 4> gameStateFilterPositive; // 16-18-20-22-24
	std::array<PG_DisaGameStateFilter, 4> gameStateFilterNegative;// 24-26-28-30-32
};


struct PG_DisaScriptMemory {
	unsigned int ux0; //0
	unsigned int ux4; //1
	unsigned int ux8; //2
	unsigned int uxC; //3
	unsigned int* codePointer; //ux10; //4
	unsigned int field0x14; //0x14
	unsigned int field0x18; ///0x18
	unsigned int scriptId; //ux1c; //7
	unsigned int field0x20; //0x20 sleepTimer?
	unsigned int index; //ux36; //9
	unsigned int field0x28; //0x28
	unsigned int field0x2c; //0x2c
	unsigned int field0x30; //0x30
	unsigned int field0x34; //0x34
	unsigned int scriptInfoSlotIndex; //0x38 DisaTScriptManager_0071b510.scriptInfosSlots
	unsigned int field0x3c; //0x3c
	unsigned int field0x40; //0x40
	unsigned int field0x44; //0x44
	unsigned int ux72; //0x48
	float field0x4c; //0x4c
	short* field_0x50; //0x50 world context
	std::array<int32_t, 8> memorybank0x54; //0x54 Sets values in DAT_007200eb
	std::array<int32_t, 3> memorybank0x74; //0x74 Numbers smaller 32. Sets values in DAT_006192c0
	unsigned short repeatLoopCounter; //0x80
	unsigned short repeatLoopMax; //0x82
	int repeatLoopStartCodePointer; //0x84
};

struct PG_DisaTScriptManager {
	int field0_0x0[4];               // 0x0    - 0xc
	int portraitIndex;				// 0x10
	int portraitColor;				// 0x14
	//int field_0x18[8];               // 0x18    - 0x38
	int field_0x18[3];               // 0x18    - 0x38
	int sizeScripts; //0x24
	uint32_t* scriptsCodeAdresses;
	uint32_t* scriptsIdList;
	int scriptsOriginAdress;
	bool* scriptActiveList;
	int sizeTalkScripts;                 // 0x38   - 0x3c
	PG_DisaTalkDatScriptEntry* talkScriptsEntries; // 0x3c
	int scriptId; //0x40 The requested id
	unsigned int dungeonId; //0x44
	unsigned int unknownFlag; //0x48
	int resolvedScriptId;               // 0x4c The script id found with given search values
	int isActive;                   // 0x50   - 0x54 true when script is running
	short field4_0x54;                  // 0x54   - 0x58
	unsigned short field_0x58;                  // 0x58
	short field5_0x58;                  // 0x58   - 0x5a
	short field5_0x5a;                  // 0x5a   - 0x5c
	unsigned char* scriptsData;                  // 0x5c   - 0x60 talk scripts
	unsigned char* scriptPointer;              // 0x60   - 0x64
	unsigned char waitUserImputMode;			// 0x64   //like waiting for user to press confirm or make amenu selection // 0x64   - 0x68
	unsigned char field8_0x65;          // 0x65   
	unsigned char speechBubbleType;  //0x66 
	unsigned char speechBubbleArrowPosition; //0x67
	unsigned char data[5756];                  // 0x68   - 0x16e4
	short speechBubbleAnimationSpeed;             // 0x16e4 - 0x16e6
	short speechBubbleAnimationFrame;             // 0x16e6 - 0x16e8 //progress of speesh bubble opening from 0 to 16
	short dialogStringLength;             // 0x16e8 - 0x16ea
	short dialogStringCurrentRow;             // 0x16ea - 0x16ec
	short field14_0x16ec;             // 0x16ec - 0x16ee
	short field15_0x16ee;             // 0x16ee - 0x16f0
	short field16_0x16f0;             // 0x16f0 - 0x16f2
	short field16_0x16f2;             // 0x16f2 - 0x16f4
	unsigned char field17_0x16f4[4];           // 0x16f4 - 0x16f8
	unsigned char field_0x16f8[42];         // 0x16fa
	unsigned char data2[12414];                // 0x1722 - 0x47fc
	short charDisplaySlot[3];	//0x47a0	
	short charDisplayIsFree[3];	//0x47a6	
	short charDisplayAnimationLength[3]; //0x47ac or could be delay
	short charDisplaySpeed[3]; //0x47b2
	short charDisplayAnimationType[3]; //0x47b8
	short field_0x47be[3]; //0x47be
	short charDisplayXOffset[3]; //0x47c4
	short charDisplayYOffset[3]; //0x47ca
	char dataMid[10]; //0x47d0
	short loadedCharIds[9]; //0x47da
	unsigned short labelPosition;     //0x47ec
	short labelTextId;                //0x47ee
	short field_0x47f0;
	short field_0x47f2;
	unsigned char data3[6];          //0x47f4	
	unsigned char field_0x47fa;          //0x47fa
	unsigned int field_0x47fc;    // 0x47fc - 0x4800
	short field21_0x4800;           // 0x4800 - 0x4802
	unsigned short voiceId;             // 0x4802 - 0x4804
	unsigned short voiceRunFlag;             // 0x4804 - End
	unsigned char field_0x4806[14];          //0x4806
	int field_0x4814;          //0x4814
	int field_0x4818;          //0x4818
	int field_0x481c;          //0x481c
	int field_0x4820;          //0x4820
	int field_0x4824;          //0x4824
	int bgBoxLocation;          //0x4828
	int bgBoxSpeed;          //0x482c

};