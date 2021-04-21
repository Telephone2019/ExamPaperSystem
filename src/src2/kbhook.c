#include "kbhook.h"

#include <stdio.h>

#ifndef WIN32_LEAN_AND_MEAN

#include <windows.h>

#define WIN32_LEAN_AND_MEAN
#endif

#define KBHDLL "KeyBoardDll"

HHOOK global_keyboard_hook = NULL;

int kbhook_run_success() {
	return 1;
}


/// <summary>
/// 键盘回调
/// </summary>
/// <param name="nnCode"></param>
/// <param name="wParam">虚拟按键的代号</param>
/// <param name="lParam">键状态</param>
/// <returns></returns>
LRESULT WINAPI KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

	if (nCode < 0 || nCode == HC_NOREMOVE) {
		// 如果代码小于零，则挂钩过程必须将消息传递给CallNextHookEx函数，而无需进一步处理，并且应返回CallNextHookEx返回的值。此参数可以是下列值之一。(来自官网手册)
		return CallNextHookEx(global_keyboard_hook, nCode, wParam, lParam);
	}
	if (lParam & 0x40000000) {
		// 【第30位的含义】键状态。如果在发送消息之前按下了键，则值为1。如果键被释放，则为0。(来自官网手册)
		// 我们只考虑被按下后松开的状态
		return CallNextHookEx(global_keyboard_hook, nCode, wParam, lParam);
	}

	return CallNextHookEx(global_keyboard_hook, nCode, wParam, lParam);
}

int InstallHook() {
	// 【参数1】钩子的类型，这里代表键盘钩子
	// 【参数2】钩子处理的函数
	// 【参数3】获取模块,PROJECT_NAME为DLL的项目名称
	// 【参数4】线程的ID，如果是全局钩子的话，这里要填0，如果是某个线程的钩子，那就需要写线程的ID
	

	global_keyboard_hook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, GetModuleHandle(KBHDLL), 0);
	if (global_keyboard_hook == NULL) {
		
		return 0;
	}
	return 1;

}

int UninstallHook() {
	if (! UnhookWindowsHookEx(global_keyboard_hook)) {
		return 0;
	}
	return 1;
}

void KeyBoardHook_Main() {

}

