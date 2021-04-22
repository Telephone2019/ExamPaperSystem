#ifndef VUTILS
#define VUTILS

#include <stddef.h>

#include <macros.h>

char* vitoa(int i, char* s, size_t len);

void* zero_malloc(size_t n);

#ifdef LOGME_WINDOWS
#include <uchar.h>
int test_wide_char_num_of_utf8_including_wide_null(const char *utf8str);
int utf8_to_utf16_must_have_sufficient_buffer_including_wide_null(const char* utf8str, char16_t* char16buf, int buf_len_wide_char_num);
#endif // LOGME_WINDOWS

#endif // !VUTILS
