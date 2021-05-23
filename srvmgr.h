#pragma once
#include <windows.h>

#pragma warning(disable:4996)

#define _CRT_SECURE_NO_DEPRECATE

#define DENY_CONNECT_FLAG 0x00001
#define PVM_FLAG 0x00002
#define DISABLE_CHAT_FLAG 0x00004
#define CHECK1_FLAG 0x00008
#define PKILL_OWN 0x00010
#define TEST_FLAG 0x00020
#define _scanf 0x05C0A50          ///// int _cdecl sscanf(const char *,const char *,...)

#define BUF_SIZE 255
#define BUF_MAX 500
//#define SRV_LOG


#include <string>
#include <vector>

typedef VOID FAR* PVOID; 

extern char aErrPoint[];
extern char aArea_Cast[];
extern char clas[BUF_MAX];
extern char text[BUF_MAX];

extern int(__cdecl *print_log)(char *message);
void to_encoding(char *s);
void to_encoding(std::string &s);
void from_encoding(std::string &s);
void from_encoding(char *s);
void from_koi(char *s);
void from_win(char *s);
void to_koi(char *s);
void to_win(char *s);

void __stdcall set_mode_2(char*s);

#ifndef type_mydata
#define type_mydata
typedef struct _MyData{
	HINSTANCE hModule;
} MYDATA, *PMYDATA;
#endif

extern HANDLE hThread;
extern char avisall[];
extern HWND MainWnd;
extern DWORD dwThreadId;

enum Encoding { KOI8, CP1251, CP866 };

DWORD WINAPI ThreadProc(LPVOID lpParam);
DWORD WINAPI DebugProc(LPVOID lpParam);
void * __stdcall get_player_by_name(const char *name);

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int InitApplication(HINSTANCE,int);
void __stdcall log_format(char *s, ...);
void __stdcall log_format2(char *s, ...);
void __stdcall log_saving(char* s);
void __stdcall log_saved(void);

void cancel_camp(void);
void public_chat(void);
void private_chat(void);
void shout_chat(void);
void part_of_save(void);
void part_of_save_char(void);
void kick_char(void);
void part_of_disconnect(void);
void part_of_camp(void);
void kick_all(void);
void kick_by_name(const char *s);
void kick_by_name_2();
void upd_all(void); 
void map_exit(void);
int verify_damage(int p1, int p2, int damage);/// p2 - 믣졡檬 p1 - 볮 ⽥뉶oid setthreadp(void);
void __stdcall send_to_player(void *p, const char *msg);
void __stdcall set_mode(int *p, char*s);
void __stdcall print_error(int*a);
void say_packet(void);
void testeaxifzero(void);
SOCKET PASCAL accept0(SOCKET s, sockaddr *addr, int *addrlen);
int PASCAL send0(SOCKET s, const char *buf, int len, int flags);
int PASCAL closesocket0(SOCKET s);
int PASCAL recv0(SOCKET s, char *buf, int len, int flags);
int __stdcall test_if_valid(int id_2, int id_1, char *login_name); 
int __stdcall print_zzz(char*s, int z);
void kick(const char *name);
int process_packet();
void max_players(void);
void cheat_codes(void);
void vision_of_all(void);
int _stdcall test_if_visible(u_char x, u_char y, short vis, u_char x2, u_char y2);
void _stdcall say_all(const char *s);
void say_all2(void);
void __stdcall cheat_say(const char *nick, const char *cmd);

void __stdcall log_format(char *s, ...);
