#pragma once
#include "TcpServer.h"

class ClientConnection
{
public:
    ClientConnection(TcpServer* server, SOCKET clientSocket, sockaddr_in clientAddress);
    ~ClientConnection();

    void handleClient();

    std::string getClientAddress() const;

private:
    std::string getThreadId() const;

    std::string receiveClientMessage();

    TcpServer* m_Server;

    SOCKET m_Socket;
    sockaddr_in m_ClientAddress;
};

