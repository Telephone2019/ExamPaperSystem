#ifndef KBHOOK
#define KBHOOK

#include <macros.h>

#ifdef LOGME_MSVC
#define DllExport __declspec( dllexport )
#endif // LOGME_MSVC

DllExport int kbhook_run_success();

DllExport int InstallHook();
DllExport int UninstallHook();

DllExport int InstallHook_s();

#endif // !KBHOOK

