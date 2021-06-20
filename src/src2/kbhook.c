#ifdef __cplusplus
extern "C" {
#endif
#include "kbhook.h"

DllExport LRESULT CALLBACK KBHOOK_KeyboardProc_______________(int nCode, WPARAM wParam, LPARAM lParam) {
	/* from offical document */
	if (nCode < 0 || nCode != HC_ACTION) { // do not process message
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
	/* from offical document */

	int _EatKeystroke = 0;
	KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
	switch (wParam)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		// the most significant bit indicates whether the key is currently up or down
		int bCtrlKeyDown =
			GetAsyncKeyState(VK_CONTROL) >> ((sizeof(SHORT) * 8) - 1);
		_EatKeystroke = (
			(p->vkCode == VK_LWIN)
			|| (p->vkCode == VK_RWIN)
			|| (p->flags & LLKHF_ALTDOWN)
			|| (p->vkCode == VK_ESCAPE && p->flags & LLKHF_ALTDOWN)
			|| (p->vkCode == VK_ESCAPE && bCtrlKeyDown) // ctrl + esc
			);
		break;
	default:
		break;
	}

	if (_EatKeystroke) {
		/* from offical document */
		return 1;
		/* from offical document */
	}
	else return CallNextHookEx(NULL, nCode, wParam, lParam);
}

DllExport LRESULT CALLBACK KBHOOK_MouseProc_______________(int nCode, WPARAM wParam, LPARAM lParam) {
	/* from offical document */
	if (nCode < 0 || nCode != HC_ACTION) { // do not process message
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
	/* from offical document */

	MSLLHOOKSTRUCT* p = (MSLLHOOKSTRUCT*)lParam;
	switch (wParam)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_MOUSEWHEEL:

		break;
	case WM_MOUSEHWHEEL:
	default:
		break;
	}

	CallNextHookEx(NULL, nCode, wParam, lParam);
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
	HOOKPROC hkprcSysMsg = (HOOKPROC)GetProcAddress(hinstDLL, "KBHOOK_KeyboardProc_______________");
	HHOOK hhookSysMsg = SetWindowsHookEx(WH_KEYBOARD_LL, hkprcSysMsg, hinstDLL, 0);
	if (hhookSysMsg == NULL) {
		CloseHandle(hinstDLL);
	}
	return hhookSysMsg;
}

typedef struct HookThreadRoutine_Parameter {
	HANDLE* hookaddr;
	HANDLE sharedobject;
	int* hooksuccess;
} HookThreadRoutine_Parameter;

static DWORD WINAPI HookThreadRoutine(_In_ LPVOID lpParameter) {
	HookThreadRoutine_Parameter param = *(HookThreadRoutine_Parameter*)lpParameter;
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

	HookThreadRoutine_Parameter* param = malloc(sizeof(HookThreadRoutine_Parameter));
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
	hthread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)HookThreadRoutine, param, NULL, NULL);

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
