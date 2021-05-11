#ifndef LOGME
#define LOGME

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

#endif // LOGME