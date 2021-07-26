//
// Created by Telephone on 2021/7/24.
//

#ifndef EXAMPAPERSYSTEM_TCPSERVERLINUX_H
#define EXAMPAPERSYSTEM_TCPSERVERLINUX_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "vlist.h"

typedef volatile struct _______________________TCPServerData_______________________ * volatile TCPServerData;
typedef volatile struct TCPServerProperty * volatile TCPServerProperty;

typedef volatile struct _______________TCPServer_______________ * volatile TCPServer;

typedef enum {
    RFBServerState_VERSION_AWAIT,
    RFBServerState_SECURITY_TYPE_AWAIT,
    RFBServerState_CHALLENGE_RESPONSE_AWAIT,
    RFBServerState_CLIENT_INIT_AWAIT,
    RFBServerState_CLIENT_MESSAGE_AWAIT,
} RFBServerState;

typedef enum {
    Action_NO_ACTION,
    Action_PROACTIVE_SHUTDOWN,
    Action_ERROR_SHUTDOWN,
    Action_RECV0_SHUTDOWN,
} Action;

typedef volatile struct _______________________TCPClient_______________________ * volatile TCPClient;

volatile struct _______________TCPServer_______________ {
    VLISTNODE
    TCPServerData data;
    TCPServerProperty property;
    enum {
        TCPServerStatus_CREATED,
        TCPServerStatus_RUNNING,
    } status;
    vlist client_list;
    int alive_clients_num;

    void (*AddCallback)(TCPServer,RFBServerState,Action(*)(TCPClient));
    bool (*Run)(TCPServer);
    void (*Destroy)(TCPServer,TCPServer*);

    void (*DeleteProperty)(TCPServerProperty);
};
void new_server_property(TCPServer);

bool new_tcp_server(TCPServer *tcpServer, int port, bool memoryLack, bool runImmediately);

typedef volatile struct _______________________TCPClientData_______________________ * volatile TCPClientData;
typedef volatile struct TCPClientProperty * volatile TCPClientProperty;

volatile struct _______________________TCPClient_______________________ {
    VLISTNODE
    TCPClientData data;
    TCPClientProperty property;
    RFBServerState state;

    void (*DeleteProperty)(TCPClientProperty);
};
void new_client_property(TCPClient);

typedef enum {
    FailType_SUCCESS,
    FailType_CONNECTION_ERROR,
    FailType_PEER_GRACEFUL_SHUTDOWN,
    FailType_READ_EAGAIN,
    FailType_WRITE_EAGAIN,
    FailType_READ_TIMEOUT,
    FailType_WRITE_TIMEOUT,
} FailType;

typedef volatile struct {
    Action action;
    bool success;
    size_t sz;
    FailType fail_type;
} ReadWriteRes;

/**
 * set the client to nonblocking mode, then read some data from client, FailType_READ_EAGAIN may occurs.
 */
ReadWriteRes tcp_read(TCPClient client, void *buff, size_t buffLen);
/**
 * set the client to nonblocking mode, then write some data to client, FailType_WRITE_EAGAIN may occurs.
 * @param success when writing is done, this flag will be set to true. This flag MUST be ACCESSIBLE until the writing is done.
 * @param fail if error occurs when writing, this flag will be set to true. This flag MUST be ACCESSIBLE until the writing is done.
 * @note the {@param buff} MUST be on the heap, when writing is done or error occurs, free() will be automatically called on it.
 */
ReadWriteRes tcp_write(TCPClient client, void *buff, size_t buffLen, bool *success, bool *fail);

#ifdef __cplusplus
}
#endif
#endif //EXAMPAPERSYSTEM_TCPSERVERLINUX_H
