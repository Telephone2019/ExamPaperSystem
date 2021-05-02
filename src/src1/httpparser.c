#include "httpparser.h"

#ifdef CASE_INSENSITIVE_STRSTR_WINDOWS
#include <Shlwapi.h>
#include <string.h>
#elif defined(CASE_INSENSITIVE_STRSTR_GCC)
#define _GNU_SOURCE
#include <string.h>
#elif defined(CASE_INSENSITIVE_STRSTR_VUTILS)
#include <vutils.h>
#include <string.h>
#else
#include <string.h>
#endif // CASE_INSENSITIVE_STRSTR_WINDOWS

#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <vutils.h>
#include <vlist.h>

static long long last_index_of_str(const char* s, const char* pattern, long long before_index

#ifdef CASE_INSENSITIVE_STRSTR
	, int case_sensitive

#ifdef CASE_INSENSITIVE_STRSTR_VUTILS
	, int *success
#endif // CASE_INSENSITIVE_STRSTR_VUTILS

#endif // CASE_INSENSITIVE_STRSTR

) {
	size_t slen = strlen(s);
	before_index = (before_index >= 0 ? before_index : 0);
	const char* before_ptr = s + before_index;
	const char* found = NULL;
	const char* pt = s;
	for (size_t i = 0; i < slen; i++)
	{
		const char* next_found =
#ifdef CASE_INSENSITIVE_STRSTR
			case_sensitive ?
#endif // CASE_INSENSITIVE_STRSTR
			strstr(pt++, pattern)
#ifdef CASE_INSENSITIVE_STRSTR
			:
#ifdef CASE_INSENSITIVE_STRSTR_WINDOWS
			StrStrI(pt++, pattern)
#elif defined(CASE_INSENSITIVE_STRSTR_GCC)
			strcasestr(pt++, pattern)
#elif defined(CASE_INSENSITIVE_STRSTR_VUTILS)
		vstrstr(pt++, pattern, case_sensitive, success)
#else
			strstr(pt++, pattern)
#endif // CASE_INSENSITIVE_STRSTR_WINDOWS
#endif // CASE_INSENSITIVE_STRSTR
			;
#if defined(CASE_INSENSITIVE_STRSTR) && defined(CASE_INSENSITIVE_STRSTR_VUTILS)
			if (!(*success))
			{
				return -1;
			}
#endif // defined(CASE_INSENSITIVE_STRSTR) && defined(CASE_INSENSITIVE_STRSTR_VUTILS)

		if (!next_found || next_found >= before_ptr)
		{
			break;
		}
		found = next_found;
	}
	return found ? (found - s) : -1;
}

static long long last_index_of_ch(const char* s, char ch, long long before_index

#ifdef CASE_INSENSITIVE_STRSTR
	, int case_sensitive

#ifdef CASE_INSENSITIVE_STRSTR_VUTILS
	, int* success
#endif // CASE_INSENSITIVE_STRSTR_VUTILS

#endif // CASE_INSENSITIVE_STRSTR

) {
	char substr[2] = {ch, 0};
#ifdef CASE_INSENSITIVE_STRSTR
	return last_index_of_str(s, substr, before_index, case_sensitive
#ifdef CASE_INSENSITIVE_STRSTR_VUTILS
		, success
#endif // CASE_INSENSITIVE_STRSTR_VUTILS
	);
#else
	return last_index_of_str(s, substr, before_index);
#endif // CASE_INSENSITIVE_STRSTR
}

// generator 可能调用失败。
// 如果某次 generator 调用失败，那么此次 p_head_index 下标不会增加，此次 buf 也不会被写入，
// 此次滑动取消（只是取消这一个字符距离的滑动，不是取消所有滑动），同时立即停止滑动。
// 返回值：成功滑动的距离（字符为单位）
static size_t bm_shift(size_t* p_head_index, size_t num, char *buf, GENERATOR_FUNCTION_TYPE* generator, GENERATOR_PARAM_TYPE* generator_param_p, char* generated_buf, size_t generated_buf_len, size_t *generated_used_len_p) {
	if (generator)
	{
		for (size_t i = 0; i < num; i++)
		{
			int continue_flag = 1;
			char read_ch = generator(generator_param_p, &continue_flag);
			if (!continue_flag)
			{
				return i;
			}
			buf[++(*p_head_index)] = read_ch;
			if (*generated_used_len_p < generated_buf_len)
			{
				generated_buf[(*generated_used_len_p)++] = read_ch;
			}
		}
	}
	else
	{
		(*p_head_index) += num;
	}
	return num;
}

