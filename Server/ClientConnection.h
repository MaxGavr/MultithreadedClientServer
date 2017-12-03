#pragma once
#include "TcpServer.h"

class ClientConnection
{
public:
    ClientConnection(TcpServer* server);
    ~ClientConnection();

    void handleClient(SOCKET clientSocket, sockaddr_in clientAddress);

private:
    TcpServer* m_Server;

    SOCKET m_Socket;
};

