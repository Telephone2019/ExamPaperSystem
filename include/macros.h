#ifndef MACROS
#define MACROS

#ifdef __GNUC__
#define LOGME_GCC
#ifdef _WIN32
#define LOGME_WINDOWS
#endif // _WIN32
#endif // __GNUC__
#ifdef _MSC_VER
#define LOGME_MSVC
#ifdef _WIN32
#define LOGME_WINDOWS
#endif // _WIN32
#endif // _MSC_VER

#endif // MACROS