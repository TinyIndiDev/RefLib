#include "stdafx.h"

#include <sstream>
#include "reflib_net_util.h"

std::string SocketGetLastErrorString()
{
    return SocketGetErrorString(WSAGetLastError());
}

std::string SocketGetErrorString(int code)
{
    switch (code)
    {
    case WSAEADDRNOTAVAIL:			return "[Address not available]";
    case WSAEADDRINUSE:				return "[Address is in use]";
    case WSANOTINITIALISED:			return "[Winsock not initialized]";
    case WSAENETDOWN:				return "[Network is down]";
    case WSAEINPROGRESS:			return "[Operation in progress]";
    case WSA_NOT_ENOUGH_MEMORY:		return "[Out of memory]";
    case WSA_INVALID_HANDLE:		return "[Invalid handle (programming error)]";
    case WSA_INVALID_PARAMETER:		return "[Invalid parameter (programming error)]";
    case WSAEFAULT:					return "[Fault]";
    case WSAEINTR:					return "[WSAEINTR]";
    case WSAEINVAL:					return "[WSAEINVAL]";
    case WSAEISCONN:				return "[WSAEISCONN]";
    case WSAENETRESET:				return "[Network reset]";
    case WSAENOTSOCK:				return "[Parameter is not a socket (programming error)]";
    case WSAEOPNOTSUPP:				return "[Operation not supported (programming error)]";
    case WSAESOCKTNOSUPPORT:		return "[Socket type not supported]";
    case WSAESHUTDOWN:				return "[WSAESHUTDOWN]";
    case WSAEWOULDBLOCK:			return "[Operation would block (programming error)]";
    case WSAEMSGSIZE:				return "[WSAEMSGSIZE]";
    case WSAETIMEDOUT:				return "[Operation timed out]";
    case WSAECONNRESET:				return "[Connection reset (programming error)]";
    case WSAENOTCONN:				return "[Socket not connected (programming error)]";
    case WSAEDISCON:				return "[WSAEDISCON]";
    case ERROR_IO_PENDING:			return "[IO operation will complete in IOCP worker thread (programming error)]";
    case WSA_OPERATION_ABORTED:		return "[WSA_OPERATION_ABORTED]";
    case ERROR_CONNECTION_ABORTED:	return "[Connection aborted locally (programming error)]";
    case ERROR_NETNAME_DELETED:		return "[Socket was already closed (programming error)]";
    case ERROR_PORT_UNREACHABLE:	return "[Destination port is unreachable]";
    case ERROR_MORE_DATA:			return "[More data is available]";
    };

    std::ostringstream oss;
    oss << "[Error code: " << code << " (0x" << std::hex << code << ")]";
    return oss.str();
}

