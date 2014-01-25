#include <stdio.h>
#include "charcheck.h"
#include "shared.h"
#include <shellapi.h>
#pragma warning(disable:4996) // no_deprecate


int __stdcall check_by_exe(char *login_name, int id_1, int id_2)
{ 
	unsigned long exit=0;   
	SHELLEXECUTEINFO inf={0};
	char cmd[] = "D:\\Games\\Allods\\PlScript\\a2c2.pl D:\\Games\\Allods\\Chr\\%c\\%s.lgn %u %u";
	char cmd2[BUF_MAX];
	_snprintf (cmd2, BUF_MAX-1, cmd, login_name[0], login_name, id_1, id_2);
	inf.cbSize=sizeof (inf);
	inf.hwnd = NULL;
	inf.nShow = SW_HIDE;
	inf.lpFile = "C:\\Perl\\bin\\perl.exe";
	inf.lpParameters = cmd2;
	inf.lpDirectory = "C:\\Games\\Allods\\PlScript";
	inf.fMask=SEE_MASK_NOCLOSEPROCESS;
	inf.lpVerb = "open";
	ShellExecuteEx (&inf);
	WaitForSingleObject(inf.hProcess, INFINITE);
	GetExitCodeProcess(inf.hProcess,&exit ); 
	return exit;
}
