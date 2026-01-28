#include "pch.h"

#include "MacAddress_Hook.h"
#include "debug.h"
#include "hook.h"

#include <cstdio>

namespace MacAddress_Hook {

    DWORD dwGetLocalMacAddress = 0x5D62D0;

    tGetLocalMacAddressWithHDDSerialNo oGetLocalMacAddressWithHDDSerialNo =
        reinterpret_cast<tGetLocalMacAddressWithHDDSerialNo>(0x005FCDED);

    void BuildFakeMacString(ZXString<char>* sMacAddress) {
        const unsigned char fake_mac[] = {0x00, 0x50, 0x56, 0xC0, 0x00, 0x01};
        const DWORD fake_hdd = 0x12345678;

        char result[32];
        int len = sprintf_s(result, "%02X%02X%02X%02X%02X%02X_%02X%02X%02X%02X",
            static_cast<unsigned char>(fake_mac[0]), static_cast<unsigned char>(fake_mac[1]),
            static_cast<unsigned char>(fake_mac[2]), static_cast<unsigned char>(fake_mac[3]),
            static_cast<unsigned char>(fake_mac[4]), static_cast<unsigned char>(fake_mac[5]),
            (fake_hdd >> 24) & 0xFF, (fake_hdd >> 16) & 0xFF,
            (fake_hdd >> 8) & 0xFF, fake_hdd & 0xFF);

        DEBUG_MESSAGE("MAC Hook: Fallback to fake MAC -> %s", result);
        sMacAddress->Assign(result, len);
    }

    void __fastcall Hooked_GetLocalMacAddressWithHDDSerialNo(void* this_ptr, void* /*edx*/, ZXString<char>* sMacAddress) {
        __try {
            oGetLocalMacAddressWithHDDSerialNo(this_ptr, sMacAddress);
            const char* mac = *sMacAddress;
            DEBUG_MESSAGE("MAC Hook: Real MAC used -> %s", mac ? mac : "");
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            DEBUG_MESSAGE("MAC Hook: GetAdaptersInfo crashed - using fake MAC");
            BuildFakeMacString(sMacAddress);
        }
    }

    void Attach() {
        AttachHook(reinterpret_cast<void**>(&oGetLocalMacAddressWithHDDSerialNo), Hooked_GetLocalMacAddressWithHDDSerialNo);

        DEBUG_MESSAGE("MAC Address Hook installed at 0x%08X", dwGetLocalMacAddress);
    }

} // namespace MacAddress_Hook
