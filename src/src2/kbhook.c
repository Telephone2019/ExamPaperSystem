#include "kbhook.h"

#include <windows.h>

#define KBHDLL "KeyBoardDll"

static HHOOK global_keyboard_hook = NULL;

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
DllExport LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

	if (nCode < 0 || nCode == HC_NOREMOVE) {
		// 如果代码小于零，则挂钩过程必须将消息传递给CallNextHookEx函数，而无需进一步处理，并且应返回CallNextHookEx返回的值。(来自官网手册)
		return CallNextHookEx(global_keyboard_hook, nCode, wParam, lParam);
	}
	if (lParam & 0x40000000) {
		// 【第30位的含义】键状态。如果在发送消息之前按下了键，则值为1。如果键被释放，则为0。(来自官网手册)
		// 我们只考虑被按下后松开的状态
		if ((wParam == VK_F5) ||
			(wParam == VK_F4) ||
			(wParam == VK_NUMPAD0)){
			InstallHook();
		}
	
		// Simulate a key press
		//keybd_event(VK_NUMPAD1,0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);

		// Simulate a key release
		//keybd_event(VK_NUMPAD1,0x45,KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,0);

		return CallNextHookEx(global_keyboard_hook, nCode, wParam, lParam);
	}

	return CallNextHookEx(global_keyboard_hook, nCode, wParam, lParam);
}

DllExport int InstallHook() {
	// 【参数1】钩子的类型，这里代表键盘钩子
	// 【参数2】钩子处理的函数
	// 【参数3】获取模块,KBDLL为DLL的项目名称
	// 【参数4】线程的ID，如果是全局钩子的话，这里要填0，如果是某个线程的钩子，那就需要写线程的ID
	
	HOOKPROC hkprcSysMsg;
	static HINSTANCE hinstDLL;
	hinstDLL = LoadLibrary(TEXT("D:\\My_Program\\ExamPaperSystem\\out\\build\\x64-Debug (默认值)\\KBHook.dll"));
	hkprcSysMsg = (HOOKPROC)GetProcAddress(hinstDLL, "LLKeyboardProc");
	global_keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, hkprcSysMsg, hinstDLL, 0);
	//global_keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, GetModuleHandle(KBHDLL), 0);
	if (global_keyboard_hook == NULL) {
		
		return 0;
	}
	return 1;


	/*
	HOOKPROC hkprcSysMsg;
	static HINSTANCE hinstDLL;
	static HHOOK hhookSysMsg;

	hinstDLL = LoadLibrary(TEXT("c:\\myapp\\sysmsg.dll"));
	hkprcSysMsg = (HOOKPROC)GetProcAddress(hinstDLL, "SysMessageProc");

	hhookSysMsg = SetWindowsHookEx(
		WH_SYSMSGFILTER,
		hkprcSysMsg,
		hinstDLL,
		0);
	*/


}

DllExport int UninstallHook() {
	if (! UnhookWindowsHookEx(global_keyboard_hook)) {
		return 0;
	}
	return 1;
}

static void KeyBoardHook_Main() {
	system("chcp");
}

