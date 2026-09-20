#pragma once

#ifndef far
#define far
#endif

/*
* The game is a 32 bit x86 binary, so its functions use the x86 calling conventions.
* x64 only has a single calling convention, the keywords below carry no meaning there
* and are expanded to nothing. The signatures stay usable as plain function pointers,
* but actually calling into the game is only possible from a 32 bit build.
*/
#if defined(_M_IX86) || defined(__i386__)
	#define PG_DISA_CDECL    __cdecl
	#define PG_DISA_STDCALL  __stdcall
	#define PG_DISA_FASTCALL __fastcall
	#define PG_DISA_THISCALL __thiscall
#else
	#define PG_DISA_CDECL
	#define PG_DISA_STDCALL
	#define PG_DISA_FASTCALL
	#define PG_DISA_THISCALL
#endif

/**
* @brief Signatures of the original game functions.
* FunctionPatcher inherits from this, so all of them are also reachable as FP::Far*Function.
*/
struct PG_DisaFunctionsDefine {
	template<typename R, typename... Args>
	using FarFunction = R(far*)(Args...);

	template<typename R, typename... Args>
	using FarStdFunction = R(PG_DISA_STDCALL far*)(Args...);

	template<typename R, typename... Args>
	using FarFastFunction = R(PG_DISA_FASTCALL far*)(Args...);

	template<typename R, typename... Args>
	using FarThisFunction = R(PG_DISA_THISCALL far*)(Args...);

	template<typename C, typename R, typename... Args>
	using FarThisFunction2 = R(PG_DISA_THISCALL far C::*)(Args...);

	template<typename R, typename... Args>
	using FarCdeclFunction = R(PG_DISA_CDECL far*)(Args...);
};
