#ifndef TCPSERVER
#define TCPSERVER

#ifdef __cplusplus
extern "C" {
#endif

#include "httpparser.h"
#include "vlist.h"

#define MIME_TYPE_HTML "text/html"
#define MIME_TYPE_BIN "application/octet-stream"
#define MIME_TYPE_JPEG "image/jpeg"
#define MIME_TYPE_PDF "application/pdf"
#define MIME_TYPE_PLAIN_TEXT "text/plain"

#define HTTP_CHARSET_UTF8 "utf-8"

typedef struct tcp_node tcp_node;
typedef struct file_handle file_handle;

typedef struct HttpHandlerPac {
	void* extra;
	tcp_node* node;
} HttpHandlerPac;

// return value:
// ==INT_MAX : active shutdown
// >0 && != INT_MAX : no shutdown
// ==0 : recv 0 shutdown
// <0 : error shutdown, in this case, the return value can be used as error code
typedef int HTTP_HANDLE_FUNC_TYPE(HttpMessage* hmsg, HttpHandlerPac* pac);

// please notice that: URLs are case-sensitive.
typedef struct HttpHandler {
	VLISTNODE
	const char* path_contains;
	HTTP_HANDLE_FUNC_TYPE* handle_func;
	void* extra;
} HttpHandler;

void tcp_server_run(int port, int memmory_lack, vlist http_handlers
, const char* phrase_200
, const char* html_200
, const char* phrase_400
, const char* html_400
, const char* phrase_500
, const char* html_500
);

// 此函数会将 node 结构体中的 socket 设置为阻塞模式，并设置读取超时时间为 node 结构体中的相应字段，然后调用 recv() 并返回 recv() 的返回值
// 此函数的日志输出是完备的
int recv_t(tcp_node* np, char* buf, int len, int flags);

// 此函数会将 node 结构体中的 socket 设置为阻塞模式，并设置发送超时时间为 node 结构体中的相应字段，然后调用 send() 并返回 send() 的返回值
// 调用 send() 的 len 参数和返回值会被打印到日志中
// 如果 send() 返回错误，错误信息会被打印到日志中
int send_t(tcp_node* np, const char* buf, int len, int flags);

// 此函数调用 send_t() 向客户端回复文本请求。
// keep_alive 参数仅仅用于生成回复报文字段，实际断开连接需要调用者手动进行。
// 返回值：
// 0 : 回复成功
// non-zero : 回复失败，不应再进行更多的 socket 操作，应尽快断开并清理连接
// 此函数的日志输出是完备的。若失败，查看日志以获取详细信息。
int send_text(tcp_node* np, int status_code, const char* reason_phrase, int keep_alive, const char* text_body_str_could_be_NULL, const char* MIME_type, const char* text_body_charset, int is_download, const char* download_filename);

// 此函数调用 send_t() 和 transmit_file() 向客户端回复文件请求：
// 如果打开文件句柄成功且成功获取文件大小，那么传输文件；
// 如果打开文件句柄成功但获取文件大小失败，回复 500 页面；
// 如果打开文件句柄失败，回复 404 页面。
// 若失败，查看日志以获取详细信息。
// keep_alive 参数仅仅用于生成回复报文字段，实际断开连接需要调用者手动进行。
// 返回值：（大于等于 0 表示没有连接错误发生，小于 0 表示发生了连接错误）
// 0 : 文件传输成功
// 1 : 回复 404 成功
// 2 : 回复 500 成功
// -1 : 回复或传输文件时失败，不应再进行更多的 socket 操作，应尽快断开并清理连接
// 此函数的日志输出是完备的。
int send_file(tcp_node* np, const char* filename, int keep_alive, const char* MIME_type, const char* file_charset, int is_download, const char* download_filename
, const char *phrase_200
, const char *html_200
, const char *phrase_404
, const char *html_404
, const char *phrase_500
, const char *html_500
);

// 此函数调用 recv_t() 接收数据并将数据转储到指定的本地文件中：
// 如果出现任意错误，打印错误日志，回复 500 页面；
// 如果未发生错误，转储数据到文件完毕后回复 200 页面。
// 若失败，查看日志以获取详细信息。
// keep_alive 参数仅仅用于生成回复报文字段，实际断开连接需要调用者手动进行。
// 返回值：（大于等于 0 表示没有连接错误发生，小于 0 表示发生了连接错误）
// 0 : 转储成功
// 1 : 回复 500 成功
// -1 : 回复或接收数据时失败，不应再进行更多的 socket 操作，应尽快断开并清理连接
// 此函数的日志输出是完备的。
int receive_file(tcp_node* np, const char* file_dir, const char* filename, int keep_alive, long long file_size
, const char* phrase_200
, const char* html_200
, const char* phrase_500
, const char* html_500
);

#ifdef __cplusplus
}
#endif

#endif // TCPSERVER