#ifndef HTTPPARSER
#define HTTPPARSER

#include <stddef.h>

int find_sub_str(size_t max_call_time, char(*generator)(void*, int*), void* generator_param_p, const char* str, const char* pattern, size_t* call_time);

#endif // !HTTPPARSER