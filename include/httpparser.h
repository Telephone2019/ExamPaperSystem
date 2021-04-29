#ifndef HTTPPARSER
#define HTTPPARSER

int find_sub_str(size_t max_call_time, char(*generator)(void*), void* generator_param_p, const char* str, const char* pattern);

#endif // !HTTPPARSER