int find_sub_str(size_t max_call_time, GENERATOR_FUNCTION_TYPE* generator, GENERATOR_PARAM_TYPE* generator_param_p, const char* str, const char* pattern, size_t* call_time, char* generated_buf, size_t generated_buf_len

#ifdef CASE_INSENSITIVE_STRSTR
	, int case_sensitive
#endif // CASE_INSENSITIVE_STRSTR

) {
	const size_t plen = strlen(pattern);
	size_t shifted = 0;
	size_t generated_used_len = 0;
	size_t nothing;
	call_time == NULL ? (call_time = &nothing) : (call_time);
	max_call_time = (str ? strlen(str) : max_call_time);
	generator = (str ? NULL : generator);
	generator_param_p = (str ? NULL : generator_param_p);

	char* temp = (str ? str : zero_malloc(max_call_time + 1));
	if (!temp)
	{
		*call_time = shifted;
		return -2;
	}

#ifdef SIZE_MAX
	// size_t is unsigned type, SIZE_MAX plus 1 will overflow to 0
	size_t head_index = SIZE_MAX;
#else
	size_t head_index = (size_t)0-(size_t)1;
#endif // SIZE_MAX

	shifted += plen;
	if (shifted > max_call_time)
	{
		shifted -= plen;
		if (!str)free(temp); *call_time = shifted;
		return -1;
	}
	size_t a_shift = bm_shift(&head_index, plen, temp, generator, generator_param_p, generated_buf, generated_buf_len, &generated_used_len);
	if (a_shift < plen)
	{
		shifted -= plen; shifted += a_shift;
		if (!str)free(temp); *call_time = shifted;
		return -3;
	}

loop:;
	size_t tail_index = head_index + 1 - plen;
	for (ptrdiff_t look_back_index = head_index; look_back_index >= (ptrdiff_t)tail_index; look_back_index--)
	{
		size_t pattern_index = look_back_index - tail_index;
		char pch = pattern[pattern_index];
		char tch = temp[look_back_index];
		if (
#ifdef CASE_INSENSITIVE_STRSTR
			case_sensitive ?
#endif // CASE_INSENSITIVE_STRSTR
			tch == pch
#ifdef CASE_INSENSITIVE_STRSTR
			:
		toupper(tch) == toupper(pch)
#endif // CASE_INSENSITIVE_STRSTR
			)
		{
			continue;
		}
		else
		{
			int success = 1;
			long long left_index_of_tch_in_pattern = last_index_of_ch(pattern, tch, pattern_index
#ifdef CASE_INSENSITIVE_STRSTR
				, case_sensitive
#ifdef CASE_INSENSITIVE_STRSTR_VUTILS
				, &success
#endif // CASE_INSENSITIVE_STRSTR_VUTILS
#endif // CASE_INSENSITIVE_STRSTR
			);
			if (!success)
			{
				if (!str)free(temp); *call_time = shifted;
				return -2;
			}
			size_t shift_len = ((long long)pattern_index) - left_index_of_tch_in_pattern;
			if (look_back_index != head_index)
			{
				char hch = temp[head_index];
				size_t pattern_index_2 = plen - 1;
				long long left_index_of_hch_in_pattern = last_index_of_ch(pattern, hch, pattern_index_2
#ifdef CASE_INSENSITIVE_STRSTR
					, case_sensitive
#ifdef CASE_INSENSITIVE_STRSTR_VUTILS
					, &success
#endif // CASE_INSENSITIVE_STRSTR_VUTILS
#endif // CASE_INSENSITIVE_STRSTR
				);
				if (!success)
				{
					if (!str)free(temp); *call_time = shifted;
					return -2;
				}
				size_t shift_len_2 = ((long long)pattern_index_2) - left_index_of_hch_in_pattern;
				if (shift_len_2 > shift_len)
				{
					shift_len = shift_len_2;
				}
			}
			shifted += shift_len;
			if (shifted > max_call_time)
			{
				shifted -= shift_len;
				if (!str)free(temp); *call_time = shifted;
				return -1;
			}
			size_t a_shift = bm_shift(&head_index, shift_len, temp, generator, generator_param_p, generated_buf, generated_buf_len, &generated_used_len);
			if (a_shift < shift_len)
			{
				shifted -= shift_len; shifted += a_shift;
				if (!str)free(temp); *call_time = shifted;
				return -3;
			}
			goto loop;
		}
	}
	if (!str)free(temp); *call_time = shifted;
	return shifted;
}

#ifdef CASE_INSENSITIVE_STRSTR
typedef struct generator_wrapper_param {
	GENERATOR_FUNCTION_TYPE* generator;
	GENERATOR_PARAM_TYPE* generator_param_p;
	vlist generated_buffer;
	long next_read_buffer_index;
} generator_wrapper_param;

