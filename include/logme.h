#ifndef LOGME
#define LOGME

typedef void LOG_FUNCTION_TYPE(const char* text, ...);

extern struct LogMe
{
    LOG_FUNCTION_TYPE* i;
    LOG_FUNCTION_TYPE* w;
    LOG_FUNCTION_TYPE* e;
    LOG_FUNCTION_TYPE* n;
} LogMe;

#endif // LOGME