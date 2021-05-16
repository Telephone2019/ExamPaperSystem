
#include <logme.h>
#include <vlist.h>
#include <vutils.h>
#include <macros.h>
#include <httpparser.h>
#include <stdlib.h>

#ifdef LOGME_WINDOWS

#include <windows.h>
#include <tcpserver.h>
#include <kbhook.h>

#endif // LOGME_WINDOWS

typedef struct node {
    VLISTNODE
        int data;
} node;

int my_run(vlist this, long i, void* extra) {
    LogMe.w("%d", ((node*)(this->get_const(this, i)))->data);
    return i > 8;
}

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
        Sleep(1000000);
        UninstallHook(hook, &hook, sharedobj, &sharedobj);
        LogMe.w("HOOK UNINSTALLED");
    }
    else {
        LogMe.e("HOOK INSTALL FAIL!");
    }
#endif // TEST_HOOK
    tcp_server_run(23456, 1);
#endif // LOGME_WINDOWS

    return 0;
}
