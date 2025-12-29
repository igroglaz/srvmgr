#include "a2types.h"
#include "config_new.h"
#include "lib\utils.hpp"
#include "log.h"
#include "server_state.h"
#include "this_call.h"

#include <cstdlib>
#include <windows.h>

void stop_server() {
    exit(0);
}

void RestartProcess() {
    // Get full path and command line of the current executable.
    WCHAR path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);

    LPWSTR args = GetCommandLineW();

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    Log() << "Restarting server on map change...";

    // Start a new process.
    auto created = CreateProcessW(
        path,       // Path to .exe
        args,       // Full command line with arguments
        NULL, NULL, // Process/thread security
        FALSE,      // Do not inherit handles
        0,          // Creation flags
        NULL, NULL, // Environment + current directory
        &si, &pi    // Startup info / process info
    );

    if (created) {
        Log() << "Restarting server: new instance created successfully, cleaning up current process";

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        ExitProcess(0);
    } else {
        Log() << "Restarting server: failed to create new process, error " << GetLastError() << ". Retaining the current process";
    }
}

void MapRotation() {
    const auto* map_times = (A2Array<int32_t>*)0x006d1618;
    const int map_index = *(int *)0x006d1634;

    Log() << "Rolling maps, new index: " << map_index << " out of " << map_times->size;

    server_state.map_index = map_index;
    server_state.Save();

    if (Config::server_restart_on_map_change) {
        RestartProcess();
        return;
    }

    if (!Config::server_rotate_maps && map_index == 0) {
        Log() << "Stopping server after the last map";
        stop_server();
    }
}

// Address: 0048aba4
int __declspec(naked) map_rotation() { 
    __asm {
        call MapRotation
        ret
        // Original instruction is unused.
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
