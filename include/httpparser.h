#ifndef HTTPPARSER
#define HTTPPARSER

#include <stddef.h>

// �˺�����ͷ���ҵ�һ�����ϵ����ַ����������ֹ���ģʽ����ģʽ�͹̶��ַ���ģʽ��
// ����ֵ��
// -1 ��û�ҵ�
// -2 : ��̬�ڴ����ʧ��
// -3 : �ַ�����������ʧ�ܡ��ַ����������óɹ��Ĵ����洢�ڲ��� call_time ָ��ı�����
// >=0 : �ҵ��ˡ�û�д���������ģʽ�·���ֵ���ַ������������õĴ������̶��ַ���ģʽ�·���ֵ���ҵ������ַ��������һ���ַ��ĺ�һ���±꣨�����Ƿ񳬳�������ޣ�
int find_sub_str(size_t max_call_time, char(*generator)(void*, int*), void* generator_param_p, const char* str, const char* pattern, size_t* call_time);

#endif // !HTTPPARSER