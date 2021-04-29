#ifndef HTTPUTILS
#define HTTPUTILS

#include <stddef.h>

#define SERVER_NAME "HttpUtils"

void http_response_date_line(char* buf, size_t buf_len);

void http_response_content_length_line(char* buf, size_t buf_len, unsigned long long content_len);

void http_response_content_type_line(int enabled, char* buf, size_t buf_len, const char* MIME_type, const char* charset);

void http_response_content_disposition_line(int enabled, char* buf, size_t buf_len, const char* filename);

void http_response(char* buf, size_t buf_len, int status_code, const char* reason_phrase, int keep_alive, const char* header_lines, unsigned long long body_len, const char* content_MIME_type, const char* content_charset, int is_download, const char* download_filename);

#endif // !HTTPUTILS
