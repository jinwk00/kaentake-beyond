#include "pch.h"

#include "mod.h"

#include "hook.h"

static bool g_bInitialized = false;
static bool g_bLogoEndCalled = false;

namespace mod {
    bool Initialize() {
        if (g_bInitialized) {
            return true;
        }

        g_bInitialized = true;
        return true;
    }

    bool IsInitialized() {
        return g_bInitialized;
    }
} // namespace mod

extern "C" {

    __declspec(dllexport) void Module_OnDllAttach() {
        if (!mod::Initialize()) {
            return;
        }
    }

    __declspec(dllexport) void Module_OnLogoEnd() {
        if (!mod::IsInitialized()) {
            return;
        }

        if (g_bLogoEndCalled) {
            return;
        }

        AttachClientHooks();
        g_bLogoEndCalled = true;
    }
}
