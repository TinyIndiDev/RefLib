#pragma once

#define NETWORK_DEF_BACKLOG                     100
#define NETWORK_DEFAULT_OVERLAPPED_COUNT        5
#define NETWORK_MAX_COMPLETION_THREAD_COUNT     32
#define NETWORK_TIMEOUT_IN_MSEC                 5000
#define NETWORK_TIMEOUT_CHECK_INTERVAL          12

#define NETWORK_MAX_CONN                        5000

#define MAX_PACKET_SIZE				            ((1024)*(64))
#define MAX_SOCKET_BUFFER_SIZE  	            (10*MAX_PACKET_SIZE)
#define MAX_SEND_ARRAY_SIZE                     10

#define NET_STATUS_DISCONNECTED 0
#define NET_STATUS_CONNECTED    1
#define NET_STATUS_RECV         (1 << 1)
#define NET_STATUS_SEND         (1 << 2)
#define NET_STATUS_CLOSING      (1 << 3)

enum NetCloseType
{
    NET_CTYPE_CLIENT,
    NET_CTYPE_SYSTEM,
    NET_CTYPE_SHUTDOWN,
};

