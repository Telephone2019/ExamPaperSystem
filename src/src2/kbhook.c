#ifdef __cplusplus
extern "C" {
#endif
#include "kbhook.h"

#include <stdio.h>

DllExport LRESULT CALLBACK KBHOOK_KeyboardProc____(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode < 0) {
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
	else
	{
		printf("vk = %llX\n", wParam);
		if (1)
		{
			return 0;
		}
		else
		{
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		}
	}
}

DllExport void UninstallHook(HANDLE hook, HANDLE* hookaddr, HANDLE shareobj, HANDLE* shareobj_addr) {
	if (hook == NULL) {
		return;
	}
	*hookaddr = *shareobj_addr = NULL;
	UnhookWindowsHookEx(hook);
	// you should not close the hook handle, because it is held by the system
	//CloseHandle(hook);
	ReleaseMutex(shareobj);
}

static void MessageLoop(HANDLE shareobj) {
	MSG msg;
	while (1) {
		if (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			DWORD waitres;
			while ((waitres = WaitForSingleObject(shareobj, 0)) == WAIT_FAILED);
			switch (waitres)
			{
			case WAIT_ABANDONED:
			case WAIT_OBJECT_0:
				CloseHandle(shareobj);
				return;
				break;
			default:
				break;
			}
		}
	}
}

static HANDLE SetHook() {
	HINSTANCE hinstDLL = LoadLibrary(TEXT("KBHook.dll"));
	if (!hinstDLL) {
		return NULL;
	}
	HOOKPROC hkprcSysMsg = (HOOKPROC)GetProcAddress(hinstDLL, "KBHOOK_KeyboardProc____");
	HHOOK hhookSysMsg = SetWindowsHookEx(WH_KEYBOARD_LL, hkprcSysMsg, hinstDLL, 0);
	if (hhookSysMsg == NULL) {
		CloseHandle(hinstDLL);
	}
	return hhookSysMsg;
}

typedef struct MyHookKey_Parameter {
	HANDLE* hookaddr;
	HANDLE sharedobject;
	int* hooksuccess;
} MyHookKey_Parameter;

static DWORD WINAPI MyHookKey(_In_ LPVOID lpParameter) {
	MyHookKey_Parameter param = *(MyHookKey_Parameter*)lpParameter;
	free(lpParameter); lpParameter = NULL;

	*param.hookaddr = SetHook();
	*param.hooksuccess = *param.hookaddr ? 1 : 0;

	if (!*param.hooksuccess) {
		return 1;
	}
	MessageLoop(param.sharedobject);
	return 0;
}

DllExport void InstallHook(int* hooksuccess, HANDLE* hookaddr, HANDLE* sharedobj_addr) {
	HANDLE hthread;

	MyHookKey_Parameter* param = malloc(sizeof(MyHookKey_Parameter));
	if (!param) {
		*hooksuccess = 0;
		return;
	}
	param->hookaddr = hookaddr;
	param->hooksuccess = hooksuccess;
	param->sharedobject = CreateMutex(NULL, 1, NULL);
	HANDLE hsobj = param->sharedobject;
	if (!param->sharedobject) {
		*hooksuccess = 0;
		free(param);
		return;
	}
	hthread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MyHookKey, param, NULL, NULL);

	if (!hthread) {
		*hooksuccess = 0;
		CloseHandle(param->sharedobject);
		free(param);
		return;
	}
	else {
		*hooksuccess = 1;
		*hookaddr = NULL;
		while (*hooksuccess && !*hookaddr);
		*sharedobj_addr = hsobj;
		CloseHandle(hthread);
		return;
	}
}

#ifdef __cplusplus
}
#endif
