#pragma once

#include "reflib_net_socket.h"
#include "reflib_composit_id.h"

namespace RefLib
{

class NetListener;
class GameObj;

class NetConnection : public NetSocket
{
public:
    NetConnection(uint32 id, uint32 salt)
        : _id(id, salt)
    {
    }

    uint64 GetConId() { return _id.GetCompId(); }

    // call when NetConnection is reused.
    void IncSalt()
    {
        _id.IncSalt();
    }

    bool Initialize(SOCKET sock, NetListener* netListener);
    virtual void OnDisconnected() override;

private:
    CompositId _id;
    NetListener* _container;
};

} // namespace RefLib
