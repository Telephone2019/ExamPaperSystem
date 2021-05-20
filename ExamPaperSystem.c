
#include <logme.h>
#include <vlist.h>
#include <vutils.h>
#include <macros.h>
#include <httpparser.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef LOGME_WINDOWS

#include <windows.h>
#include <tcpserver.h>
#include <kbhook.h>

#endif // LOGME_WINDOWS

#define HTML_200 "<html>\n"\
"<body>\n"\
"<h1>Great! 非常棒！</h1>\n"\
"<div name=\"hw\" id=\"hw\">\n"\
"<a href=\"/?#end\">Click here to test url fragment!(Jump to the end)</a>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"<h1>Hello, world!</h1>\n"\
"</div>\n"\
"<div name=\"end\" id=\"end\">\n"\
"<a href=\"/?#hw\">Click here to test url fragment!(Jump to hello,world)</a>\n"\
"</div>\n"\
"</body>\n"\
"</html>\n"
#define HTML_400 "<html>\n<body>\n<h1>Bad Request 这是一个糟糕的请求</h1>\n</body>\n</html>\n"
#define HTML_404 "<html>\n<body>\n<h1>File Not Found 文件找不到</h1>\n</body>\n</html>\n"
#define HTML_500 "<html>\n<body>\n<h1>Internal Server Error 服务器内部发生了错误</h1>\n</body>\n</html>\n"

#define REASON_PHRASE_200 "OK"
#define REASON_PHRASE_400 "Bad Request"
#define REASON_PHRASE_404 "Not Found"
#define REASON_PHRASE_500 "Internal Server Error"

typedef struct node {
    VLISTNODE
        int data;
} node;

int my_run(vlist this, long i, void* extra) {
    LogMe.w("%d", ((node*)(this->get_const(this, i)))->data);
    return i > 8;
}

enum PAPER_FILENAME_ARRAY_INDEX {
    FILE_NAME_INDEX = 0,
    DOWNLOAD_NAME_INDEX,
    MIME_TYPE_INDEX,
    FILE_CHARSET_INDEX
};
const char* const paper_filenames[][4] = {
    {"D:\\同步盘\\Documents\\各种标准文档\\C++17官方标准文档Plus\\ISOIEC 14882 2017.pdf", "C++开发测试题.pdf", MIME_TYPE_PDF, HTTP_CHARSET_UTF8},
    {"D:\\同步盘\\Documents\\各种标准文档\\Wi-Fi_Display_Technical_Specification_v2.1_0.pdf", "硬件开发测试题.pdf", MIME_TYPE_PDF, HTTP_CHARSET_UTF8}
};

enum EXAM_START_TIMESTAMPS_S_ARRAY_INDEX {
    EXAM_START_TS_INDEX = 0,
    EXAM_DURATION_INDEX
};
const long long exam_start_timestamps_s[][2] = {
    {1621494000LL, 7200LL},
    {1621497600LL, 7200LL}
};

