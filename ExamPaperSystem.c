
#include <logme.h>
#include <vlist.h>
#include <vutils.h>
#include <macros.h>
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

int my_run(vlist this, long i) {
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
    vlist list = make_vlist(sizeof(node));
    for (size_t i = 0; i < 10; i++)
    {
        list->add(list, &((node) { .data = i }));
    }
    list->insert(list, 0, &((node) { .data = -8 }));
    list->insert(list, 6, &((node) { .data = 99 }));
    list->remove(list, 9);
    list->foreach(list, my_run);
    delete_vlist(list, &list);

    void* zptr = zero_malloc(50);
    LogMe.b("zero malloc ptr = %p", zptr);
    free(zptr);

#ifdef LOGME_WINDOWS
    kbhook_run_success();
    tcp_server_run(23456, 1);
#endif // LOGME_WINDOWS

    return 0;
}
