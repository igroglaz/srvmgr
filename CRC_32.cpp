/****************************************************************************
CRC_32.cpp : implementation file for the CRC_32 class
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

created : October 2001
      
Revision History:

October 11, 2001   - changed SendMessage to PostMessage in CRC32ThreadProc

****************************************************************************/

#include "CRC_32.h"
#include <io.h>         // _close(), _read()
#include <fcntl.h>      // _O_* flags
#include <share.h>      // _SH_DENYWR
#include <tchar.h>      // UNICODE compatibility
#include <commctrl.h>   // PBM_* progress bar messages

// use a 100 KB buffer (a larger buffer does not seem to run
// significantly faster, but takes more RAM)
#define BUFFERSIZE 102400

/////////////////////////////////////////////////////////////////////////////
//
//  CRC_32::CRC32ThreadProc  (static private member function)
//    Calculates the CRC-32 value for a file or data buffer.
//
//  Parameters :
//    lpVoid [in] : A pointer to a CRCStruct structure, type casted as a void pointer
//
//  Returns :
//    The CRC-32 value of the supplied buffer or file
//
//  Note :
//    If this function is started as a thread, it will send PBM_* messages to the supplied
//    progress bar control. When the thread is finished, this function will send a 
//    WM_CRC_THREAD_DONE message to the parent window of the progress bar. the WPARAM parameter
//    will contain the HANDLE of the thread, and the LPARAM will contain the CRC-32 value
//    of the supplied buffer or file.
//
/////////////////////////////////////////////////////////////////////////////

DWORD WINAPI CRC_32::CRC32ThreadProc(LPVOID lpVoid)
{
    LPCRCStruct pCRCSt = (LPCRCStruct)lpVoid;
    ULONG CRC = 0xFFFFFFFF;
    HWND Progress = NULL;
    if (::IsWindow(pCRCSt->hWnd))
    {   // setup the progress bar
        Progress = pCRCSt->hWnd;
        ::PostMessage(Progress, PBM_SETPOS, 0, 0);
        ::PostMessage(Progress, PBM_SETRANGE32, 0, 100);
    }
    if (pCRCSt->pByte)
    {   // calculate CRC for a buffer
        for (UINT offset = 0; offset < pCRCSt->size; offset += BUFFERSIZE)
        {
            pCRCSt->pCRC_32->Calculate(pCRCSt->pByte + offset,
                                       (pCRCSt->size - offset > BUFFERSIZE) ? BUFFERSIZE : (pCRCSt->size - offset),
                                       CRC);
            if(::IsWindow(Progress))
            {
                int percent = offset > pCRCSt->size ? 100 : (int)(((double)offset / (double)pCRCSt->size) * 100);
                ::PostMessage(Progress, PBM_SETPOS, percent, 0);
            }
        }
    }
    else if (pCRCSt->FileName)
    {   // calculate CRC for a file
        LONGLONG done = 0;
        UINT size = BUFFERSIZE;
        BYTE buffer[BUFFERSIZE];

        // Open the file
        int theFile = _tsopen(pCRCSt->FileName, _O_RDONLY | _O_SEQUENTIAL | _O_BINARY, _SH_DENYWR);
        if (theFile != -1)
        {   // Get the file size
            _lseeki64(theFile, 0L, SEEK_SET);
            LONGLONG length = _lseeki64(theFile, 0L, SEEK_END);
            _lseeki64(theFile, 0L, SEEK_SET);
        
            // process the file
            while (size == BUFFERSIZE)
            {
                size = _read(theFile, buffer, BUFFERSIZE);
                if (size)
                {
                    pCRCSt->pCRC_32->Calculate(buffer, size, CRC);
                    if (::IsWindow(Progress))
                    {   // update the progress bar
                        done += size;
                        int percent = (int)(((long double)done / (long double)length) * 100);
                        ::PostMessage(Progress, PBM_SETPOS, percent, 0);
                    }
                }
            }
            _close(theFile);
        }
    }

    CRC ^= 0xFFFFFFFF;
    
    if(IsWindow(Progress))
        // notify dialog that we are done
        ::PostMessage(::GetParent(Progress), WM_CRC_THREAD_DONE, (WPARAM)pCRCSt->Thread, CRC);
    
    // clean up
    // Known Problem : If the thread is prematurely terminated,
    // this cleanup code is never run, resulting in a memory leak.
    delete pCRCSt->pByte;
    delete pCRCSt;

    // return our CRC-32 value
    return CRC;
}

