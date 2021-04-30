#include "httputils.h"

#include <stdio.h>
#include <time.h>

#include <macros.h>

#define HTTP_KEEP_ALIVE_VALUE "keep-alive"
#define HTTP_NOT_KEEP_ALIVE_VALUE "close"

void http_response_date_line(char* buf, size_t buf_len) {
	const char* days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
		"Aug", "Sep", "Oct", "Nov", "Dec" };

#ifdef LOGME_MSVC
	struct tm stm;

	errno_t get_time_res = gmtime_s(&stm, &((time_t) { time(NULL) }));

	if (get_time_res != 0)
	{
		if (buf_len > 0)
		{
			*buf = '\0';
		}
	}
	else
	{
		snprintf(buf, buf_len, "%s, %d %s %d %02d:%02d:%02d GMT\r\n",
			days[stm.tm_wday], stm.tm_mday, months[stm.tm_mon],
			stm.tm_year + 1900, stm.tm_hour, stm.tm_min, stm.tm_sec);
	}
#else
	if (buf_len > 0)
	{
		*buf = '\0';
	}
#endif // LOGME_MSVC
}

void http_response_content_length_line(char* buf, size_t buf_len, unsigned long long content_len) {
	if (content_len <= 0ULL)
	{
		if (buf_len > 0)
		{
			*buf = '\0';
		}
	}
	else
	{
		snprintf(buf, buf_len, "Content-Length: %llu\r\n", content_len);
	}
}

void http_response_content_type_line(int enabled, char* buf, size_t buf_len, const char* MIME_type, const char* charset) {
	if (MIME_type == NULL)
	{
		MIME_type = "";
	}
	enabled ?
		(
			charset == NULL ?
			snprintf(buf, buf_len, "Content-Type: %s\r\n", MIME_type) :
			snprintf(buf, buf_len, "Content-Type: %s; charset=%s\r\n", MIME_type, charset)
			) :
		(
			buf_len > 0 ?
			*buf = '\0' :
			0
			);
}

void http_response_content_disposition_line(int enabled, char* buf, size_t buf_len, const char* filename) {
	enabled ?
		(
			filename == NULL ?
			snprintf(buf, buf_len, "Content-Disposition: attachment\r\n") :
			snprintf(buf, buf_len, "Content-Disposition: attachment; filename=\"%s\"\r\n", filename)
			) :
		(
			buf_len > 0 ?
			*buf = '\0' :
			0
			);
}

void http_response(char* buf, size_t buf_len, int status_code, const char* reason_phrase, int keep_alive, const char* header_lines, unsigned long long body_len, const char* content_MIME_type, const char* content_charset, int is_download, const char* download_filename) {
	if (reason_phrase == NULL)
	{
		reason_phrase = "";
	}
	if (header_lines == NULL)
	{
		header_lines = "";
	}

	char date_line[100];
	http_response_date_line(date_line, sizeof(date_line));
	char content_len_line[100];
	http_response_content_length_line(content_len_line, sizeof(content_len_line), body_len);
	char content_type_line[100];
	http_response_content_type_line(
		body_len > 0,
		content_type_line,
		sizeof(content_type_line),
		content_MIME_type,
		content_charset
	);
	char content_disposition_line[1000];
	http_response_content_disposition_line(
		body_len > 0 && is_download,
		content_disposition_line,
		sizeof(content_disposition_line),
		download_filename
	);

	snprintf(buf, buf_len,
		"HTTP/1.1 %d %s\r\n"
		"%s"
		"%s"
		"%s"
		"%s"
		"Server: " SERVER_NAME "\r\n"
		"Connection: %s\r\n"
		"%s"
		"\r\n",
		status_code, reason_phrase,
		date_line,
		content_len_line,
		content_type_line,
		content_disposition_line,
		keep_alive ? HTTP_KEEP_ALIVE_VALUE : HTTP_NOT_KEEP_ALIVE_VALUE,
		header_lines
	);
}
