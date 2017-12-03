#pragma once

#include <WinSock2.h>
#include <mutex>
#include <vector>
#include <string>

class TcpServer
{
public:
    TcpServer();
    ~TcpServer();

    void start();

    void sendData(const char* buf);
    void receiveData(SOCKET sock);

    void addMessage(std::string msg);

private:
    SOCKET m_Socket;

    std::mutex m_Mutex;
    std::vector<std::string> m_Log;
};