/////////////////////////////////////////////////////////////////////////////
//
//  CRC_32 constructor  (public member function)
//    Sets up the CRC-32 reference table
//
//  Parameters :
//    None
//
//  Returns :
//    Nothing
//
/////////////////////////////////////////////////////////////////////////////

CRC_32::CRC_32()
{
    // This is the official polynomial used by CRC-32 
    // in PKZip, WinZip and Ethernet. 
    ULONG ulPolynomial = 0x04C11DB7;

    // 256 values representing ASCII character codes.
    for (int i = 0; i <= 0xFF; i++)
    {
        Table[i] = Reflect(i, 8) << 24;
        for (int j = 0; j < 8; j++)
            Table[i] = (Table[i] << 1) ^ (Table[i] & (1 << 31) ? ulPolynomial : 0);
        Table[i] = Reflect(Table[i], 32);
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  CRC_32::Reflect  (private member function)
//    used by the constructor to help set up the CRC-32 reference table
//
//  Parameters :
//    ref [in] : the value to be reflected
//    ch  [in] : the number of bits to move
//
//  Returns :
//    the new value
//
/////////////////////////////////////////////////////////////////////////////

ULONG CRC_32::Reflect(ULONG ref, char ch)
{
    ULONG value = 0;

    // Swap bit 0 for bit 7
    // bit 1 for bit 6, etc.
    for(int i = 1; i < (ch + 1); i++)
    {
        if (ref & 1)
            value |= 1 << (ch - i);
        ref >>= 1;
    }
    return value;
}

/////////////////////////////////////////////////////////////////////////////
//
//  CRC_32::Calculate  (private member function)
//    Calculates the CRC-32 value for the given buffer
//
//  Parameters :
//    buffer [in] : pointer to the data bytes
//    size   [in] : the size of the buffer
//    CRC    [in] : the initial CRC-32 value
//          [out] : the new CRC-32 value
//
//  Returns :
//    Nothing
//
/////////////////////////////////////////////////////////////////////////////

void CRC_32::Calculate(const LPBYTE buffer, UINT size, ULONG &CRC)
{   // calculate the CRC
    LPBYTE pbyte = buffer;

    while (size--)
        CRC = (CRC >> 8) ^ Table[(CRC & 0xFF) ^ *pbyte++];
}

/////////////////////////////////////////////////////////////////////////////
//
//  CRC_32::CalcCRC  (public member function)
//    calculates the CRC-32 value for the given buffer
//
//  Parameters :
//    buffer      [in] : a pointer to the data bytes
//    size        [in] : the size of the buffer
//    ProgressWnd [in] : the HWND of the progress bar
//
//  Returns :
//    if ProgressWnd is not a window returns the CRC-32 value of the buffer
//    if ProgressWnd is a window, returns the HANDLE of the created thread
//    returns NULL if an error occurs
//
//  Note :
//    ProgressWnd is passed through the IsWindow() API function. If IsWindow()
//    returns zero, CalcCRC() will calculate the CRC-32 value directly. if IsWindow()
//    returns nonzero, CalcCRC() will start a seperate thread. The thread will send 
//    PBM_* progress bar messages to the ProgressWnd. When the thread is finished,
//    the thread will send a WM_CRC_THREAD_DONE message to the parent window of the
//    ProgressWnd. the WPARAM parameter will contain the HANDLE of the thread, and
//    the LPARAM will contain the CRC-32 value of the buffer.
//
/////////////////////////////////////////////////////////////////////////////

DWORD CRC_32::CalcCRC(LPVOID buffer, UINT size, HWND ProgressWnd/*= NULL*/)
{   // check the validity of the data
    if (!buffer || !size)
        return 0;

    if (!IsWindow(ProgressWnd))
    {   // calculate CRC directly
        DWORD CRC = 0xFFFFFFFF;
        Calculate ((LPBYTE)buffer, size, CRC);
        return CRC ^ 0xFFFFFFFF;
    }

    // start the thread
    LPCRCStruct pCRCSt = new CRCStruct();
    DWORD ThreadID;
    HANDLE Handle = ::CreateThread(NULL, 0, CRC32ThreadProc, (LPVOID)pCRCSt, CREATE_SUSPENDED, &ThreadID);
    if (Handle)
    {   // thread successfully created
        pCRCSt->pCRC_32 = this;
        pCRCSt->FileName[0] = 0;
        pCRCSt->pByte = new BYTE[size];
        memcpy(pCRCSt->pByte, buffer, size);
        pCRCSt->size = size;
        pCRCSt->hWnd = ProgressWnd;
        pCRCSt->Thread = Handle;
        ::ResumeThread(Handle);
    }
    else // thread creation failed, clean up
        delete pCRCSt;
    return (DWORD)Handle;
}

/////////////////////////////////////////////////////////////////////////////
//
//  CRC_32::CalcCRC  (public member function)
//    calculates the CRC-32 value for the given buffer
//
//  Parameters :
//    FileName    [in] : the complete path to the file
//    ProgressWnd [in] : the HWND of the progress bar
//
//  Returns :
//    if ProgressWnd is not a window returns the CRC-32 value of the file
//    if ProgressWnd is a window, returns the HANDLE of the created thread
//    returns NULL if an error occurs
//
//  Note :
//    ProgressWnd is passed through the IsWindow() API function. If IsWindow()
//    returns zero, CalcCRC() will calculate the CRC-32 value directly. if IsWindow()
//    returns nonzero, CalcCRC() will start a seperate thread. The thread will send 
//    PBM_* progress bar messages to the ProgressWnd. When the thread is finished,
//    the thread will send a WM_CRC_THREAD_DONE message to the parent window of the
//    ProgressWnd. the WPARAM parameter will contain the HANDLE of the thread, and
//    the LPARAM will contain the CRC-32 value of the file.
//
/////////////////////////////////////////////////////////////////////////////

DWORD CRC_32::CalcCRC(LPCTSTR FileName, HWND ProgressWnd/*= NULL*/)
{   // make sure the file exists and is not a directory
    DWORD attrib = ::GetFileAttributes(FileName);
    if (attrib == 0xFFFFFFFF || attrib & FILE_ATTRIBUTE_DIRECTORY)
        return 0;

    // setup the CRCStruct
    LPCRCStruct pCRCSt = new CRCStruct();
    pCRCSt->pCRC_32 = this;
    _tcsncpy(pCRCSt->FileName, FileName, _MAX_PATH);
    pCRCSt->pByte = NULL;
    pCRCSt->size = 0;
    pCRCSt->hWnd = ProgressWnd;
    pCRCSt->Thread = NULL;

    if (!IsWindow(ProgressWnd))
    {   // calculate CRC directly
        return CRC32ThreadProc((LPVOID)pCRCSt);
    }

    // start the thread
    DWORD ThreadID;
    HANDLE Handle = ::CreateThread(NULL, 0, CRC32ThreadProc, (LPVOID)pCRCSt, CREATE_SUSPENDED, &ThreadID);
    if (Handle)
    {   // thread successfully created
        pCRCSt->Thread = Handle;
        ::ResumeThread(Handle);
    }
    else // thread creation failed, clean up
        delete pCRCSt;
    return (DWORD)Handle;
}

/////////////////////////////////////////////////////////////////////////////
//
//  End of CRC_32.cpp
//
/////////////////////////////////////////////////////////////////////////////



    
