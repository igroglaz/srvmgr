#pragma once

#define _WIN32_WINNT 0x0501

#include <winsock2.h>
#include <windows.h>
#include <stdint.h>

#undef SendMessage
#undef GetCurrentTime

typedef uint8_t byte;
