#include "logme.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "macros.h"

#ifdef LOGME_WINDOWS // 用这个宏来判断是否是 windows 平台

#include <windows.h>

#define GREEN FOREGROUND_GREEN
#define YELLOW 6
#define RED FOREGROUND_RED
#define WHITE 7
#define NORMAL_WHILE WHITE
#define NORMAL NORMAL_WHILE
#define LINE "\n"

#else

#define GREEN "\e[1;32m"
#define YELLOW "\e[1;33m"
#define RED "\e[1;31m"
#define NORMAL "\e[0m"
#define LINE "\n"

#endif

static void* malloc_n(size_t n) {
    void* res = malloc(n);
    memset(res, 0, n);
    return res;
}

static char* line(const char* s) {
    char* res = malloc_n(strlen(s) + strlen(LINE) + 1);

    strcat(res, s);
    strcat(res, LINE);

    return res;
}

#ifdef LOGME_WINDOWS

static void set_console_text_color(WORD color) {
    HANDLE consolehwnd = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(consolehwnd, color);
}

static void reset_console_text_color() {
    set_console_text_color(NORMAL);
}

static void l(const char* text, WORD color, va_list vlist) {
    const char* text_line = line(text);
    set_console_text_color(color);
    vprintf(text_line, vlist);
    reset_console_text_color();
    free(text_line);
}

#else

static const char* beautify(const char* s, const char* color) {
    char* res = malloc_n(strlen(s) + strlen(color) + strlen(NORMAL) + 1);

    strcat(res, color);
    strcat(res, s);
    strcat(res, NORMAL);

    return res;
}

static void l(const char* text, const char* color, va_list vlist) {
    const char* bs = beautify(text, color);
    const char* bs_line = line(bs);
    vprintf(bs_line, vlist);
    free(bs_line);
    free(bs);
}

#endif

static void log_me_i__(const char* text, ...) {
    va_list vlist;
    va_start(vlist, text);
    l(text, GREEN, vlist);
    va_end(vlist);
}
static void log_me_w__(const char* text, ...) {
    va_list vlist;
    va_start(vlist, text);
    l(text, YELLOW, vlist);
    va_end(vlist);
}
static void log_me_e__(const char* text, ...) {
    va_list vlist;
    va_start(vlist, text);
    l(text, RED, vlist);
    va_end(vlist);
}
static void log_me_n__(const char* text, ...) {
    va_list vlist;
    va_start(vlist, text);
    l(text, NORMAL, vlist);
    va_end(vlist);
}

struct LogMe LogMe = { log_me_i__, log_me_w__, log_me_e__, log_me_n__ };