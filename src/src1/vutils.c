#include "vutils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <macros.h>

char* vitoa(int i, char* s, size_t len) {
	snprintf(s, len, "%d", i);
	return s;
}

void* zero_malloc(size_t n) {
    void* res = malloc(n);
	if (res != NULL)
	{
		memset(res, 0, n);
	}
    return res;
}

#ifdef LOGME_WINDOWS
#include<windows.h>
int test_wide_char_num_of_utf8_including_wide_null(const char* utf8str) {
	return MultiByteToWideChar(65001, 0, utf8str, -1, NULL, 0);
}
int utf8_to_utf16_must_have_sufficient_buffer_including_wide_null(const char* utf8str, char16_t* char16buf, int buf_len_wide_char_num) {
	return MultiByteToWideChar(65001, 0, utf8str, -1, char16buf, buf_len_wide_char_num);
}
#endif // LOGME_WINDOWS