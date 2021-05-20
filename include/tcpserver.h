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

// �˺����Ὣ node �ṹ���е� socket ����Ϊ����ģʽ�������ö�ȡ��ʱʱ��Ϊ node �ṹ���е���Ӧ�ֶΣ�Ȼ����� recv() ������ recv() �ķ���ֵ
// �˺�������־������걸��
int recv_t(tcp_node* np, char* buf, int len, int flags);

// �˺����Ὣ node �ṹ���е� socket ����Ϊ����ģʽ�������÷��ͳ�ʱʱ��Ϊ node �ṹ���е���Ӧ�ֶΣ�Ȼ����� send() ������ send() �ķ���ֵ
// ���� send() �� len �����ͷ���ֵ�ᱻ��ӡ����־��
// ��� send() ���ش��󣬴�����Ϣ�ᱻ��ӡ����־��
int send_t(tcp_node* np, const char* buf, int len, int flags);

// �˺������� send_t() ��ͻ��˻ظ��ı�����
// keep_alive ���������������ɻظ������ֶΣ�ʵ�ʶϿ�������Ҫ�������ֶ����С�
// ����ֵ��
// 0 : �ظ��ɹ�
// non-zero : �ظ�ʧ�ܣ���Ӧ�ٽ��и���� socket ������Ӧ����Ͽ�����������
// �˺�������־������걸�ġ���ʧ�ܣ��鿴��־�Ի�ȡ��ϸ��Ϣ��
int send_text(tcp_node* np, int status_code, const char* reason_phrase, int keep_alive, const char* text_body_str_could_be_NULL, const char* MIME_type, const char* text_body_charset, int is_download, const char* download_filename);

// �˺������� send_t() �� transmit_file() ��ͻ��˻ظ��ļ�����
// ������ļ�����ɹ��ҳɹ���ȡ�ļ���С����ô�����ļ���
// ������ļ�����ɹ�����ȡ�ļ���Сʧ�ܣ��ظ� 500 ҳ�棻
// ������ļ����ʧ�ܣ��ظ� 404 ҳ�档
// ��ʧ�ܣ��鿴��־�Ի�ȡ��ϸ��Ϣ��
// keep_alive ���������������ɻظ������ֶΣ�ʵ�ʶϿ�������Ҫ�������ֶ����С�
// ����ֵ�������ڵ��� 0 ��ʾû�����Ӵ�������С�� 0 ��ʾ���������Ӵ���
// 0 : �ļ�����ɹ�
// 1 : �ظ� 404 �ɹ�
// 2 : �ظ� 500 �ɹ�
// -1 : �ظ������ļ�ʱʧ�ܣ���Ӧ�ٽ��и���� socket ������Ӧ����Ͽ�����������
// �˺�������־������걸�ġ�
int send_file(tcp_node* np, const char* filename, int keep_alive, const char* MIME_type, const char* file_charset, int is_download, const char* download_filename
, const char *phrase_200
, const char *html_200
, const char *phrase_404
, const char *html_404
, const char *phrase_500
, const char *html_500
);

// �˺������� recv_t() �������ݲ�������ת����ָ���ı����ļ��У�
// �������������󣬴�ӡ������־���ظ� 500 ҳ�棻
// ���δ��������ת�����ݵ��ļ���Ϻ�ظ� 200 ҳ�档
// ��ʧ�ܣ��鿴��־�Ի�ȡ��ϸ��Ϣ��
// keep_alive ���������������ɻظ������ֶΣ�ʵ�ʶϿ�������Ҫ�������ֶ����С�
// ����ֵ�������ڵ��� 0 ��ʾû�����Ӵ�������С�� 0 ��ʾ���������Ӵ���
// 0 : ת���ɹ�
// 1 : �ظ� 500 �ɹ�
// -1 : �ظ����������ʱʧ�ܣ���Ӧ�ٽ��и���� socket ������Ӧ����Ͽ�����������
// �˺�������־������걸�ġ�
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