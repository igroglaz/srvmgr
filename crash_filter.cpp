#include "syslib.h"
#include "srvmgr.h"

int exc = 0;
char awarn[]="Warning: recursive exception, characters unsaved!\n";
char asavi[]="Saving characters...\n";

void _declspec(naked) exc_handler(void) 
{
	__asm
	{ //6081F0
		push	offset asavi
		call	log_format
		call	upd_all
		retn
	}
}

bool exception_already = false;
bool exception_secondary = false;

DWORD exc_handler_run(struct _EXCEPTION_POINTERS *info)
{
	__try
	{
		// dump info
		log_format("EXCEPTION DUMP:\neax=%08Xh,ebx=%08Xh,ecx=%08Xh,edx=%08Xh,\nesp=%08Xh,ebp=%08Xh,esi=%08Xh,edi=%08Xh;\neip=%08Xh;\naddr=%08Xh,code=%08Xh,flags=%08Xh\n",
				info->ContextRecord->Eax,
				info->ContextRecord->Ebx,
				info->ContextRecord->Ecx,
				info->ContextRecord->Edx,
				info->ContextRecord->Esp,
				info->ContextRecord->Ebp,
				info->ContextRecord->Esi,
				info->ContextRecord->Edi,
				info->ContextRecord->Eip,
				info->ExceptionRecord->ExceptionAddress,
				info->ExceptionRecord->ExceptionCode,
				info->ExceptionRecord->ExceptionFlags);

		log_format("BEGIN STACK TRACE: 0x%08Xh <= ", info->ExceptionRecord->ExceptionAddress);
		unsigned long stebp = *(unsigned long*)(info->ContextRecord->Ebp);
		while(true)
		{
			bool bad_ebp = false;
			if(stebp & 3) bad_ebp = true;
			if(!bad_ebp && IsBadReadPtr((void*)stebp, 8)) bad_ebp = true;

			if(bad_ebp) /* ? */ break;

			log_format2("%08Xh <= ", *(unsigned long*)(stebp+4));
			stebp = *(unsigned long*)(stebp); // o_O
		}
		log_format2("END STACK TRACE\n");
		
		ExitProcess(1);
	}
	__except(EXCEPTION_EXECUTE_HANDLER) { /* empty */ }

	return EXCEPTION_EXECUTE_HANDLER;
}

void SetExceptionFilter()
{
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&exc_handler_run);
}