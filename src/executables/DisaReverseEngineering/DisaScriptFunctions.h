#pragma once

#include <DisaFunctions.h>
#include <string>

struct DisaScriptMemory {
    unsigned int ux0; //0
    unsigned int ux4; //1
    unsigned int ux8; //2
    unsigned int ux12; //3
    unsigned int* codePointer; //ux16; //4
    unsigned int ux20; //5
    unsigned int ux24; //6
    unsigned int scriptId; //ux28; //7
    unsigned int ux32; //8 sleepTimer?
    unsigned int index; //ux36; //9
    unsigned int ux40; //10
    unsigned int ux44; //11
    unsigned int ux48; //12
    unsigned int ux52; //13
    unsigned int ux56; //14
    unsigned int ux60; //15
    unsigned int ux64; //16
    unsigned int ux68; //17
    unsigned int ux72; //18
    unsigned int ux76; //19
    unsigned int ux80; //20
    int ux84; //21
    int ux88; //22
    int ux92; //23
    int ux96; //24
    int ux100; //25
    int ux104; //26
    int ux108; //27
    int ux112; //28
    int ux116; //29
    unsigned short repeatLoopCounter; //30
    unsigned short repeatLoopMax; //31
    int repeatLoopStartCodePointer; //32
};

void __fastcall FUN_004b6420_ScriptExec(DisaScriptMemory* disaScriptMemory, int param_2);

//call SCRIPT
int __fastcall FUN_004bd930_RunScript(int scriptId, int param_2); //find script and run it ->  FUN_004b6420 FUN_0045c0f0(gameloop)

void __fastcall FUN_004f7e70_ScriptLoop(short param_1, short param_2);

void replaceDefaultCase();

int runCustomScript(const std::string& scriptName, int param_2); //find custom script and run it