#pragma once

#include <type_traits>
#define far


class FunctionPatcher {
public:
	constexpr static unsigned int usedBase = 0x00400000;

	unsigned int getBaseAdress() const { return m_baseAddress; }
	void setBaseAdress(unsigned int address) { m_baseAddress = address; }

	static void init();

	static FunctionPatcher* get();

	/**
	* @brief Replace original fucntion with given
	* All addresses are virutal
	* @return originalFuncTargetAddress
	*/
	static unsigned int replaceFunction(unsigned int originalFuncAddress, unsigned int replacementFunctionAddress);
	template<typename T>
	static unsigned int replaceFunction(unsigned int originalFuncAddress, T&& replacementFunction) {
		return replaceFunction(originalFuncAddress, reinterpret_cast<unsigned int>(std::addressof(replacementFunction)));
	}

	/**
	* @brief Replace a address with given adress
	* All addresses are virutal
	* @return originalTargetAddress
	*/
	static unsigned int replaceAddress(unsigned int originalAddressLocation, unsigned int replacementAddress);
	template<typename T>
	static unsigned int replaceAddress(unsigned int originalAddressLocation, T&& replacementFunction) {
		return replaceAddress(originalAddressLocation, reinterpret_cast<unsigned int>(std::addressof(replacementFunction)));
	}

	/**
	* @brief Replace original fucntion with given
	* For call opcode "FF 15" that represents an instruction for an indirect far call.
	* @param replacementPointerAddress Is not the address to the function, but the address where the function pointer memory is located
	* @return originalFuncTargetAddress
	*/
	static unsigned int replaceFunctionIndirectCall(unsigned int originalFuncAddress, unsigned int replacementPointerAddress);
	template<typename T>
	static unsigned int replaceFunctionIndirectCall(unsigned int originalFuncAddress, T&& replacementPointerAddress) {
		return replaceFunctionIndirectCall(originalFuncAddress, reinterpret_cast<unsigned int>(std::addressof(replacementPointerAddress)));
	}

	static void removeProtect(unsigned int address);
	template<typename T>
	static T& getData(unsigned int address) {
		address -= usedBase;
		address += get()->getBaseAdress();
		return *((T*)(address));
	}



	static void callFar(unsigned int funcAddress);
#pragma runtime_checks( "", off ) //seams like it dosn't work for templates :( Must be disabled generally in the settings for the project :(
	template<typename R, typename... Args>
	using FarFunction = R(far*)(Args...);

	template<typename R = void, typename... Args>
	static R callFarA(unsigned int funcAddress, Args... args) {
		funcAddress -= usedBase;
		funcAddress += FunctionPatcher::get()->getBaseAdress();
		
		return ((FarFunction<R, Args...>)(funcAddress))(args...);
	}

	template<typename R, typename... Args>
	using FarStdFunction = R(__stdcall far*)(Args...);

	template<typename R = void, typename... Args>
	static R callStdFarA(unsigned int funcAddress, Args... args) {
		funcAddress -= usedBase;
		funcAddress += FunctionPatcher::get()->getBaseAdress();

		return ((FarStdFunction<R, Args...>)(funcAddress))(args...);
	}

	template<typename R, typename... Args>
	using FarFastFunction = R(__fastcall far*)(Args...);


	#define popStackDef(I) \
		if constexpr (I == 1) {\
			_asm {\
				add esp, 4\
			}\
		}\
		else if constexpr (I == 2) {\
			_asm {\
				add esp, 8\
			}\
		}\
		else if constexpr (I == 3) {\
			_asm {\
				add esp, 12\
			}\
		}\
		else if constexpr (I == 4) {\
			_asm {\
				add esp, 16\
			}\
		}\
		else if constexpr (I == 5) {\
			_asm {\
				add esp, 20\
			}\
		}\
		else if constexpr (I == 6) {\
			_asm {\
				add esp, 24\
			}\
		}\
		else if constexpr (I == 7) {\
			_asm {\
				add esp, 28\
			}\
		}\
		else if constexpr (I == 8) {\
			_asm {\
				add esp, 32\
			}\
		}\
		else if constexpr (I == 9) {\
			_asm {\
				add esp, 36\
			}\
		}\
		else if constexpr (I == 10) {\
			_asm {\
				add esp, 40\
			}\
		}\
		else if constexpr (I == 11) {\
			_asm {\
				add esp, 44\
			}\
		}\
		else if constexpr (I == 12) {\
			_asm {\
				add esp, 48\
			}\
		}\
		else if constexpr (I == 13) {\
			_asm {\
				add esp, 52\
			}\
		}\
		else if constexpr (I == 14) {\
			_asm {\
				add esp, 56\
			}\
		}

