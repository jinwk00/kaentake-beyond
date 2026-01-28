#include "pch.h"

#include "bosshp.h"
#include "hook.h"
#include "wvs/util.h"

#include <cstdio>

namespace {
constexpr int kBaseScreenWidth = 800;
constexpr int kBossHpTooltipBaseY = 37;
constexpr int kBossHpTooltipXOffset = 0;
constexpr int kBossHpTooltipYOffset = 10;
constexpr DWORD kCUIMiniMapInstance = 0x00BED788;
constexpr DWORD kCFieldShowMobHpTag = 0x005336CA;
constexpr DWORD kCFieldInit = 0x00528DBC;
constexpr DWORD kCFieldDispose = 0x00529035;
constexpr DWORD kCUIToolTipSetToolTipString = 0x008E6E7D;
constexpr DWORD kCUIToolTipClearToolTip = 0x008E6E23;
constexpr DWORD kCUIToolTipDisposeToolTip = 0x008E6BA3;
constexpr DWORD kCUIToolTipCreateToolTip = 0x008E49B5;
constexpr DWORD kCUserLocalUpdate = 0x0094A144;

constexpr int kToolTipSize = 1304;

char g_boss_hp_tooltip[kToolTipSize];
double g_boss_hp_percentage = 0.0;

int ReadInt(DWORD dwAddress) {
    DWORD dwOldProtect = 0;
    VirtualProtect(reinterpret_cast<void*>(dwAddress), sizeof(int), PAGE_EXECUTE_READ, &dwOldProtect);
    int result = *reinterpret_cast<unsigned int*>(dwAddress);
    VirtualProtect(reinterpret_cast<void*>(dwAddress), sizeof(int), dwOldProtect, &dwOldProtect);
    return result;
}

int GetMiniMapWidth() {
    return ReadInt(ReadInt(kCUIMiniMapInstance) + 0x24);
}

int GetBossHpTooltipX() {
    return ((get_screen_width() - kBaseScreenWidth) / 2) + GetMiniMapWidth() + kBossHpTooltipXOffset;
}

int GetBossHpTooltipY() {
    return kBossHpTooltipBaseY + kBossHpTooltipYOffset;
}

using UIToolTipSetToolTipString = void(__fastcall*)(void* pThis, void* edx, int x, int y, const char* sToolTip);
static auto s_ui_tooltip_set_tooltip_string = reinterpret_cast<UIToolTipSetToolTipString>(kCUIToolTipSetToolTipString);

void SetToolTipString(void* instance, int x, int y, const char* sToolTip) {
    s_ui_tooltip_set_tooltip_string(instance, nullptr, x, y, sToolTip);
}

using UIToolTipClearToolTip = void(__fastcall*)(void* pThis, void* edx);
static auto s_ui_tooltip_clear_tooltip = reinterpret_cast<UIToolTipClearToolTip>(kCUIToolTipClearToolTip);

void ClearToolTip(void* instance) {
    s_ui_tooltip_clear_tooltip(instance, nullptr);
}

using UIToolTipDisposeToolTip = void(__fastcall*)(void* pThis, void* edx);
static auto s_ui_tooltip_dispose_tooltip = reinterpret_cast<UIToolTipDisposeToolTip>(kCUIToolTipDisposeToolTip);

void DisposeToolTip(void* instance) {
    s_ui_tooltip_dispose_tooltip(instance, nullptr);
}

using UIToolTipCreateToolTip = void(__fastcall*)(void* pThis, void* edx);
static auto s_ui_tooltip_create_tooltip = reinterpret_cast<UIToolTipCreateToolTip>(kCUIToolTipCreateToolTip);

void CreateToolTip(void* instance) {
    s_ui_tooltip_create_tooltip(instance, nullptr);
}

void DrawBossHpNumberIfNeeded() {
    if (g_boss_hp_percentage <= 0.0) {
        return;
    }

    char sToolTip[20] = {};
    std::snprintf(sToolTip, sizeof(sToolTip), "%.2f%%", g_boss_hp_percentage);
    SetToolTipString(&g_boss_hp_tooltip, GetBossHpTooltipX(), GetBossHpTooltipY(), sToolTip);
}

void DrawBossHpNumber(int nHP, int nMaxHP) {
    if (nHP > 0) {
        g_boss_hp_percentage = static_cast<double>(nHP) / static_cast<double>(nMaxHP) * 100.0;
        return;
    }

    g_boss_hp_percentage = 0.0;
    ClearToolTip(&g_boss_hp_tooltip);
}

void DisposeBossHpNumber() {
    g_boss_hp_percentage = 0.0;
    ClearToolTip(&g_boss_hp_tooltip);
}

static auto s_cuserlocal_update = reinterpret_cast<void(__thiscall*)(void*)>(kCUserLocalUpdate);
void __fastcall CUserLocalUpdate_hook(void* pThis, void* _EDX) {
    s_cuserlocal_update(pThis);
    DrawBossHpNumberIfNeeded();
}

static auto s_cfield_show_mob_hp_tag = reinterpret_cast<void(__thiscall*)(void*, unsigned int, int, int, int, int)>(kCFieldShowMobHpTag);
void __fastcall CFieldShowMobHpTag_hook(void* pThis, void* _EDX, unsigned int dwMobID, int nColor, int nBgColor, int nHP, int nMaxHP) {
    s_cfield_show_mob_hp_tag(pThis, dwMobID, nColor, nBgColor, nHP, nMaxHP);
    DrawBossHpNumber(nHP, nMaxHP);
}

static auto s_cfield_init = reinterpret_cast<void(__thiscall*)(void*)>(kCFieldInit);
void __fastcall CFieldInit_hook(void* pThis, void* _EDX) {
    if (g_boss_hp_percentage > 0.0) {
        DisposeBossHpNumber();
    }

    DisposeToolTip(&g_boss_hp_tooltip);
    CreateToolTip(&g_boss_hp_tooltip);
    s_cfield_init(pThis);
}

static auto s_cfield_dispose = reinterpret_cast<void(__thiscall*)(void*)>(kCFieldDispose);
void __fastcall CFieldDispose_hook(void* pThis, void* _EDX) {
    DisposeBossHpNumber();
    s_cfield_dispose(pThis);
}
} // namespace

void AttachBossHpMod() {
    ATTACH_HOOK(s_cuserlocal_update, CUserLocalUpdate_hook);
    ATTACH_HOOK(s_cfield_show_mob_hp_tag, CFieldShowMobHpTag_hook);
    ATTACH_HOOK(s_cfield_init, CFieldInit_hook);
    ATTACH_HOOK(s_cfield_dispose, CFieldDispose_hook);
}
