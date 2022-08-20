#include "screenshots.h"
#include <ctime>
#include "config_new.h"

static std::vector<ClientScreenshot> Screenshots;
static int ScreenshotUID = 0;

uint32_t ClientScreenshot_Enqueue(byte* gm, byte* target)
{
    ClientScreenshot cs;
    cs.SourcePlayer = gm;
    cs.TargetPlayer = target;
    cs.RequestedAt = time(NULL);
    cs.UID = Config::IPAddressP << 16 | (++ScreenshotUID);
    Screenshots.push_back(cs);
    return cs.UID;
}

ClientScreenshot* ClientScreenshot_FindByUID(uint32_t uid)
{
    for (std::vector<ClientScreenshot>::iterator it = Screenshots.begin(); it != Screenshots.end(); ++it)
    {
        ClientScreenshot& cs = (*it);
        if (cs.UID == uid)
            return &cs;
    }
    return nullptr;
}

void ClientScreenshot_Drop(uint32_t uid)
{
    for (std::vector<ClientScreenshot>::iterator it = Screenshots.begin(); it != Screenshots.end(); ++it)
    {
        ClientScreenshot& cs = (*it);
        
        if (cs.UID == uid)
        {
            std::vector<ClientScreenshot>::iterator prev = it-1;
            Screenshots.erase(it);
            it = prev;
        }
    }
}

void ClientScreenshot_DropPlayer(byte* player)
{
    for (std::vector<ClientScreenshot>::iterator it = Screenshots.begin(); it != Screenshots.end(); ++it)
    {
        ClientScreenshot& cs = (*it);
        if (cs.SourcePlayer == player)
            cs.SourcePlayer = nullptr;
        if (cs.TargetPlayer == player)
            cs.TargetPlayer = nullptr;
    }
}