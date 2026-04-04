#pragma once
#include <glm/glm.hpp>

/**
* @brief Compares most likly two vec3 values by y and z. If both are queal or nan then the pointer is used as fallback.
*/
bool FUN_004280a0(const glm::vec3* param_1, const glm::vec3* param_2);

void __fastcall FUN_00427da0(glm::vec3** param_1, glm::vec3** param_2);
void __fastcall FUN_00427da0_insertionSort(glm::vec3** param_1, glm::vec3** param_2);
void __fastcall FUN_00427ff0_pushHeap(glm::vec3** begin, int index, int param_3, glm::vec3** insertValue);
void __fastcall FUN_00427c30_makeHeap(glm::vec3** begin, glm::vec3** end);

void _stdcall FUN_00426770(int param_1, unsigned int param_2, unsigned char depthMask);
void _stdcall FUN_00426770_Dummy(int param_1, unsigned int param_2, unsigned char depthMask);