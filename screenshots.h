#pragma once

#include "zxmgr.h"

struct ClientScreenshot
{
    A2Player* SourcePlayer;
    A2Player* TargetPlayer;
    uint32_t RequestedAt;
    uint32_t UID;
};

uint32_t ClientScreenshot_Enqueue(A2Player* gm, A2Player* target);
ClientScreenshot* ClientScreenshot_FindByUID(uint32_t uid);
void ClientScreenshot_Drop(uint32_t uid);
void ClientScreenshot_DropPlayer(A2Player* player);
