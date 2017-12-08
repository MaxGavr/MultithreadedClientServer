#pragma once

#include <WinSock2.h>
#include <mutex>
#include <vector>

#include "log4cpp\Category.hh"
#include "log4cpp\convenience.h"

LOG4CPP_LOGGER("")

class TcpServer
{
public:
    TcpServer();
    ~TcpServer();

    void start();
    void handleConnection(SOCKET clientSocket, sockaddr clientAddress);

    void addMessage(const std::string&  msg);

private:
    bool bindSocket();

    SOCKET m_Socket;

    std::mutex m_Mutex;
    std::vector<std::string> m_MessagePool;
};

