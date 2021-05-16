#ifndef KBHOOK
#define KBHOOK

#ifdef __cplusplus
extern "C" {
#endif

#include <macros.h>
#include <Windows.h>

#ifdef LOGME_MSVC
#define DllExport __declspec( dllexport )
#endif // LOGME_MSVC

DllExport LRESULT CALLBACK KBHOOK_KeyboardProc____(int nCode, WPARAM wParam, LPARAM lParam);

DllExport void UninstallHook(HANDLE hook, HANDLE* hookaddr, HANDLE shareobj, HANDLE* shareobj_addr);

DllExport void InstallHook(int* hooksuccess, HANDLE* hookaddr, HANDLE* sharedobj_addr);

#ifdef __cplusplus
}
#endif

#endif // !KBHOOK

