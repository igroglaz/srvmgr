#pragma once

void _stdcall say_all(const char *s);
void _declspec(naked) kick_all(void);
void _declspec(naked) upd_all(void);
void _declspec(naked) kick_char(void);

extern int(__cdecl *print_log)(char *message);