#pragma once

#include "zxmgr.h"

struct ClientScreenshot
{
    byte* SourcePlayer;
    byte* TargetPlayer;
    uint32_t RequestedAt;
    uint32_t UID;
};

uint32_t ClientScreenshot_Enqueue(byte* gm, byte* target);
ClientScreenshot* ClientScreenshot_FindByUID(uint32_t uid);
void ClientScreenshot_Drop(uint32_t uid);
void ClientScreenshot_DropPlayer(byte* player);