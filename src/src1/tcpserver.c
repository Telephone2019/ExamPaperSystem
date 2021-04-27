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

#define DEFAULT_RECV_TIMEOUT_S 15
#define DEFAULT_SEND_TIMEOUT_S 15

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
	long recv_timeout_s;
	long send_timeout_s;
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
	LogMe.nt("Connection thread [tid = %lu ] [client socket = %p ] exit.", connection_p->tid, connection_p->socket);
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

// 不想再收发数据时，请调用此函数来主动关闭连接。
// 调用此函数后无法再调用 recv() 或 send()。
// 调用此函数意味着退出线程。
static int active_shutdown(node *cnt_p, params *params_p, int returned) {
	LogMe.it("active_shutdown( %p )", cnt_p->socket);
	// 主动关闭连接意味着我们不想再收发数据，但关闭连接前应该保证我们先前想要发送的数据已被发送。
	// 等待本机发送缓冲区内的数据都发送完后，按照TCP协议，友善地主动发送 FIN 向对方表明我们想关闭连接。对方收到 FIN 后，我们的写资源会自动释放。
	if (shutdown(cnt_p->socket, SD_SEND) == SOCKET_ERROR) {
		// 如果出错了，说明对方已不可达，接下来释放写资源。由于不会再收到对方发送的数据且不再关心未处理的数据，接下来释放读资源。
		LogMe.et("actively shutdown( %p , SD_SEND ) [tid = %lu ] failed with error: %d", cnt_p->socket, cnt_p->tid, WSAGetLastError());
	}
	else
	{
		// 如果没出错，写资源已被释放。按照TCP协议，对方尚未发送 FIN，我们本应等待对方继续发送的数据和 FIN，但对方可能
		// 永远都不会发送 FIN（不管对方是否是出于恶意），而且我们也不再关心未处理的数据和对方继续发送的数据，因此接下来释放读资源。
		// 读资源释放后，对方会收到 “连接已重置” 的错误。
	}
	// 释放 读资源+写资源，释放 socket。
	return clean_up_connection(cnt_p, params_p, returned);
}

// 当 recv() 函数返回 0 时，对方表明不再向我们发送数据并想要关闭连接。请不要再调用 recv()，并尽快发送完需要发送的数据，然后调用此函数来关闭连接。
// 调用此函数后无法再调用 recv() 或 send()。
// 调用此函数意味着退出线程。
static int recv_0_shutdown(node* cnt_p, params* params_p, int returned) {
	LogMe.bt("recv_0_shutdown( %p )", cnt_p->socket);
	// 因为 recv() 函数表明我们接收到对方的 FIN，意味着对方不会再发送数据且我们也已经处理完对方发来的所有数据，所以释放读资源。
	if (shutdown(cnt_p->socket, SD_RECEIVE) == SOCKET_ERROR) {
		LogMe.et("recv_0 shutdown( %p , SD_RECEIVE ) [tid = %lu ] failed with error: %d", cnt_p->socket, cnt_p->tid, WSAGetLastError());
	}
	// 是时候关闭连接了，不要让对方久等。
	// 关闭连接前应该保证我们先前想要发送的数据已被发送。
	// 等待本机发送缓冲区内的数据都发送完后，按照TCP协议，友善地向对方发送 FIN，表明我们已经发送完需要发送的数据。对方收到 FIN 后，我们的写资源会自动释放。
	if (shutdown(cnt_p->socket, SD_SEND) == SOCKET_ERROR) {
		// 如果出错了，说明对方已不可达，接下来释放写资源。
		LogMe.et("recv_0 shutdown( %p , SD_SEND ) [tid = %lu ] failed with error: %d", cnt_p->socket, cnt_p->tid, WSAGetLastError());
	}
	else
	{
		// 如果没出错，写资源已被释放。
	}
	// 释放 读资源+写资源，释放 socket。
	return clean_up_connection(cnt_p, params_p, returned);
}

// send() 或 recv() 发生错误时，请不要再进行任何 socket 操作，立即调用此函数来关闭连接。
// 调用此函数后无法再调用 recv() 或 send()。
// 调用此函数意味着退出线程。
static int error_shutdown(node* cnt_p, params* params_p, int returned) {
	LogMe.et("error_shutdown( %p )", cnt_p->socket);
	// send() 或 recv() 发生错误意味着对方已不可达，无法进行任何更多的读写操作，并且我们也已经处理完对方发来的所有数据。
	// 此时直接释放 读资源+写资源。
	// 
	// 释放 读资源+写资源，释放 socket。
	return clean_up_connection(cnt_p, params_p, returned);
}

