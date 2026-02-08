#pragma once

#include <string>

class Client {
public:
    static int m_nGameHeight;
    static int m_nGameWidth;
    static int MsgAmount;
    static bool WindowedMode;
    static bool RemoveLogos;
    static int speedMovementCap;
    static std::string ServerIP_AddressFromINI;
};
