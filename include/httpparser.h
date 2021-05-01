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

// 此函数从头查找第一个符合的子字符串。有两种工作模式：流模式和固定字符串模式。
// 如果工作在流模式下，函数返回时，如果 call_time 不是 NULL，字符生成器调用成功的次数存储在参数 call_time 指向的变量中。
// 如果工作在固定字符串模式下，如果 call_time 不是 NULL，那么函数运行时可能会修改参数 call_time 指向的变量的值，但写入 call_time 的值没有意义。
// 返回值：
// -1 ：没找到
// -2 : 动态内存分配失败
// -3 : 字符生成器调用失败
// >=0 : 找到了。没有错误发生。流模式下返回值是字符生成器被调用的次数；固定字符串模式下返回值是找到的子字符串的最后一个字符的后一个下标（不管是否超出数组界限）
int find_sub_str(size_t max_call_time, GENERATOR_FUNCTION_TYPE* generator, GENERATOR_PARAM_TYPE* generator_param_p, const char* str, const char* pattern, size_t* call_time, char* generated_buf, size_t generated_buf_len

#ifdef CASE_INSENSITIVE_STRSTR
	, int case_sensitive
#endif // CASE_INSENSITIVE_STRSTR

);

#ifdef CASE_INSENSITIVE_STRSTR
// 从流中取出下一个 HTTP 报文，不合法的数据也会被从流中取出，但不合法的数据会被丢弃。提取出的报文以字符串的形式存放于 message_pp 指向的指针指向的一块内存中。
// 返回值：
// >= 0 : HTTP 报文的长度（不包括结尾的空字符）
// -1 : 参数不合法，method_p 是空指针或 message_pp 是空指针
// -2 : find_sub_str() 动态内存分配失败
// -3 : 字符生成器 generator 调用失败
// -4 : 已经检测到合法报文，但尝试开辟一块内存来存放报文时发生了动态内存分配失败
// 其他负数 : find_sub_str() 返回此错误码
int next_http_message(HttpMethod* method_p, char** message_pp, GENERATOR_FUNCTION_TYPE* generator, GENERATOR_PARAM_TYPE* generator_param_p);
#endif // CASE_INSENSITIVE_STRSTR

#endif // !HTTPPARSER