// 此函数会将 node 结构体中的 socket 设置为阻塞模式，并设置读取超时时间为 node 结构体中的相应字段，然后调用 recv() 并返回 recv() 的返回值
static int recv_t(node *np, char *buf, int len, int flags) {
	ioctlsocket(np->socket, FIONBIO, &((u_long) {0})); // 0:blocking 1:non-blocking
	setsockopt(np->socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&((DWORD) { ((DWORD)(np->recv_timeout_s))*1000 }), sizeof(DWORD));
	return recv(np->socket, buf, len, flags);
}

// 此函数会将 node 结构体中的 socket 设置为阻塞模式，并设置发送超时时间为 node 结构体中的相应字段，然后调用 send() 并返回 send() 的返回值
static int send_t(node *np, const char *buf, int len, int flags) {
	ioctlsocket(np->socket, FIONBIO, &((u_long) { 0 })); // 0:blocking 1:non-blocking
	setsockopt(np->socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&((DWORD) { ((DWORD)(np->send_timeout_s)) * 1000 }), sizeof(DWORD));
	return send(np->socket, buf, len, flags);
}

static DWORD WINAPI connection_run(_In_ LPVOID params_p) {
	node* np = (*((params*)params_p)).node_p;
	const char* body = "<html>\n<body>\n<h1>Hello, World!</h1>\n</body>\n</html>\n";
	char resp[1000];
	sprintf(resp, "HTTP/1.1 200 OK\r\nDate: Tue, 27 April 2021 22:55:30 GMT+8\r\nServer: Cone\r\nContent-Length: %d\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n\r\n%s", strlen(body), body);
	int send_res = send_t(np, resp, strlen(resp), 0);
	LogMe.bt("send_res = %d", send_res);
	for (int i = 1;i < 10000;i++) {
		char a;
		int recv_res = recv_t(np, &a, 1, 0);
		if (recv_res > 0)
		{
			putchar(a);
		}
		else if (recv_res == 0)
		{
			return recv_0_shutdown(np, params_p, 0);
		}
		else
		{
			LogMe.et("recv_t() on socket [ %p ] failed with error: %d", np->socket, WSAGetLastError());
			return error_shutdown(np, params_p, 1);
		}
	}
	putchar('\n');
	return active_shutdown(np, params_p, 0); // 主动关闭连接
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
	enum reasons { AcceptFail = 0, MallocFail, CreateThreadFail, SetSockOptFail, Debug } reason;
	SOCKET fail_socket = INVALID_SOCKET;

	// 接受新的 TCP 连接
	while (
		client_sockaddr_len = sizeof(client_sockaddr),
		ClientSocket = accept(ListenSocket, &client_sockaddr, &client_sockaddr_len),
		reason = (ClientSocket != INVALID_SOCKET)
		) {
		LogMe.it("accepted client socket: %p", ClientSocket);
		// 设置套接字为连接成功后调用 closesocket() 时立即释放读写资源、然后立即释放 socket 并返回，即 SO_DONTLINGER 设为 false
		iResult = setsockopt(ClientSocket, SOL_SOCKET, SO_DONTLINGER, (char*)&((DWORD) { 0 }), sizeof(DWORD));
		if (iResult == SOCKET_ERROR) {
			LogMe.et("setsockopt for client socket [ %p ] SO_DONTLINGER failed with error: %u", WSAGetLastError());
			fail_socket = ClientSocket;
			closesocket(ClientSocket); ClientSocket = INVALID_SOCKET;
			reason = SetSockOptFail;
			break;
		}
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
		np->recv_timeout_s = DEFAULT_RECV_TIMEOUT_S;
		np->send_timeout_s = DEFAULT_SEND_TIMEOUT_S;
		np->open = 1;
		pp->node_p = np;
		connections_list->quick_add(connections_list, np);
		LogMe.wt("Connection thread [tid = %lu ] [client socket = %p ] start.", np->tid, np->socket);
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