#include "tcpserver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <logme.h>
#include <vutils.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

static int init_winsock() {
	WSADATA wsaData;

	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		LogMe.et("WSAStartup failed: %d", iResult);
	}
	else
	{
		LogMe.it("WSAStartup succeeded");
	}
	return iResult;
}

void tcp_server_run(int port) {
	const char* exit_words = "tcp server exited";
	const char* hello_words = "tcp server started";

	if (init_winsock() != 0) {
		LogMe.et(exit_words);
		return;
	}
	LogMe.it(hello_words);

	struct addrinfo* result = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_UNSPEC; // Both IPv4 and IPv6
	hints.ai_socktype = SOCK_STREAM; // Stream socket
	hints.ai_protocol = IPPROTO_TCP; // Socket over TCP
	hints.ai_flags = AI_PASSIVE;  // Listen socket

	// 获取 socket 指示信息列表
	int gres = getaddrinfo(
		NULL, // Listen socket
		vitoa(port, (char[30]){0}, 30), // Port
		&hints,
		&result
	);

	if (gres != 0) {
		LogMe.et("getaddrinfo failed: %d", gres);
		WSACleanup();
		LogMe.et(exit_words);
		return;
	}
	
	// 根据获取到的指示信息创建 监听套接字
	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		LogMe.et("Error at socket(): %ld", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		LogMe.et(exit_words);
		return;
	}
	LogMe.it("%p", ListenSocket);
}