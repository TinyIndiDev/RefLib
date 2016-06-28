#include "stdafx.h"

#include "reflib_net_connector.h"
#include "reflib_net_obj.h"
#include "reflib_net_api.h"
#include "reflib_util.h"

namespace RefLib
{

NetConnector::NetConnector(NetService* container)
    : NetConnectionProxy(container)
{
}

NetConnector::~NetConnector()
{
}

bool NetConnector::Connect(const std::string& ipStr, uint32 port, std::weak_ptr<NetObj> obj)
{
    auto p = obj.lock();
    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(p, "Connect: NetObj is null", false);

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ipStr.c_str(), &addr.sin_addr) != 1)
    {
        DebugPrint("NetConnector Initialization: inet_pton failed due to the invalid ip address.");
        return false;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        DebugPrint("Cannot create listen socket: %s", SocketGetLastErrorString().c_str());
        return false;
    }

    return p->Connect(sock, addr);
}

} // namespace RefLib
