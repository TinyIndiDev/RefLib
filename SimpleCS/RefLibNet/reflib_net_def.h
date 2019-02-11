#pragma once

#define NETWORK_DEF_BACKLOG                     100
#define NETWORK_DEFAULT_OVERLAPPED_COUNT        5
#define NETWORK_MAX_COMPLETION_THREAD_COUNT     32

#define NETWORK_MAX_CONN                        5000

#define MAX_PACKET_SIZE				            ((1024)*(64))
#define DEF_SOCKET_BUFFER_SIZE  	            (10*MAX_PACKET_SIZE)
#define MAX_SOCKET_BUFFER_SIZE  	            (20*MAX_PACKET_SIZE)
#define MAX_SEND_ARRAY_SIZE                     10

#define NET_STATUS_DISCONNECTED     0
#define NET_STATUS_CONN_PENDING     (1 << 0)
#define NET_STATUS_CONNECTED        (1 << 1)
#define NET_STATUS_RECV_PENDING     (1 << 2)
#define NET_STATUS_SEND_PENDING     (1 << 3)
#define NET_STATUS_CLOSE_PENDING    (1 << 4)

enum NetCloseType
{
    NET_CTYPE_CLIENT,
    NET_CTYPE_SYSTEM,
    NET_CTYPE_SHUTDOWN,
};

enum NetServiceChildType
{
    NET_CTYPE_NA,
    NET_CTYPE_LISTENER,
    NET_CTYPE_CONNECTOR,
    NET_CTYPE_NETWORKER,
};