	template<typename R = void, typename... Args>
	static inline R callFastFarA(unsigned int funcAddress, Args... args) {
		funcAddress -= usedBase;
		funcAddress += FunctionPatcher::get()->getBaseAdress();
		if constexpr (std::is_void_v<R>) {
			((FarFastFunction<R, Args...>)(funcAddress))(std::forward<Args>(args)...);
			popStackDef((sizeof...(args) - 2)); //not defined by standard
		}else{
			R ret = ((FarFastFunction<R, Args...>)(funcAddress))(std::forward<Args>(args)...);
			popStackDef((sizeof...(args) - 2)); //not defined by standard
			return ret;
		}
	}

	template<typename R = void, typename... Args>
	static inline R callStdFastFarA(unsigned int funcAddress, Args... args) {
		funcAddress -= usedBase;
		funcAddress += FunctionPatcher::get()->getBaseAdress();
		if constexpr (std::is_void_v<R>) {
			((FarFastFunction<R, Args...>)(funcAddress))(std::forward<Args>(args)...);
		}
		else {
			R ret = ((FarFastFunction<R, Args...>)(funcAddress))(std::forward<Args>(args)...);
			return ret;
		}
	}

	template<typename R, typename... Args>
	using FarThisFunction = R(__thiscall far*)(Args...);

	/**
	* @brief Calls a thiscall
	* this is put into ECX register, arguments into the stack.
	* The callee (the function that is called) is resposible to clean the stack.
	*/
	template<typename R = void, typename... Args>
	static R __declspec(noinline) callThisFarA(unsigned int funcAddress, Args... args) { //this is in ECX register
		funcAddress -= usedBase;
		funcAddress += FunctionPatcher::get()->getBaseAdress();

		if constexpr (std::is_void_v<R>) {
			((FarThisFunction<R, Args...>)(funcAddress))(std::forward<Args>(args)...);
			_asm {
				add esp, 0 //force stack cleanup
			}
		}
		else {
			R ret = ((FarThisFunction<R, Args...>)(funcAddress))(std::forward<Args>(args)...);
			_asm {
				add esp, 0 //force stack cleanup
			}
			return ret;
		}
	}

	template <typename... Args>
	constexpr static std::size_t countFloats() {
		return (0 + ... + (std::is_same_v<std::remove_cv_t<Args>, float> ? 1 : 0));
	}

	/**
	* @brief Calls a Variadic thiscall
	* this is put into ECX register, arguments into the stack.
	* The caller is resposible to clean the stack.
	*/
	template<typename R = void, typename... Args>
	static R __declspec(noinline) callVariadicThisFarA(unsigned int funcAddress, Args... args) { //this is in ECX register
		funcAddress -= usedBase;
		funcAddress += FunctionPatcher::get()->getBaseAdress();
		constexpr std::size_t floatCount = countFloats<Args...>();
		if constexpr (std::is_void_v<R>) {
			((FarThisFunction<R, Args...>)(funcAddress))(std::forward<Args>(args)...);
			popStackDef((sizeof...(args) - 1 - floatCount));
		}
		else {
			R ret = ((FarThisFunction<R, Args...>)(funcAddress))(std::forward<Args>(args)...);
			popStackDef((sizeof...(args) - 1 - floatCount));
			return ret;
		}
	}

	template<typename C, typename R, typename... Args>
	using FarThisFunction2 = R(__thiscall far C::*)(Args...);

	template<typename C, typename R = void, typename... Args>
	static R callThisFarA2(C* self, unsigned int funcAddress, Args... args) {
		funcAddress -= usedBase;
		funcAddress += FunctionPatcher::get()->getBaseAdress();

		FarThisFunction2<C, R, Args...> funcPtr;
		*((unsigned int*)&funcPtr) = funcAddress;
		return (self->*funcPtr)(args...);
	}

	template<typename R, typename... Args>
	using FarCdeclFunction = R(__cdecl far*)(Args...);

	template<typename R = void, typename... Args>
	static R callCdeclFarA(unsigned int funcAddress, Args... args) {
		funcAddress -= usedBase;
		funcAddress += FunctionPatcher::get()->getBaseAdress();

		if constexpr (std::is_void_v<R>) {
			((FarCdeclFunction<R, Args...>)(funcAddress))(args...);
			_asm {
				nop //force stack cleanup
			}
		}
		else {
			R ret = ((FarCdeclFunction<R, Args...>)(funcAddress))(args...);
			_asm {
				nop //force stack cleanup
			}
			return ret;
		}

		

	}

#pragma runtime_checks( "", restore)

private:
	unsigned int m_baseAddress = 0;

};

using FP = FunctionPatcher;
