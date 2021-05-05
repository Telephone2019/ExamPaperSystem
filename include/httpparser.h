#ifndef HTTPPARSER
#define HTTPPARSER

#include <stddef.h>

#include <macros.h>
#ifdef LOGME_WINDOWS
#define CASE_INSENSITIVE_STRSTR
#define CASE_INSENSITIVE_STRSTR_WINDOWS
#elif defined(LOGME_GCC)
#define CASE_INSENSITIVE_STRSTR
#define CASE_INSENSITIVE_STRSTR_GCC
#else 
#define CASE_INSENSITIVE_STRSTR
#define CASE_INSENSITIVE_STRSTR_VUTILS
#endif // LOGME_WINDOWS

#ifdef LOGME_MSVC
#define CASE_INSENSITIVE_STRCMP
#define CASE_INSENSITIVE_STRCMP_MSVC
#elif defined(LOGME_GCC)
#define CASE_INSENSITIVE_STRCMP
#define CASE_INSENSITIVE_STRCMP_GCC
#else 
#define CASE_INSENSITIVE_STRCMP
#define CASE_INSENSITIVE_STRCMP_VUTILS
#endif // LOGME_MSVC

#include <vlist.h>

#define MAX_HTTP_HEADERS_LENGTH 28672

typedef enum HttpMethod {
	GET = 0, POST, HEAD, PUT, DELETE_, CONNECT, OPTIONS, TRACE, PATCH, INVALID_METHOD
} HttpMethod;
HttpMethod httpMethodFromStr(const char *method_name);
const char* getConstHttpMethodNameStr(HttpMethod http_method);

typedef char(GENERATOR_FUNCTION_TYPE)(void*, int*);

typedef void GENERATOR_PARAM_TYPE;

// �˺�����ͷ���ҵ�һ�����ϵ����ַ����������ֹ���ģʽ����ģʽ�͹̶��ַ���ģʽ��
// �����������ģʽ�£���������ʱ����� call_time ���� NULL���ַ����������óɹ��Ĵ����洢�ڲ��� call_time ָ��ı����С�
// ��������ڹ̶��ַ���ģʽ�£���� call_time ���� NULL����ô��������ʱ���ܻ��޸Ĳ��� call_time ָ��ı�����ֵ����д�� call_time ��ֵû�����塣
// ����ֵ��
// -1 ��û�ҵ�
// -2 : ��̬�ڴ����ʧ��
// -3 : �ַ�����������ʧ��
// >=0 : �ҵ��ˡ�û�д���������ģʽ�·���ֵ���ַ������������õĴ������̶��ַ���ģʽ�·���ֵ���ҵ������ַ��������һ���ַ��ĺ�һ���±꣨�����Ƿ񳬳�������ޣ�
int find_sub_str(size_t max_call_time, GENERATOR_FUNCTION_TYPE* generator, GENERATOR_PARAM_TYPE* generator_param_p, const char* str, const char* pattern, size_t* call_time, char* generated_buf, size_t generated_buf_len

#ifdef CASE_INSENSITIVE_STRSTR
	, int case_sensitive
#endif // CASE_INSENSITIVE_STRSTR

);

#ifdef CASE_INSENSITIVE_STRSTR
// ������ȡ����һ�� HTTP ���ģ����Ϸ�������Ҳ�ᱻ������ȡ���������Ϸ������ݻᱻ��������ȡ���ı������ַ�������ʽ����� message_pp ָ���ָ��ָ���һ���ڴ��С�
// �����ȡ�ɹ��ˣ��������ʵ���ʱ���ͷ� message_pp ָ���ָ��ָ����ڴ棬���������ڴ�й©��
// ����ֵ��
// >= 0 : HTTP ���ĵĳ��ȣ���������β�Ŀ��ַ���
// -1 : �������Ϸ���method_p �ǿ�ָ��� message_pp �ǿ�ָ��
// -2 : find_sub_str() ��̬�ڴ����ʧ��
// -3 : �ַ������� generator ����ʧ��
// -4 : �Ѿ���⵽�Ϸ����ģ������Կ���һ���ڴ�����ű���ʱ�����˶�̬�ڴ����ʧ��
// �������� : find_sub_str() ���ش˴�����
int next_http_message(HttpMethod* method_p, char** message_pp, GENERATOR_FUNCTION_TYPE* generator, GENERATOR_PARAM_TYPE* generator_param_p);
#endif // CASE_INSENSITIVE_STRSTR

#ifdef CASE_INSENSITIVE_STRCMP
typedef struct HttpHeader {
	VLISTNODE
	char* field;
	char* value;
} HttpHeader, KeyValuePair;
void freeHttpHeader(HttpHeader* hh);
#define freeKeyValuePair(x) freeHttpHeader(x);
typedef struct HttpMessage {
	int malloc_success;
	int success;
	char* error_name;
	char* error_reason;
	HttpMethod method;
	int http_major;
	int http_minor;
	char* url;
	char* path;
	vlist query_string;
	vlist url_fragment;
	vlist http_headers;
} HttpMessage;
// ����ô˺�������ȡһ���ѳ�ʼ���� HttpMessage �ṹ��
HttpMessage makeHttpMessage();
// �� HttpMessage �ṹ�岻�ٱ�ʹ�ã�����ô˺������ͷ� HttpMessage �ṹ���ڵĶ�̬�ڴ�
void freeHttpMessage(HttpMessage* httpmsg);
// ����HTTP�����ַ������˺��������HTTP���ĵõ�һ�� HttpMessage �ṹ�壬Ȼ���䷵�ء�
// �Ӵ˺������غ�Ӧ���ȼ�鷵�ص� HttpMessage �ṹ���е� malloc_success �ֶΣ�������ֶ�Ϊ 0��˵���˽ṹ�����𻵣�
// �벻Ҫ��ʹ�ô˽ṹ�岢����ʹ�� freeHttpMessage() �ͷŴ˽ṹ�塣
// ��� HttpMessage �ṹ��δ�𻵣���ô success �ֶδ�������ɹ����0ʧ�ܣ�non-zero�ɹ������������ʧ�ܣ�
// ʧ�ܵ���Ϣ����ϸԭ��洢�� error_name �� error_reason ָ����ַ����С�
// ��Ҫ���ķ��صĽṹ���е��κ�ָ���ֶΣ������޸�ָ��ָ��ı�����ֵ���������޸�ָ�뱾�������������ڴ�й¶��
// �����ص� HttpMessage �ṹ�岻�ٱ�ʹ�ã������ freeHttpMessage() ���ͷ��������������ڴ�й©��
HttpMessage parse_http_message(const char* message);
#endif // CASE_INSENSITIVE_STRCMP


#endif // !HTTPPARSER