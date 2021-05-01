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
#endif // LOGME_WINDOWS

#define MAX_HTTP_HEADERS_LENGTH 28672

typedef enum HttpMethod {
	GET = 0, POST, HEAD, PUT, DELETE_, CONNECT, OPTIONS, TRACE, PATCH, INVALID_METHOD
} HttpMethod;

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
// ����ֵ��
// >= 0 : HTTP ���ĵĳ��ȣ���������β�Ŀ��ַ���
// -1 : �������Ϸ���method_p �ǿ�ָ��� message_pp �ǿ�ָ��
// -2 : find_sub_str() ��̬�ڴ����ʧ��
// -3 : �ַ������� generator ����ʧ��
// -4 : �Ѿ���⵽�Ϸ����ģ������Կ���һ���ڴ�����ű���ʱ�����˶�̬�ڴ����ʧ��
// �������� : find_sub_str() ���ش˴�����
int next_http_message(HttpMethod* method_p, char** message_pp, GENERATOR_FUNCTION_TYPE* generator, GENERATOR_PARAM_TYPE* generator_param_p);
#endif // CASE_INSENSITIVE_STRSTR

#endif // !HTTPPARSER