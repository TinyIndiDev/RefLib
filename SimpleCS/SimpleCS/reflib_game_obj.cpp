#include "stdafx.h"
#include "reflib_game_obj.h"
#include "reflib_util.h"
#include "reflib_net_connection.h"

namespace RefLib
{

GameObj::GameObj(const CompositId& id, HANDLE comPort)
    : _id(id)
    , _conn(nullptr)
    , _comPort(comPort)
{

}

GameObj::~GameObj()
{
}

bool GameObj::Initialize(NetConnection* conn)
{
    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(conn, "conn is null", false);

    _id.IncSalt();
    _conn = conn;
    return true;
}

// Called when game object is expired
void GameObj::Reset()
{
}

// called by network thead
void GameObj::RecvPacket(MemoryBlock* packet)
{
    _recvPackets.push(packet);

    ::PostQueuedCompletionStatus(_comPort, 0, (ULONG_PTR)this, NULL);
}

// called by business logic thread
void GameObj::OnRecvPacket()
{
}

void GameObj::Send(char* data, uint16 dataLen)
{
    if (_conn)
        _conn->Send(data, dataLen);
}

} //namespace RefLib
