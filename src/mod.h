#pragma once

#include <windows.h>

// Export declarations for the module functions
extern "C" {
    __declspec(dllexport) void Module_OnDllAttach();
    __declspec(dllexport) void Module_OnLogoEnd();
}

namespace mod {
    bool Initialize();
    bool IsInitialized();
}
