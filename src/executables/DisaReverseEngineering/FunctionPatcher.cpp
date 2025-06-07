#include "FunctionPatcher.h"
#include <Windows.h>
#include <iostream>
#include <psapi.h>
#include <Tlhelp32.h>

static FunctionPatcher* g_functionPatcher = nullptr;

void FunctionPatcher::init() {
    if (g_functionPatcher) return;


    //HMODULE hModule = GetModuleHandle(TEXT("dis1_st_patch_fixed_mod.exe"));
    HMODULE hModule = GetModuleHandle(nullptr);
    if (hModule == NULL) {
        std::cerr << "Failed to get module handle" << std::endl;
        return;
    }

    // Get module information
    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo))) {
        std::cerr << "Failed to get module information" << std::endl;
        return;
    }

    {
        TCHAR szModuleName[MAX_PATH];
        // Get the name of the main process
        if (GetModuleFileName(hModule, szModuleName, MAX_PATH) != 0) {
            std::cout << "Main process name: " << szModuleName << std::endl;
        }
    }

    g_functionPatcher = new FunctionPatcher();
    g_functionPatcher->setBaseAdress(reinterpret_cast<unsigned int>(moduleInfo.lpBaseOfDll));

    // Print the base address of the module
    std::cout << "Base address of module: 0x" << std::hex << moduleInfo.lpBaseOfDll << std::endl;
}

FunctionPatcher* FunctionPatcher::get() {
    return g_functionPatcher;
}

int far add(int a, int b) {
    return a + b;
}

unsigned int FunctionPatcher::replaceFunction(unsigned int originalFuncAddress, unsigned int replacementFunctionAddress) {
    originalFuncAddress -= usedBase;
    originalFuncAddress += FunctionPatcher::get()->getBaseAdress();

    //TODO add base address
    unsigned int* originalFuncLoc = (unsigned int*)(originalFuncAddress + 1);
    unsigned int originalFuncTargetAddress = originalFuncAddress + *originalFuncLoc + 0x00000005;
   

    DWORD oldProtect;
    if (!VirtualProtect(reinterpret_cast<LPVOID>(originalFuncAddress), 32, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        std::cerr << "Failed to change memory protection." << std::endl;
        return 0;
    }

    *originalFuncLoc = replacementFunctionAddress - originalFuncAddress - 0x00000005;
    
    return originalFuncTargetAddress;
}

unsigned int FunctionPatcher::replaceAddress(unsigned int originalAddressLocation, unsigned int replacementAddress) {
    originalAddressLocation -= usedBase;
    originalAddressLocation += FunctionPatcher::get()->getBaseAdress();

    //TODO add base address
    unsigned int* originalFuncLoc = (unsigned int*)(originalAddressLocation);
    unsigned int originalFuncTargetAddress = *originalFuncLoc;


    DWORD oldProtect;
    if (!VirtualProtect(reinterpret_cast<LPVOID>(originalAddressLocation), 32, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        std::cerr << "Failed to change memory protection." << std::endl;
        return 0;
    }

    *originalFuncLoc = replacementAddress;

    return originalFuncTargetAddress;
}

unsigned int FunctionPatcher::replaceFunctionIndirectCall(unsigned int originalFuncAddress, unsigned int replacementPointerAddress) {
    originalFuncAddress -= usedBase;
    originalFuncAddress += FunctionPatcher::get()->getBaseAdress();

    unsigned int* originalFuncLoc = (unsigned int*)(originalFuncAddress + 2); //has two values
    unsigned int originalFuncTargetAddress = *originalFuncLoc;


    DWORD oldProtect;
    if (!VirtualProtect(reinterpret_cast<LPVOID>(originalFuncAddress), 32, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        std::cerr << "Failed to change memory protection." << std::endl;
        return 0;
    }

    *originalFuncLoc = replacementPointerAddress;

    return originalFuncTargetAddress;
}

void FunctionPatcher::callFar(unsigned int funcAddress) {
    funcAddress -= usedBase;
    funcAddress += FunctionPatcher::get()->getBaseAdress();
    __asm {
        CALL far funcAddress;
    }
}