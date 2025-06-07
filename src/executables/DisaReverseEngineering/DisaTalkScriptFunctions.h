#pragma once

#include <DisaFunctions.h>

static unsigned int patch_FUN_004b6b93_OrgAddress = 0;
static unsigned int patch_FUN_00463f7a_OrgAddress = 0;

struct DisaTScriptManager {
	int field0_0x0[4];               // 0x0    - 0xc
	int portraitIndex;				// 0x10
	int portraitColor;				// 0x14
	int field_0x18[8];               // 0x18    - 0x38
	int totalScripts;                 // 0x38   - 0x3c
	DisaTalkDatScriptEntry* scriptsEntries; // 0x3c
	int scriptId; //0x40 The requested id
	unsigned int dungeonId; //0x44
	unsigned int unknownFlag; //0x48
	int resolvedScriptId;               // 0x4c The script id found with given search values
	int isActive;                   // 0x50   - 0x54 true when script is running
	short field4_0x54;                  // 0x54   - 0x58
	unsigned short field_0x58;                  // 0x58
	short field5_0x58;                  // 0x58   - 0x5a
	short field5_0x5a;                  // 0x5a   - 0x5c
	unsigned char* scriptsData;                  // 0x5c   - 0x60
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
	unsigned char field_0x481c[12];          //0x481c
	int bgBoxLocation;          //0x4828
	int bgBoxSpeed;          //0x482c

	/// run talk script
	int __thiscall FUN_004bf7e0_RunTalkScript(unsigned int scriptId, unsigned int dungeonId, unsigned int unknownFlag, unsigned int param_4);
	int __thiscall call(const std::string& scriptName, unsigned int param_4);

	void clearCharDisplay();
private:
	void setupScript(unsigned int scriptId, unsigned int dungeonId, unsigned int unknownFlag, unsigned int param_4);
};

void __fastcall FUN_004bfa60_TalkScriptExec(DisaTScriptManager* disaTScriptManager);