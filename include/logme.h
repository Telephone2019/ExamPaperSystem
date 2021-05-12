#ifndef LOGME
#define LOGME

#include <macros.h>
#include <stdarg.h>

typedef void LOG_FUNCTION_TYPE(const char* text, ...);

extern const struct LogMe
{
    LOG_FUNCTION_TYPE* i;
    LOG_FUNCTION_TYPE* w;
    LOG_FUNCTION_TYPE* e;
    LOG_FUNCTION_TYPE* n;
    LOG_FUNCTION_TYPE* b;

    LOG_FUNCTION_TYPE* it;
    LOG_FUNCTION_TYPE* wt;
    LOG_FUNCTION_TYPE* et;
    LOG_FUNCTION_TYPE* nt;
    LOG_FUNCTION_TYPE* bt;
} LogMe;

// avoid calling this function from multiple threads
void logme_init();

#ifdef V_BARE_METAL
// �˺��������ʽ������ַ�����
void logme_vprintf(const char* restrict format, va_list vlist);
// �˺������ص�ǰʱ��ֵ��������־�е�ʱ���ǡ���λ�Զ��������Զ���
long long logme_get_time();
// �˺���ִ�� LogMe ���������ⲿģ���ʼ����
// ���磬����� logme_vprintf() ��ʹ�ô����������ô��Ȼ��ʹ�� LogMe ǰӦ��ʼ�����ڡ�
// ��������£�����Խ����ڳ�ʼ��������� logme_prepare() �У���Ϊ logme_init() ����� logme_prepare()��
// logme_prepare() �ķ���ֵָʾ�ⲿģ���Ƿ��ʼ���ɹ���0ʧ�ܣ�����ֵ�ɹ�����
// logme_init() ��ѭ������ logme_prepare() ֱ�� logme_prepare() ���ط���ֵ��
// �������Ҫ��ʼ���ⲿģ�飬���� logme_prepare() ��ֱ�ӷ��ط���ֵ��
int logme_prepare();
#endif // V_BARE_METAL

#endif // LOGME