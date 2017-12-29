#pragma once
#include "common.h"


#include "log4cpp\Category.hh"
#include "log4cpp\convenience.h"

LOG4CPP_LOGGER_N(fLog, "file")
LOG4CPP_LOGGER_N(cLog, "console")


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

    std::string generateRandomString(size_t length);

private:
    SOCKET m_Socket;
};

