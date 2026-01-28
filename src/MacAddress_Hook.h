#pragma once

#include <windows.h>

#include "ztl/zstr.h"

namespace MacAddress_Hook {

    extern DWORD dwGetLocalMacAddress;

    using tGetLocalMacAddressWithHDDSerialNo = void(__thiscall*)(void* this_ptr, ZXString<char>* sMacAddress);

    extern tGetLocalMacAddressWithHDDSerialNo oGetLocalMacAddressWithHDDSerialNo;

    void BuildFakeMacString(ZXString<char>* sMacAddress);
    void __fastcall Hooked_GetLocalMacAddressWithHDDSerialNo(void* this_ptr, void* edx, ZXString<char>* sMacAddress);
    void Attach();

} // namespace MacAddress_Hook
