#include "tcpserver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <logme.h>
#include <winsock2.h>

static int init_winsock() {
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		LogMe.e("WSAStartup failed: %d", iResult);
	}
	else
	{
		LogMe.i("WSAStartup succeeded");
	}
	return iResult;
}

void tcp_server_run() {
	const char* exit_words = "tcp server exited";
	const char* hello_words = "tcp server started";
	if (init_winsock() != 0) {
		LogMe.et(exit_words);
		return;
	}
	LogMe.it(hello_words);
}