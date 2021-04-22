#include "tcpserver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <logme.h>
#include <vutils.h>
#include <vlist.h>

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

typedef struct node {
	VLISTNODE
		HANDLE handle;
	int open;
} node;

static vlist child_threads_list = NULL;
static int has_open;

// return non-zero to break
static int check_cth(vlist this, long i) {
	has_open += ((node*)(this->get_const(this, i)))->open;
	return 0;
}

static int all_closed(){
	has_open = 0;
	if (child_threads_list != NULL)
	{
		child_threads_list->foreach(child_threads_list, check_cth);
	}
	return !has_open;
}

void tcp_server_run(int port) {
	const char* exit_words = "tcp server exited";
	const char* hello_words = "tcp server started";

	LogMe.it(hello_words);

	if (init_winsock() != 0) {
		LogMe.et(exit_words);
		return;
	}

	struct addrinfo* result = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET6; // IPv6 for dual-stack socket
	hints.ai_socktype = SOCK_STREAM; // Stream socket
	hints.ai_protocol = IPPROTO_TCP; // Socket over TCP
	hints.ai_flags = AI_PASSIVE;  // Listen socket

	// 获取 socket 指示信息列表
	int iResult = getaddrinfo(
		NULL, // Listen socket
		vitoa(port, (char[30]){0}, 30), // Port
		&hints,
		&result
	);

	if (iResult != 0) {
		LogMe.et("getaddrinfo failed: %d", iResult);
		WSACleanup();
		LogMe.et(exit_words);
		return;
	}
	
	// 根据获取到的第一个指示信息创建 监听套接字
	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		LogMe.et("Error at socket(): %ld", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		LogMe.et(exit_words);
		return;
	}

	// 设置套接字为 IPv4 IPv6 双协议栈套接字
	iResult = setsockopt(ListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&((DWORD) { 0 }), sizeof(DWORD));
	if (iResult == SOCKET_ERROR) {
		LogMe.et("setsockopt for IPV6_V6ONLY failed with error: %u", WSAGetLastError());
		closesocket(ListenSocket);
		freeaddrinfo(result);
		WSACleanup();
		LogMe.et(exit_words);
		return;
	}
	else {
		LogMe.it("Set IPV6_V6ONLY: false");
	}

	// 根据获取到的第一个指示信息绑定 监听套接字
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		LogMe.et("bind failed with error: %d", WSAGetLastError());
		closesocket(ListenSocket);
		freeaddrinfo(result);
		WSACleanup();
		LogMe.et(exit_words);
		return;
	}
	LogMe.it("bind succeeded on port: %d", port);

	// 释放不再需要的指示信息列表
	freeaddrinfo(result);

	// 监听 监听套接字
	// SOMAXCONN 是 “接收新 TCP 连接队列” 的系统默认的最大长度
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		LogMe.et("Listen failed with error: %ld", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		LogMe.et(exit_words);
		return;
	}
	LogMe.it("listen succeeded on port: %d", port);

	LogMe.it("listening...");

	// 初始化线程列表
	child_threads_list = make_vlist(sizeof(node));

	SOCKET ClientSocket;
	struct sockaddr_storage client_sockaddr;
	int client_sockaddr_len;
	int accept_succeed;

	// 接受新的 TCP 连接
	while (
		client_sockaddr_len = sizeof(client_sockaddr),
		ClientSocket = accept(ListenSocket, &client_sockaddr, &client_sockaddr_len),
		accept_succeed = (ClientSocket != INVALID_SOCKET)
		) {
		LogMe.it("accepted: ");
		accept_succeed = 0;
		break;
	}

	// 解释退出原因
	if (!accept_succeed)
	{
		LogMe.et("accept failed: %d", WSAGetLastError());
	}

	// 等待所有的连接线程都自动退出
	LogMe.et("waiting for all the connection threads to exit...");
	while (!all_closed());
	LogMe.et("All the connection threads have exited");

	// 释放线程列表
	delete_vlist(child_threads_list, &child_threads_list);

	closesocket(ListenSocket);
	WSACleanup();
	LogMe.et(exit_words);
}