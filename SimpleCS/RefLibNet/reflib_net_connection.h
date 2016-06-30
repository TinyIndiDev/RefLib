#pragma once

#include "reflib_net_socket.h"
#include "reflib_composit_id.h"

namespace RefLib
{

class GameObj;
class NetConnectionProxy;

class NetConnection : public NetSocket
{
public:
    NetConnection(uint32 id, uint32 salt)
        : _id(id, salt)
        , _container(nullptr)
    {}

    CompositId GetCompId() const { return _id; }

    // call when NetConnection is reused.
    void IncSalt()
    {
        _id.IncSalt();
    }

    void RegisterParent(std::weak_ptr<NetObj> parent);

    bool Initialize(SOCKET sock, NetConnectionProxy* container);

    virtual bool RecvPacket(MemoryBlock* packet) override;
    virtual void OnConnected() override;
    virtual void OnDisconnected() override;

private:
    CompositId _id;
    std::weak_ptr<NetObj> _parent;
    NetConnectionProxy* _container;
};

} // namespace RefLib
