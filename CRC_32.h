/****************************************************************************
CRC_32.h : header file for the CRC_32 class
written by PJ Arends
pja@telus.net

based on the CRC-32 code found at
http://www.createwindow.com/programming/crc32/crcfile.htm

For updates check http://www3.telus.net/pja/crc32.htm

-----------------------------------------------------------------------------
This code is provided as is, with no warranty as to it's suitability or usefulness
in any application in which it may be used. This code has not been tested for
UNICODE builds, nor has it been tested on a network ( with UNC paths ).

This code may be used in any way you desire. This file may be redistributed by any
means as long as it is not sold for profit, and providing that this notice and the
authors name are included.

If any bugs are found and fixed, a note to the author explaining the problem and
fix would be nice.
-----------------------------------------------------------------------------
****************************************************************************/

#ifndef _CRC_32_H_EA6C0EE0_BC30_11d5_B625_A58C4DF45B22_INCLUDED
#define _CRC_32_H_EA6C0EE0_BC30_11d5_B625_A58C4DF45B22_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WINDOWS_
#include <windows.h>
#endif // _WINDOWS_

#define WM_CRC_THREAD_DONE WM_APP + 0x2DB1

class CRC_32
{
    typedef struct tag_CRCStruct
    {
        CRC_32 *pCRC_32;
        TCHAR FileName[_MAX_PATH];
        LPBYTE pByte;
        UINT size;
        HWND hWnd;
        HANDLE Thread;
    } CRCStruct, *LPCRCStruct;

public:
    CRC_32();
    DWORD CalcCRC(LPCTSTR FileName, HWND ProgressWnd = NULL);
    DWORD CalcCRC(LPVOID buffer, UINT size, HWND ProgressWnd = NULL);

private:
    static DWORD WINAPI CRC32ThreadProc(LPVOID lpVoid);
    void Calculate (const LPBYTE buffer, UINT size, ULONG &crc);
    ULONG Reflect(ULONG ref, char ch);
    ULONG Table[256];
};

#endif // _CRC_32_H_EA6C0EE0_BC30_11d5_B625_A58C4DF45B22_INCLUDED

/////////////////////////////////////////////////////////////////////////////
//
//  End of CRC_32.h
//
/////////////////////////////////////////////////////////////////////////////