typedef struct char_node {
	VLISTNODE
		char ch;
} char_node;

static char generator_wrapper(void* generator_wrapper_param_p, int* continue_flag) {
	generator_wrapper_param* gwpp = generator_wrapper_param_p;
	GENERATOR_FUNCTION_TYPE* generator = gwpp->generator;
	GENERATOR_PARAM_TYPE* gpp = gwpp->generator_param_p;
	vlist generated_buffer = gwpp->generated_buffer;
	if (gwpp->next_read_buffer_index >= generated_buffer->size)
	{
		char read_ch = generator(gpp, continue_flag);
		if (*continue_flag)
		{
			// 只有 generator 调用成功，才把 generator 的返回字符放入缓冲区
			generated_buffer->add(generated_buffer, &((char_node) {.ch = read_ch}));
			gwpp->next_read_buffer_index = generated_buffer->size;
		}
		return read_ch;
	}
	else
	{
		*continue_flag = 1;
		return ((const char_node*)(generated_buffer->get_const(generated_buffer, gwpp->next_read_buffer_index++)))->ch;
	}
}

int next_http_message(HttpMethod *method_p, char **message_pp, GENERATOR_FUNCTION_TYPE* generator, GENERATOR_PARAM_TYPE* generator_param_p) {
	if (!method_p || !message_pp)
	{
		return -1;
	}
	const char *(http_method_names[]) = {
		"GET", "POST", "HEAD", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"
	};
	vlist generated_buffer = make_vlist(sizeof(char_node));
	generator_wrapper_param gwp = {
		.generator = generator,
		.generator_param_p = generator_param_p,
		.generated_buffer = generated_buffer,
		.next_read_buffer_index = 0
	};
	int malloc_fail_type = 0;
again:;
	*method_p = INVALID_METHOD;
	for (HttpMethod i = GET; i < INVALID_METHOD; i++)
	{
		int method_f_res = find_sub_str(MAX_HTTP_HEADERS_LENGTH, generator_wrapper, &gwp, NULL, http_method_names[i], NULL, NULL, 0, 0);
		if (method_f_res >= 0)
		{
			*method_p = i;
			int empty_line_f_res = find_sub_str(MAX_HTTP_HEADERS_LENGTH, generator_wrapper, &gwp, NULL, "\r\n\r\n", NULL, NULL, 0, 0);
			if (empty_line_f_res >= 0)
			{
				int m_start_index = method_f_res - strlen(http_method_names[i]);
				int m_end_index = method_f_res + empty_line_f_res;
				int m_len = m_end_index - m_start_index;
				*message_pp = zero_malloc(m_len + 1);
				if (!(*message_pp))
				{
					malloc_fail_type = -4;
					goto malloc_fail;
				}
				for (int m_i = m_start_index, dest_i = 0; m_i < m_end_index; m_i++, dest_i++)
				{
					(*message_pp)[dest_i] = ((const char_node*)(generated_buffer->get_const(generated_buffer, m_i)))->ch;
				}
				delete_vlist(generated_buffer, &generated_buffer);
				return m_len;
			}
			else if (empty_line_f_res == -1)
			{
				goto not_found;
			}
			else if (empty_line_f_res == -2)
			{
				malloc_fail_type = -2;
				goto malloc_fail;
			}
			else if (empty_line_f_res == -3)
			{
				goto generator_fail;
			}
			else
			{
				method_f_res = empty_line_f_res;
				goto unknown_error;
			}
		}
		else if (method_f_res == -1)
		{
			// not found
		not_found:;
			gwp.next_read_buffer_index = 0;
			continue;
		}
		else if (method_f_res == -2)
		{
			// malloc fail
			malloc_fail_type = -2;
		malloc_fail:;
			*method_p = INVALID_METHOD;
			*message_pp = NULL;
			delete_vlist(generated_buffer, &generated_buffer);
			return malloc_fail_type;
		}
		else if (method_f_res == -3)
		{
			// generator() fail
		generator_fail:;
			*method_p = INVALID_METHOD;
			*message_pp = NULL;
			delete_vlist(generated_buffer, &generated_buffer);
			return -3;
		}
		else
		{
			// unknown error
		unknown_error:;
			*method_p = INVALID_METHOD;
			*message_pp = NULL;
			delete_vlist(generated_buffer, &generated_buffer);
			return method_f_res;
		}
	}
	if (*method_p == INVALID_METHOD)
	{
		generated_buffer->remove(generated_buffer, 0);
		gwp.next_read_buffer_index = 0;
		goto again;
	}
	delete_vlist(generated_buffer, &generated_buffer);
	while (1);
}
#endif // CASE_INSENSITIVE_STRSTR