#include "pch.h"

#include "client_patches.h"

#include "client.h"
#include "hook.h"

#include <vector>

namespace {

constexpr BYTE kJmpOpcode = 0xE9;
constexpr DWORD kPetEquipCapRemoval = 0x0046D473 - (0x0046D43B + 5);

void FillBytes(uintptr_t address, unsigned char value, size_t count) {
    std::vector<unsigned char> bytes(count, value);
    PatchMemory(TO_PVOID(address), bytes.data(), bytes.size());
}

void WriteByte(uintptr_t address, unsigned char value) {
    PatchMemory(TO_PVOID(address), &value, sizeof(value));
}

void WriteInt(uintptr_t address, unsigned int value) {
    PatchMemory(TO_PVOID(address), &value, sizeof(value));
}

void WriteDouble(uintptr_t address, double value) {
    PatchMemory(TO_PVOID(address), &value, sizeof(value));
}

} // namespace

void ApplyClientPatches() {
    // Gameplay
    WriteInt(0x00780743 + 3, Client::speedMovementCap); // set speed cap
    WriteInt(0x008C4286 + 1, Client::speedMovementCap); // set speed cap
    WriteInt(0x0094D91E + 1, Client::speedMovementCap); // set speed cap

    // No Whack - jump from 0x009698BC to 0x00969A39
    WriteByte(0x009698BC, kJmpOpcode); // jmp opcode
    WriteInt(0x009698BC + 1, 0x00969A39 - (0x009698BC + 5)); // rel32 = dst - (src+5)

    // No Breath Removal
    WriteByte(0x00452316, 0x7C);

    // Climbing Speed
    WriteInt(0x009CC6F9 + 2, 0x00C1CF80);
    WriteDouble(0x00C1CF80, 4.00);

    // Super Tubi
    WriteByte(0x00485C01 + 2, 0x90);

    // Monster magnet fix on static enemies
    FillBytes(0x0096C554, 0x90, 4);

    // Maker Skill Instant
    WriteByte(0x00826F92 + 2, 0x08);
    WriteByte(0x00826F92 + 3, 0x01);
    WriteByte(0x00826F92 + 4, 0x00);
    WriteByte(0x00826F92 + 5, 0x00);

    // PetEquip Cap Removal
    WriteByte(0x0046D43B, kJmpOpcode);
    WriteInt(0x0046D43B + 1, kPetEquipCapRemoval);

    // Damage
    WriteInt(0x008C3304 + 1, 2147483647);
    WriteInt(0x00AFE8A4 + 1, 2147483647);

    // Cosmetic / UI
    // Repeat same thing more than 3x (ChatSpam) & Infinite ChatBox
    WriteByte(0x004905EB, 0xEB);
    WriteByte(0x004CAA09, 0xEB); // Infinite chat 1 of 2 scroll through chat box
    WriteByte(0x004CAA84, 0xEB); // Infinite chat 2 of 2 scroll through chat box

    // Swear filter
    FillBytes(0x007A03C8, 0x90, 2); // remove 3rd party censor (also removes the ?? spam)

    // Transactional
    // Enable Keyboard for PIC
    FillBytes(0x004CA8BA, 0x90, 2);

    // Allow Cash Items to be Traded
    FillBytes(0x004F3FB8, 0x90, 6);
    FillBytes(0x004F3FC4, 0x90, 6);

    // Misc
    WriteByte(0x0040013E, 0x2F); // 4g edit, not sure if it still works after execution
}
