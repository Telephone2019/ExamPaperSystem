#ifndef KBHOOK
#define KBHOOK

#ifdef __cplusplus
extern "C" {
#endif

#include "macros.h"
#include <Windows.h>

#ifdef LOGME_MSVC
#define DllExport __declspec( dllexport )
#endif // LOGME_MSVC

typedef enum HookType
{
	HT_KEYBOARD,
	HT_MOUSE
} HookType;

DllExport void UninstallHook(HANDLE hook, HANDLE* hookaddr, HANDLE shareobj, HANDLE* shareobj_addr);

DllExport void InstallHook(int* hooksuccess, HANDLE* hookaddr, HANDLE* sharedobj_addr, HookType type);

#ifdef __cplusplus
}
#endif

#endif // !KBHOOK

