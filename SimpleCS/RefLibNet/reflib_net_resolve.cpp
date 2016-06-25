#include "stdafx.h"

#include "reflib_net_resolve.h"
#include <string>

namespace RefLib
{

int NetResolve::PrintAddress(const SOCKADDR *sa, int salen)
{
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];
    int hostlen = NI_MAXHOST;
    int servlen = NI_MAXSERV;

    int rc = getnameinfo(sa, salen, host, hostlen, serv, servlen, NI_NUMERICHOST | NI_NUMERICSERV);
    if (rc != 0)
    {
        DebugPrint("%s: getnameinfo failed: %d", __FILE__, rc);
        return rc;
    }

    // If the port is zero then don't print it
    if (strcmp(serv, "0") != 0)
    {
        if (sa->sa_family == AF_INET)
            DebugPrint("[%s]:%s", host, serv);
        else
            DebugPrint("%s:%s", host, serv);
    }
    else
        DebugPrint("%s", host);
    
    return NO_ERROR;
}

PADDRINFOA NetResolve::ResolveAddressA(const std::string& addr, const std::string& port)
{
    ADDRINFOA hints;
    PADDRINFOA res = nullptr;
    PADDRINFOA ptr = nullptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = ((addr.c_str()) ? 0 : AI_PASSIVE);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int rc = getaddrinfo(addr.c_str(), port.c_str(), &hints, &res);
    if (rc != 0)
    {
        DebugPrint("Invalid address %s, getaddrinfo failed: %d", addr.c_str(), rc);
        return nullptr;
    }
    return res;
}

} // namespace RefLib