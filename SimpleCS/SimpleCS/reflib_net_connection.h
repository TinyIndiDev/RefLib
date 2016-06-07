#pragma once

#include "reflib_net_socket.h"

namespace RefLib
{

class NetListener;

class NetConnection : public NetSocket
{
public:
    NetConnection(uint32 id, uint32 salt)
        : _id(id)
        , _salt(salt)
    {
    }

    uint64 GetConId() { return (_id << 16 | _salt); }

    // call when NetConnection is reused.
    void IncSalt()
    {
        _salt = (_salt + 1) % MAXUINT32;
    }

    bool Initialize(SOCKET sock, NetListener* netListener);
    virtual void OnDisconnected() override;

private:
    uint32 _id;
    uint32 _salt;

    NetListener* _container;
};

} // namespace RefLib