int get_exam_id(int position) {
    if (position & 1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

typedef struct FindQuery {
    const char* expected_field;
    const char* value;
} FindQuery;

int find_query(vlist this_vlist, long i, void* extra) {
    FindQuery* fq = extra;
    KeyValuePair* kv = this_vlist->get(this_vlist, i);
    if (strlen(kv->field) == strlen(fq->expected_field) && strstr(kv->field, fq->expected_field))
    {
        fq->value = kv->value;
    }
    return 0; // go on
}

#ifdef LOGME_WINDOWS
int get_paper(HttpMessage* hmsg, HttpHandlerPac* hpac) {
    FindQuery fq = {
        .expected_field = "pos",
        .value = NULL
    };
    if (!hmsg->query_string)
    {
    handle_400:
        if (
            send_text(
                hpac->node,
                400,
                REASON_PHRASE_400,
                1,
                HTML_400,
                MIME_TYPE_HTML,
                HTTP_CHARSET_UTF8,
                0,
                NULL
            ) != 0
            )
        {
            return -97;
        }
        return 2;
    }
    hmsg->query_string->foreach(hmsg->query_string, find_query, &fq);
    int pos = -1;
    if (!fq.value || (sscanf(fq.value, "%d", &pos), pos < 0))
    {
        goto handle_400;
    }
    int eid = get_exam_id(pos);
    char encoded_name[500];
    const char* dlname = paper_filenames[eid][DOWNLOAD_NAME_INDEX];
    url_encode(dlname, strlen(dlname), encoded_name, sizeof(encoded_name), 0);
    if (
        send_file(
            hpac->node,
            paper_filenames[eid][FILE_NAME_INDEX],
            1,
            paper_filenames[eid][MIME_TYPE_INDEX],
            paper_filenames[eid][FILE_CHARSET_INDEX],
            1,
            encoded_name
        ) < 0
        ) {
        LogMe.et("get_paper() send failed");
        return -98;
    }
    return 1;
}

int get_exam_time(HttpMessage* hmsg, HttpHandlerPac* hpac) {
    FindQuery fq = {
        .expected_field = "pos",
        .value = NULL
    };
    if (!hmsg->query_string)
    {
    handle_400:
        if (
            send_text(
                hpac->node,
                400,
                REASON_PHRASE_400,
                1,
                HTML_400,
                MIME_TYPE_HTML,
                HTTP_CHARSET_UTF8,
                0,
                NULL
            ) != 0
            )
        {
            return -97;
        }
        return 2;
    }
    hmsg->query_string->foreach(hmsg->query_string, find_query, &fq);
    int pos = -1;
    if (!fq.value || (sscanf(fq.value, "%d", &pos), pos < 0))
    {
        goto handle_400;
    }
    int eid = get_exam_id(pos);
    time_t raw_timestamp_s;
    time(&raw_timestamp_s);
    long long exam_time_s = exam_start_timestamps_s[eid][EXAM_START_TS_INDEX];
    long long exam_duration_s = exam_start_timestamps_s[eid][EXAM_DURATION_INDEX];
    char response_body[150] = { 0 };
    snprintf(response_body, sizeof(response_body), "%lld %lld %lld", (long long)raw_timestamp_s, exam_time_s, exam_duration_s);
    if (
        send_text(
            hpac->node,
            200,
            REASON_PHRASE_200,
            1,
            response_body,
            MIME_TYPE_PLAIN_TEXT,
            HTTP_CHARSET_UTF8,
            0,
            NULL
        ) != 0
        )
    {
        return -98;
    }
    return 1;
}

const char* const url_path_patterns[] = {
    "/paper",
    "/examtime"
};

HTTP_HANDLE_FUNC_TYPE* const procs[] = {
    get_paper,
    get_exam_time
};

const void* const extras[] = {
    NULL,
    NULL
};

int generate_http_handlers(vlist hlist) {
    int path_num = sizeof(url_path_patterns) / sizeof(const char*);
    for (int i = 0; i < path_num; i++)
    {
        HttpHandler* hhd = zero_malloc(sizeof(HttpHandler));
        if (!hhd)
        {
            return 0;
        }
        hlist->quick_add(hlist, hhd);
        hhd->path_contains = url_path_patterns[i];
        hhd->handle_func = procs[i];
        hhd->extra = extras[i];
    }
    return 1;
}
#endif // LOGME_WINDOWS

int main()
{

#ifdef LOGME_WINDOWS
    system("chcp 65001");
#endif // LOGME_WINDOWS

    logme_init();

    LogMe.e("error");
    LogMe.w("warning");
    LogMe.i("message");
    LogMe.n("normal");
    LogMe.b("blue");

    LogMe.et("error");
    LogMe.wt("warning");
    LogMe.it("message");
    LogMe.nt("normal");
    LogMe.bt("blue");

    LogMe.i("你好，大白！%s %u", "你的学号是", 20202021);

    const char* parent_str;
    const char* sub_str;

    parent_str = "你的学号是";
    sub_str = "学";
    LogMe.i("你好，大白！%s 的子串 %s 的查找结果是 %d", parent_str, sub_str, find_sub_str(0, NULL, NULL, parent_str, sub_str, NULL, NULL, 0, 1));

    parent_str = "你的学号是";
    sub_str = "你";
    LogMe.i("你好，大白！%s 的子串 %s 的查找结果是 %d", parent_str, sub_str, find_sub_str(0, NULL, NULL, parent_str, sub_str, NULL, NULL, 0, 1));

    parent_str = "你的学号是";
    sub_str = "我";
    LogMe.i("你好，大白！%s 的子串 %s 的查找结果是 %d", parent_str, sub_str, find_sub_str(0, NULL, NULL, parent_str, sub_str, NULL, NULL, 0, 1));

    vlist list = make_vlist(sizeof(node));
    for (size_t i = 0; i < 10; i++)
    {
        list->add(list, &((node) { .data = i }));
    }
    list->insert(list, 0, &((node) { .data = -8 }));
    list->insert(list, 6, &((node) { .data = 99 }));
    list->remove(list, 9);
    list->foreach(list, my_run, NULL);
    delete_vlist(list, &list);

    void* zptr = zero_malloc(50);
    LogMe.b("zero malloc ptr = %p", zptr);
    free(zptr);

#ifdef LOGME_WINDOWS
//#define TEST_HOOK
#ifdef TEST_HOOK
    int hook_success;
    HANDLE hook, sharedobj;
    InstallHook(&hook_success, &hook, &sharedobj);
    if (hook_success) {
        LogMe.i("HOOK INSTALL SUCCESS!");
        Sleep(15000);
        UninstallHook(hook, &hook, sharedobj, &sharedobj);
        LogMe.w("HOOK UNINSTALLED");
    }
    else {
        LogMe.e("HOOK INSTALL FAIL!");
    }
#endif // TEST_HOOK
    vlist handlers = make_vlist(sizeof(HttpHandler));
    if (!handlers || !generate_http_handlers(handlers))
    {
        malloc_fail:
        delete_vlist(handlers, &handlers);
        LogMe.et("Malloc failed when generating HTTP handlers");
        return -1;
    }
    tcp_server_run(23456, 1, handlers);
    delete_vlist(handlers, &handlers);
#endif // LOGME_WINDOWS

    return 0;
}
