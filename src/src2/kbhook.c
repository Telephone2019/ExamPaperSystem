#include "kbhook.h"

#include <windows.h>

HOOKPROC hkprcSysMsg;
static HINSTANCE hinstDLL;
static HHOOK hhookSysMsg;

static void KeyBoardHook_Main();

DllExport int kbhook_run_success() {
	KeyBoardHook_Main();
	return 1;
}


/// <summary>
/// 键盘回调
/// </summary>
/// <param name="nnCode"></param>
/// <param name="wParam">虚拟按键的代号</param>
/// <param name="lParam">键状态</param>
/// <returns></returns>
/// 如果是使用WH_KEYBOARD, 要写在DLL中, 通过注入线程来调用, 这里要注意传入Hook线程的窗口句柄,用来Post一个消息通知Hook线程来处理消息
DllExport LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

	if (wParam == 'z' || wParam == VK_NUMPAD0 || GetKeyState(VK_LWIN) < 0) {
		return 1;
	}

	return CallNextHookEx(hhookSysMsg, nCode, wParam, lParam);
}


//如果是使用WH_KEYBOARD_LL, 直接写在Hook线程就可以了
DllExport LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

	//if()

	SetWindowsHookExA(WH_KEYBOARD, LLKeyboardProc, hinstDLL, 0);

	return CallNextHookEx(hhookSysMsg, nCode, wParam, lParam);
}




/*
您必须将全球挂钩程序置于与安装挂钩程序的应用程序分开的 DLL 中。安装应用程序必须具有 DLL 模块的句柄，然后才能安装挂钩程序。
*/
DllExport int InstallHook() {

	hinstDLL = LoadLibrary(TEXT("KBHook.dll"));
	if (hinstDLL) {
		hkprcSysMsg = (HOOKPROC)GetProcAddress(hinstDLL, "KeyboardProc");
	}
	if (hkprcSysMsg) {
		// 【参数1】钩子的类型，这里代表键盘钩子
		// 【参数2】钩子处理的函数
		// 【参数3】获取模块,DLL的项目名称
		// 【参数4】线程的ID，如果是全局钩子的话，这里要填0，如果是某个线程的钩子，那就需要写线程的ID
		hhookSysMsg = SetWindowsHookExA(WH_KEYBOARD, hkprcSysMsg, hinstDLL, 0);
	}
	if (hhookSysMsg) {
		return 1;
	}
	return 0;
}

//另一种方法
DllExport int InstallHook_s() {
	hhookSysMsg = SetWindowsHookExA(WH_KEYBOARD, hkprcSysMsg, hinstDLL, 0);
	if (hhookSysMsg != NULL) {
		return 1;
	}
	return 0;
}


DllExport int UninstallHook() {
	if (! UnhookWindowsHookEx(hhookSysMsg)) {
		return 0;
	}
	return 1;
}

static void KeyBoardHook_Main() {
	system("chcp");

	InstallHook();

	/*
	hinstDLL = LoadLibrary(TEXT("KBHook.dll"));
	hkprcSysMsg = (HOOKPROC)GetProcAddress(hinstDLL, "InstallHook_s");
	*/

	//循环消息队列
	MSG msg;
	while (GetMessageA(&msg,hhookSysMsg,0,0) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	/*
	while (GetMessageA(&msg,hkprcSysMsg,0,0) != 0)
	*/

	UninstallHook();

}

