#ifndef HTTPPARSER
#define HTTPPARSER

#include <stddef.h>

// 此函数从头查找第一个符合的子字符串。有两种工作模式：流模式和固定字符串模式。
// 返回值：
// -1 ：没找到
// -2 : 动态内存分配失败
// -3 : 字符生成器调用失败。字符生成器调用成功的次数存储在参数 call_time 指向的变量中
// >=0 : 找到了。没有错误发生。流模式下返回值是字符生成器被调用的次数；固定字符串模式下返回值是找到的子字符串的最后一个字符的后一个下标（不管是否超出数组界限）
int find_sub_str(size_t max_call_time, char(*generator)(void*, int*), void* generator_param_p, const char* str, const char* pattern, size_t* call_time);

#endif // !HTTPPARSER