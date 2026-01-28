#include "pch.h"

#include "client.h"

int Client::m_nGameHeight = 720;
int Client::m_nGameWidth = 1280;
int Client::MsgAmount = 26;
bool Client::WindowedMode = true;
bool Client::RemoveLogos = true;
int Client::speedMovementCap = 140;
std::string Client::ServerIP_AddressFromINI = "127.0.0.1";
// Beta Server: 139.99.101.70
// Launch Server: 51.178.31.171
// localhost: 127.0.0.1
