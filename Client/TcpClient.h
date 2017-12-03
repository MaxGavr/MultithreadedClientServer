#pragma once
#include <WinSock2.h>


class TcpClient
{
public:
    TcpClient();
    ~TcpClient();

    void connectToServer(const char* ipAddress, const u_short ipPort);
    void sendMessage();
    void sendData(const char* buf);
    void receiveData();

    void disconnect();
    
    void sleep();

private:
    SOCKET m_Socket;

    const char* m_Message = "Totally random string!";
};

