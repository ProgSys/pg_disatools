#pragma once

#include <DisaFunctions.h>
#include <string>
#include <Disa/dataStructs/PG_DisaScriptManager.h>
#include <Disa/PG_DisaIdentifier.h>

void __fastcall FUN_004b6420_ScriptExec(PG_DisaScriptMemory* disaScriptMemory, int param_2);

//call SCRIPT
int __fastcall FUN_004bd930_RunScript(int scriptId, DisaWorldContext* param_2); //find script and run it ->  FUN_004b6420 FUN_0045c0f0(gameloop)

void __fastcall FUN_004f7e70_ScriptLoop(short param_1, short param_2);

void replaceDefaultCase();

int runCustomScript(const PG_DisaIdentifier& scriptName, DisaWorldContext* param_2); //find custom script and run it
int runCustomScript(const PG_DisaIdentifier& scriptName); //find custom script and run it