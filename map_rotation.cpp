#include "config_new.h"
#include <stdlib.h>
#include "lib\utils.hpp"
#include "this_call.h"

void stop_server()
{
    exit(0);
}

void __stdcall reset_map_counter()
{
    if (Config::server_rotate_maps)
    {
        Printf("Rolling maps");
        int *server_map_index = (int *)(void *)0x006D1634;
        *server_map_index = 0;
    }
    else
    {
        Printf("Stopping server after the last map");
        stop_server();
    }
}

int __declspec(naked) imp_reset_map_counter()
{ 
    __asm
    {
        mov        edx, reset_map_counter
        call    edx
        ret
    }
}

void __fastcall NotifyAboutMapRotation(int32_t remaining_ms, int32_t throttle_div_16) {
    const int game_mode = *(int*)0x006d1648;
    if (game_mode != 0 && game_mode != 3) {
        return;
    }

    int remaining = remaining_ms / 1000;
    int throttle = throttle_div_16 << 4;

    // Vanilla logic: post "server will restart in X minutes" every minute for the last 10 minutes.
    if (1 < remaining && remaining < 660 && (remaining_ms % 60000) < throttle) {
        this_call(0x0051ce86, (void*)0x006c3a08, (void*)12, (void*)remaining, (void*)0);
    }

    // Vanilla logic: post "server will restart in X seconds" every 15 sec for the minute.
    if (1 < remaining && remaining < 60 && (remaining_ms % 15000) < throttle) {
        this_call(0x0051ce86, (void*)0x006c3a08, (void*)12, (void*)remaining, (void*)0);
    }

    // This is new logic. Post "server will restart in X seconds" every second for the last 10 seconds.
    if (remaining <= 10 && (remaining_ms % 1000) < throttle) {
        this_call(0x0051ce86, (void*)0x006c3a08, (void*)12, (void*)remaining, (void*)0);
    }
}

// Address: 0048aa87
void __declspec(naked) notify_about_map_rotation() {
    __asm {
        mov ecx, eax  // Remaining map time in ms.
        mov edx, DWORD PTR [ebp-0x24]  // Main window.
        mov edx, DWORD PTR [edx+0x42c]  // Last notification sent time, I guess in server tics.

        call NotifyAboutMapRotation

        mov edx, 0x0048ab38
        jmp edx
    }
}
