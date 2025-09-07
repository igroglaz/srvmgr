#include "crash_filter.h"

#include "syslib.h"
#include "srvmgr.h"

#include <windows.h>
#include <dbghelp.h>

int exc = 0;
char awarn[]="Warning: recursive exception, characters unsaved!\n";
char asavi[]="Saving characters...\n";

void _declspec(naked) exc_handler(void) 
{
    __asm
    { //6081F0
        push    offset asavi
        call    log_format
        call    upd_all
        retn
    }
}

bool exception_already = false;
bool exception_secondary = false;

void Traceback(CONTEXT* ctx) {
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();

    STACKFRAME64 frame = {};
    frame.AddrPC.Offset = ctx->Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ctx->Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = ctx->Esp;
    frame.AddrStack.Mode = AddrModeFlat;

    DWORD machineType = IMAGE_FILE_MACHINE_I386;

    SymInitialize(hProcess, NULL, TRUE);
    log_format("=== Stack Trace Start ===\n");

    for (int i = 0; i < 64; ++i) {
        if (!StackWalk64(
                machineType,
                hProcess,
                hThread,
                &frame,
                ctx,
                NULL,
                SymFunctionTableAccess64,
                SymGetModuleBase64,
                NULL))
            break;

        DWORD64 addr = frame.AddrPC.Offset;
        if (addr == 0)
            break;

        BYTE symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {};
        SYMBOL_INFO* symbol = (SYMBOL_INFO*)symbolBuffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        if (SymFromAddr(hProcess, addr, &displacement, symbol)) {
            log_format("  [0x%08llx] %s + 0x%llx\n", addr, symbol->Name, displacement);
        } else {
            log_format("  [0x%08llx] (no symbol)\n", addr);
        }
    }

    SymCleanup(hProcess);
    log_format("=== Stack Trace End ===\n");
}

DWORD exc_handler_run(EXCEPTION_POINTERS *info)
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

        PrintStackTrace(info->ContextRecord->Ebp);
        Traceback(info->ContextRecord);
        
        ExitProcess(1);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) { /* empty */ }

    return EXCEPTION_EXECUTE_HANDLER;
}

void SetExceptionFilter()
{
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&exc_handler_run);
}

void PrintStackTrace(int ebp) {
    log_format("BEGIN STACK TRACE: 0x%08Xh <= ", ebp);
    unsigned long stebp = *(unsigned long*)(ebp);
    while (true) {
        if ((stebp & 3) || IsBadReadPtr((void*)stebp, 8)) {
            break;
        }

        log_format2("%08Xh <= ", *(unsigned long*)(stebp+4));
        stebp = *(unsigned long*)(stebp);
    }
    log_format2("END STACK TRACE\n");
}
