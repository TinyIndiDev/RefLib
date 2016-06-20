#pragma once

namespace RefLib
{

class NetResolve
{
public:
    static int PrintAddress(const SOCKADDR *sa, int salen);
    static PADDRINFOA ResolveAddressA(const std::string& addr, const std::string& port);
};

} // namespace RefLib
