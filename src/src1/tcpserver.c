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
#include <stdlib.h>

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
	DWORD tid;
	SOCKET socket;
	int open;
} node;

typedef struct params {
	node* node_p;
} params;

static vlist connections_list = NULL;
static int has_open;

// return non-zero to break
static int check_cth(vlist this, long i) {
	has_open += ((node*)(this->get_const(this, i)))->open;
	return 0;
}

static int all_closed(){
	has_open = 0;
	if (connections_list != NULL)
	{
		connections_list->foreach(connections_list, check_cth);
	}
	return !has_open;
}

static int clean_up_connection(node *connection_p, params *params_p, int returned) {
	if (closesocket(connection_p->socket) != 0) {
		LogMe.et("closesocket( %p ) [tid = %lu ] failed with error: %d", connection_p->socket, connection_p->tid, WSAGetLastError());
	}
	if (!CloseHandle(connection_p->handle))
	{
		LogMe.et("CloseHandle( %p ) [tid = %lu ] failed with error: %lu", connection_p->handle, connection_p->tid, GetLastError());
	}
	LogMe.nt("Connection thread [tid = %lu ] exit.", connection_p->tid);
	connection_p->open = 0;
	free(params_p);
	return returned;
}

static char16_t *get_utf_16_file_name_from_utf8(const char* u8fn) {
	int wnum = test_wide_char_num_of_utf8_including_wide_null(u8fn);
	char16_t* w_fn = malloc(sizeof(char16_t) * wnum);
	if (w_fn == NULL)
	{
		return NULL;
	}
	utf8_to_utf16_must_have_sufficient_buffer_including_wide_null(u8fn, w_fn, wnum);
	return w_fn;
}

static HANDLE get_file_hd(const char *filename) {
	const char* prefix = "\\\\?\\";
	const char* suffix = "";
	char* fn_temp = zero_malloc(strlen(filename) + strlen(prefix) + strlen(suffix) + 1);
	if (fn_temp == NULL)
	{
		LogMe.et("Open file [ %s ] failed with error: Malloc Fail", filename);
		return NULL;
	}
	strcat(fn_temp, prefix);
	strcat(fn_temp, filename);
	strcat(fn_temp, suffix);
	char16_t* wide_filename = get_utf_16_file_name_from_utf8(fn_temp);
	free(fn_temp); fn_temp = NULL;
	if (wide_filename == NULL)
	{
		LogMe.et("Open file [ %s ] failed with error: Malloc Fail", filename);
		return NULL;
	}
	HANDLE res = CreateFileW(
		wide_filename,
		GENERIC_READ,			// 只读
		FILE_SHARE_READ,		// 可以和其它进程一起读（读共享）
		NULL,					// lpSecurityAttributes 参数的默认值
		OPEN_EXISTING,			// 只有存在时才打开，否则失败
		FILE_ATTRIBUTE_NORMAL,	// 普通文件
		NULL					// hTemplateFile 参数的默认值
	);
	free(wide_filename); wide_filename = NULL;
	if (res == INVALID_HANDLE_VALUE)
	{
		DWORD last_err = GetLastError();
		LogMe.et("Open file [ %s ] failed with error: %lu", filename, last_err);
		return NULL;
	}
	else
	{
		return res;
	}
}

static DWORD WINAPI connection_run(_In_ LPVOID params_p) {
	node* connection_p = (*((params*)params_p)).node_p;
	return clean_up_connection(connection_p, params_p, 0);
}

// return zero to remove current node from vlist
static int closed_cnt_filter(vlist this, long i) {
	return ((node*)(this->get_const(this, i)))->open;
}

static void print_addrinfo_list(struct addrinfo *result) {
	int num = 0;
	for (;result != NULL; result = result->ai_next, num++)
	{
		char host_name[100];
		char port_str[100];
		getnameinfo(
			result->ai_addr, result->ai_addrlen, host_name, sizeof(host_name), port_str, sizeof(port_str), NI_NUMERICHOST | NI_NUMERICSERV
		);
		LogMe.it("addrinfo%d ip: [ %s ] port %s", num, host_name, port_str);
	}
}

