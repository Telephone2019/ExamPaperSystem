#include "httpparser.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <vutils.h>

static long long last_index_of_str(const char* s, const char* pattern, long long before_index) {
	size_t slen = strlen(s);
	before_index = (before_index >= 0 ? before_index : 0);
	const char* before_ptr = s + before_index;
	const char* found = NULL;
	const char* pt = s;
	for (size_t i = 0; i < slen; i++)
	{
		const char *next_found = strstr(pt++, pattern);
		if (!next_found || next_found >= before_ptr)
		{
			break;
		}
		found = next_found;
	}
	return found ? (found - s) : -1;
}

static long long last_index_of_ch(const char* s, char ch, long long before_index) {
	char substr[2] = {ch, 0};
	return last_index_of_str(s, substr, before_index);
}

static void bm_shift(size_t* p_head_index, size_t num, char *buf, char(*generator)(void*), void *generator_param_p) {
	if (generator)
	{
		for (size_t i = 0; i < num; i++)
		{
			buf[++(*p_head_index)] = generator(generator_param_p);
		}
	}
	else
	{
		(*p_head_index) += num;
	}
}

// 此函数从头查找第一个符合的子字符串。有两种工作模式：流模式和固定字符串模式。
// 返回值：
// -1 ：没找到
// -2 : 动态内存分配失败
// >=0 : 流模式下代表字符生成器被调用的次数；固定字符串模式下代表找到的子字符串的最后一个字符的后一个下标（不管是否超出数组界限）
int find_sub_str(size_t max_call_time, char(*generator)(void*), void* generator_param_p, const char *str, const char *pattern) {
	const size_t plen = strlen(pattern);
	size_t shifted = 0;
	max_call_time = (str ? strlen(str) : max_call_time);
	generator = (str ? NULL : generator);
	generator_param_p = (str ? NULL : generator_param_p);

	char* temp = (str ? str : zero_malloc(max_call_time + 1));
	if (!temp)
	{
		return -2;
	}

	// size_t is unsigned type, SIZE_MAX plus 1 will overflow to 0
	size_t head_index = SIZE_MAX;

	shifted += plen;
	if (shifted > max_call_time)
	{
		if (!str)free(temp);
		return -1;
	}
	bm_shift(&head_index, plen, temp, generator, generator_param_p);

	loop:
	size_t tail_index = head_index + 1 - plen;
	for (ptrdiff_t look_back_index = head_index; look_back_index >= (ptrdiff_t)tail_index; look_back_index--)
	{
		size_t pattern_index = look_back_index - tail_index;
		char pch = pattern[pattern_index];
		char tch = temp[look_back_index];
		if (tch == pch)
		{
			continue;
		}
		else
		{
			long long left_index_of_tch_in_pattern = last_index_of_ch(pattern, tch, pattern_index);
			size_t shift_len = ((long long)pattern_index) - left_index_of_tch_in_pattern;
			if (look_back_index != head_index)
			{
				char hch = temp[head_index];
				size_t pattern_index_2 = plen - 1;
				long long left_index_of_hch_in_pattern = last_index_of_ch(pattern, hch, pattern_index_2);
				size_t shift_len_2 = ((long long)pattern_index_2) - left_index_of_hch_in_pattern;
				if (shift_len_2 > shift_len)
				{
					shift_len = shift_len_2;
				}
			}
			shifted += shift_len;
			if (shifted > max_call_time)
			{
				if (!str)free(temp);
				return -1;
			}
			bm_shift(&head_index, shift_len, temp, generator, generator_param_p);
			goto loop;
		}
	}
	if (!str)free(temp);
	return shifted;
}