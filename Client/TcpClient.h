#pragma once
#include <WinSock2.h>

#include "log4cpp\Category.hh"
#include "log4cpp\convenience.h"

LOG4CPP_LOGGER("")

class TcpClient
{
public:
    TcpClient();
    ~TcpClient();

    bool connectToServer(const char* ipAddress, const u_short ipPort);
    void exchangeData();
    void sendData(const char* buf);
    void receiveData();

    void disconnect();
    
    void sleep();

private:
    SOCKET m_Socket;

    const char* m_Message = "Totally random string!";
};

