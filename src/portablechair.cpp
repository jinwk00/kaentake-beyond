#include "pch.h"
#include "hook.h"
#include "wvs/util.h"
#include "ztl/ztl.h"
#include <memory>


static int g_bAdminHide = 0; // CAvatar->m_bAdminHide will be hijacked to pack bodyRelMove offsets

class CItemInfo : public TSingleton<CItemInfo, 0x00BE78D8> {
public:
    IWzPropertyPtr GetItemInfo(int32_t nItemID) {
        IWzPropertyPtr result;
        reinterpret_cast<IWzPropertyPtr*(__thiscall*)(CItemInfo*, IWzPropertyPtr*, int)>(0x005DA83C)(this, std::addressof(result), nItemID);
        return result;
    }
};

class CUser {
public:
    MEMBER_AT(IWzVector2DPtr, 0x1140, m_pBodyOrigin) // CAvatar->m_pBodyOrigin
    MEMBER_AT(int, 0x2AE0, m_bAdminHide)             // CAvatar->m_bAdminHide
    MEMBER_HOOK(void, 0x0093C7C3, SetActivePortableChair, int nItemID)
    MEMBER_HOOK(void, 0x00930B27, Update)
};

void CUser::SetActivePortableChair_hook(int nItemID) {
    CUser::SetActivePortableChair(this, nItemID);
    if (nItemID == 0) {
        m_bAdminHide = 0;
        return;
    }
    // Resolve bodyRelMove offsets
    IWzPropertyPtr pInfo = CItemInfo::GetInstance()->GetItemInfo(nItemID);
    if (!pInfo) {
        return;
    }
    IWzVector2DPtr pVector;
    IUnknownPtr pUnknown = pInfo->item[L"bodyRelMove"].GetUnknown();
    if (FAILED(pUnknown.QueryInterface(__uuidof(IWzVector2D), &pVector))) {
        return;
    }
    // Pack offset into m_bAdminHide field
    int packed = 0;
    packed |= (pVector->x & 0xFFFF) << 16;
    packed |= (pVector->y & 0xFFFF);
    m_bAdminHide = packed;
}

void CUser::Update_hook() {
    CUser::Update(this);
    int packed = m_bAdminHide;
    if (packed) {
        int x = (packed >> 16) & 0xFFFF;
        if (x & 0x8000) {
            x |= 0xFFFF0000;
        }
        int y = (packed & 0xFFFF);
        if (y & 0x8000) {
            y |= 0xFFFF0000;
        }
        m_pBodyOrigin->RelMove(x, y);
    }
}


static auto CUserLocal__IsAdminHide = reinterpret_cast<void(__thiscall*)(void*)>(0x00949130);
int __fastcall CUserLocal__IsAdminHide_hook(void* pThis, void* _EDX) {
    return g_bAdminHide;
}

static auto CField__OnAdminResult_jmp = 0x00535351;
static auto CField__OnAdminResult_ret = 0x00535357;
void __declspec(naked) CField__OnAdminResult_hook() {
    __asm {
        mov     [ g_bAdminHide ], eax
        jmp     [ CField__OnAdminResult_ret ]
    }
}

void AttachPortableChairMod() {
    // Handle bodyRelMove
    ATTACH_HOOK(CUser::SetActivePortableChair, CUser::SetActivePortableChair_hook);
    ATTACH_HOOK(CUser::Update, CUser::Update_hook);

    // Restore bIsAdminHide
    ATTACH_HOOK(CUserLocal__IsAdminHide, CUserLocal__IsAdminHide_hook);
    PatchJmp(CField__OnAdminResult_jmp, &CField__OnAdminResult_hook);
}