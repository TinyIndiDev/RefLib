# RefLib
PURPOSE:
- This network library is designed for indie game developer.

OVERVIEW:
- This library is simple but very close to the skeleton libraries used in the commercial game.
- NetService manages network connections and one of listener and connector.
- NetService has two groups of threads. One is for network and the other is for receive function of game object which is define by application programmer.

![](https://github.com/goopymoon/RefLib/blob/master/Doc/RefLib.jpg)

USAGE:
- To communicate you only have to initialize NetService and define a class which inherits NetObj class with overrided OnConnected(), OnDisconnected(), OnRecvPacket() and call Send().
- IocpClient and IocpServer example shows how to make echo server and client.

LIMITATION:
- It's not fully tested, since this code is not used under commercial environment.
