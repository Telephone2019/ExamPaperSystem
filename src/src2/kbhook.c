#ifdef __cplusplus
extern "C" {
#endif
#include "kbhook.h"

#include "logme.h"

DllExport LRESULT CALLBACK KBHOOK_KeyboardProc_______________(int nCode, WPARAM wParam, LPARAM lParam) {
	/* from offical document */
	if (nCode < 0 || nCode != HC_ACTION) { // do not process message
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
	/* from offical document */

	int _EatKeystroke = 0;
	int ctrl_down = 0, alt_down = 0;
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
		ctrl_down = bCtrlKeyDown;
		alt_down = p->flags & LLKHF_ALTDOWN;
		_EatKeystroke = (
			(p->vkCode == VK_LWIN)
			|| (p->vkCode == VK_RWIN)
			|| (alt_down)
			|| (p->vkCode == VK_ESCAPE && alt_down)
			|| (p->vkCode == VK_ESCAPE && bCtrlKeyDown) // ctrl + esc
			);
		break;
	default:
		break;
	}

	if (_EatKeystroke) {
		LogMe.e("Key [ %lu%s%s ] Not Allowed!", p->vkCode, ctrl_down?" + Ctrl":"", alt_down?" + Alt":"");
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
		LogMe.i("L Button Down!");
		break;
	case WM_LBUTTONUP:
		LogMe.i("L Button Up!");
		break;
	case WM_RBUTTONDOWN:
		LogMe.i("R Button Down!");
		break;
	case WM_RBUTTONUP:
		LogMe.i("R Button Up!");
		break;
	case WM_MBUTTONDOWN:
		LogMe.i("M Button Down!");
		break;
	case WM_MBUTTONUP:
		LogMe.i("M Button Up!");
		break;
	case WM_MOUSEWHEEL:
		short high_word = HIWORD(p->mouseData);
		if (high_word > 0)
		{
			LogMe.i("Wheel scroll away!");
		}
		else if (high_word < 0)
		{
			LogMe.i("Wheel scroll toward!");
		}
		else
		{
			LogMe.e("Unknown wheel action");
		}
		break;
	case WM_MOUSEMOVE:
		LogMe.b("Move to (x,y): (%ld,%ld)", p->pt.x, p->pt.y);
		break;
	case WM_MOUSEHWHEEL:
	default:
		break;
	}

	CallNextHookEx(NULL, nCode, wParam, lParam);
}

DllExport void UninstallHook(HANDLE hook, HANDLE* hookaddr, HANDLE shareobj, HANDLE* shareobj_addr) {
	if (hook == NULL || shareobj == NULL) {
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
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
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

static const char* get_proc_name(HookType type) {
	switch (type)
	{
	default:
	case HT_KEYBOARD:
		return "KBHOOK_KeyboardProc_______________";
	case HT_MOUSE:
		return "KBHOOK_MouseProc_______________";
	}
}

static int get_hook_int(HookType type) {
	switch (type)
	{
	default:
	case HT_KEYBOARD:
		return WH_KEYBOARD_LL;
	case HT_MOUSE:
		return WH_MOUSE_LL;
	}
}

static HANDLE SetHook(HookType type) {
	HINSTANCE hinstDLL = LoadLibrary(TEXT("KBHook.dll"));
	if (!hinstDLL) {
		return NULL;
	}
	HOOKPROC hkprcSysMsg = (HOOKPROC)GetProcAddress(hinstDLL, get_proc_name(type));
	HHOOK hhookSysMsg = SetWindowsHookEx(get_hook_int(type), hkprcSysMsg, hinstDLL, 0);
	if (hhookSysMsg == NULL) {
		CloseHandle(hinstDLL);
	}
	return hhookSysMsg;
}

typedef struct HookThreadRoutine_Parameter {
	HANDLE* hookaddr;
	HANDLE sharedobject;
	int* hooksuccess;
	HookType hooktype;
} HookThreadRoutine_Parameter;

static DWORD WINAPI HookThreadRoutine(_In_ LPVOID lpParameter) {
	HookThreadRoutine_Parameter param = *(HookThreadRoutine_Parameter*)lpParameter;
	free(lpParameter); lpParameter = NULL;

	*param.hookaddr = SetHook(param.hooktype);
	*param.hooksuccess = *param.hookaddr ? 1 : 0;

	if (!*param.hooksuccess) {
		return 1;
	}
	MessageLoop(param.sharedobject);
	return 0;
}

DllExport void InstallHook(int* hooksuccess, HANDLE* hookaddr, HANDLE* sharedobj_addr, HookType type) {
	HANDLE hthread;

	HookThreadRoutine_Parameter* param = malloc(sizeof(HookThreadRoutine_Parameter));
	if (!param) {
		*hooksuccess = 0;
		*hookaddr = *sharedobj_addr = NULL;
		return;
	}
	param->hookaddr = hookaddr;
	param->hooksuccess = hooksuccess;
	param->sharedobject = CreateMutex(NULL, 1, NULL);
	param->hooktype = type;
	HANDLE hsobj = param->sharedobject;
	if (!param->sharedobject) {
		free(param);
		*hooksuccess = 0;
		*hookaddr = *sharedobj_addr = NULL;
		return;
	}
	hthread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)HookThreadRoutine, param, NULL, NULL);

	if (!hthread) {
		CloseHandle(param->sharedobject);
		free(param);
		*hooksuccess = 0;
		*hookaddr = *sharedobj_addr = NULL;
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