void tcp_server_run(int port, int memmory_lack) {
	const char* exit_words = "tcp server exited";
	const char* hello_words = "tcp server started";
	const long max_cnt_list_size = memmory_lack ? 6 : 2000;

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
		&hints, // Hints
		&result // Results
	);

	if (iResult != 0) {
		LogMe.et("getaddrinfo failed: %d", iResult);
		WSACleanup();
		LogMe.et(exit_words);
		return;
	}

	// 打印指示信息列表
	print_addrinfo_list(result);
	
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
	connections_list = make_vlist(sizeof(node));
	if (connections_list == NULL) {
		LogMe.et("Unable to init connections_list");
		closesocket(ListenSocket);
		WSACleanup();
		LogMe.et(exit_words);
		return;
	}

	SOCKET ClientSocket;
	struct sockaddr_storage client_sockaddr;
	int client_sockaddr_len;
	enum reasons { AcceptFail = 0, MallocFail, CreateThreadFail, Debug } reason;
	SOCKET fail_socket = INVALID_SOCKET;

	// 接受新的 TCP 连接
	while (
		client_sockaddr_len = sizeof(client_sockaddr),
		ClientSocket = accept(ListenSocket, &client_sockaddr, &client_sockaddr_len),
		reason = (ClientSocket != INVALID_SOCKET)
		) {
		LogMe.it("accepted client socket: %p", ClientSocket);
		node* np = malloc(sizeof(node));
		params* pp = malloc(sizeof(params));
		if (np == NULL || pp == NULL)
		{
			fail_socket = ClientSocket;
			closesocket(ClientSocket); ClientSocket = INVALID_SOCKET;
			free(np); np = NULL;
			free(pp); pp = NULL;
			reason = MallocFail;
			break;
		}
		np->handle = CreateThread(
			NULL, // 默认安全属性
			0, // 使用默认的栈初始物理内存大小（commit size）
			connection_run, // 指向线程起始函数的指针
			pp, // 要向线程起始函数传递的指针（此指针作为参数）
			CREATE_SUSPENDED, // 创建线程，但不马上运行
			&(np->tid) // 存放创建的线程的 ID 的地址
		);
		if (np->handle == NULL)
		{
			fail_socket = ClientSocket;
			closesocket(ClientSocket); ClientSocket = INVALID_SOCKET;
			free(np); np = NULL;
			free(pp); pp = NULL;
			reason = CreateThreadFail;
			break;
		}
		np->socket = ClientSocket;
		np->open = 1;
		pp->node_p = np;
		connections_list->quick_add(connections_list, np);
		LogMe.wt("Connection thread [tid = %lu ] [clinet socket = %p ] start.", np->tid, np->socket);
		ResumeThread(np->handle);

		LogMe.bt("connections_list Size: %ld", connections_list->size);

		if (connections_list->size > max_cnt_list_size)
		{
			LogMe.et("Removed %ld closed connections from connections_list", connections_list->flush(connections_list, closed_cnt_filter));
		}

		/* Debug Code */
		/*fail_socket = ClientSocket;
		reason = Debug;
		break;*/
	}

	// 解释退出原因
	switch (reason)
	{
	case AcceptFail:
		LogMe.et("Accept failed wiht error: %d", WSAGetLastError());
		break;
	case MallocFail:
		LogMe.et("[on accept socket %p ] Malloc failed", fail_socket);
		break;
	case CreateThreadFail:
		LogMe.et("[on accept socket %p ] CreateThread failed", fail_socket);
		break;
	case Debug:
		LogMe.et("[on accept socket %p ] Debug", fail_socket);
		break;
	default:
		LogMe.et("[on accept socket %p ] Unknown Error", fail_socket);
		break;
	}

	// 等待所有的连接线程都自动退出
	LogMe.et("waiting for all the connection threads to exit...");
	while (!all_closed());
	LogMe.et("All the connection threads have exited");

	// 释放线程列表
	delete_vlist(connections_list, &connections_list);

	closesocket(ListenSocket);
	WSACleanup();
	LogMe.et(exit_words);
}