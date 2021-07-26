//
// Created by Telephone on 2021/7/24.
//
#ifdef __cplusplus
extern "C" {
#endif

#include "tcpserverlinux.h"

__attribute__ ((weak)) void new_server_property(TCPServer server){
    server->property = NULL;
    server->DeleteProperty = NULL;
}
__attribute__ ((weak)) void new_client_property(TCPClient client){
    client->property = NULL;
    client->DeleteProperty = NULL;
}

#ifdef __cplusplus
}
#endif
