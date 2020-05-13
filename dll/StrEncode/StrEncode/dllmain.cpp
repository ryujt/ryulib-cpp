// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

extern "C" {
#include <iconv.h>
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) bool change_charset(const char *szSrcCharset, const char *szDstCharset, char *szSrc, int nSrcLength, char *szDst, int nDstLength)
{
    iconv_t it = iconv_open(szDstCharset, szSrcCharset);
    if (it == (iconv_t)(-1)) return false;
    bool result = true;
    ZeroMemory(szDst, nDstLength);
    size_t nSrcStrLen = nSrcLength;
    size_t nDstStrLen = nDstLength;
    size_t cc = iconv(it, &szSrc, &nSrcStrLen, &szDst, &nDstStrLen);
    if (cc == (size_t)(-1)) result = false;
    if (iconv_close(it) == -1) result = false;
    return result;
}
