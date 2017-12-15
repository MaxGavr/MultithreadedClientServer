#include "TcpClient.h"
#include "common.h"
#include <iostream>
#include <sstream>
#include <algorithm>


TcpClient::TcpClient()
    : m_Socket(INVALID_SOCKET)
{
    LOG4CPP_DEBUG_S(fLog) << "Creating client socket";

    m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_Socket == INVALID_SOCKET)
        LOG4CPP_ERROR_S(fLog) << "Can't create client socket. Error code: " << WSAGetLastError();
}

TcpClient::~TcpClient()
{
}

bool TcpClient::connectToServer(const char* ipAddress, const u_short ipPort)
{
    LOG4CPP_DEBUG_S(fLog) << "Connecting to " << ipAddress << ":" << ipPort;

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_addr.S_un.S_addr = inet_addr(ipAddress);
    hint.sin_port = htons(ipPort);

    int result = connect(m_Socket, (sockaddr*) &hint, sizeof(hint));
    if (result == SOCKET_ERROR)
    {
        LOG4CPP_ERROR_S(fLog) << "Can not connect to host: " << ipAddress << ":" << ipPort << ". Error code: " << WSAGetLastError();

        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;

        return false;
    }

    LOG4CPP_INFO_S(cLog) << "Connected to " << ipAddress << ":" << ipPort;
    LOG4CPP_DEBUG_S(fLog) << "Connection successfull";
    return true;
}

void TcpClient::exchangeData()
{
    sendData(generateRandomString(32).c_str());
    receiveData();
}

void TcpClient::sendData(const char* buf)
{
    LOG4CPP_INFO_S(cLog) << "Sending \"" << buf << "\" to host";
    LOG4CPP_DEBUG_S(fLog) << "Sending data to server: " << buf;

    int result = send(m_Socket, buf, strlen(buf) + 1, 0);
    if (result == SOCKET_ERROR)
        LOG4CPP_ERROR_S(fLog) << "Sending failed. Error code: " << WSAGetLastError();

    LOG4CPP_DEBUG_S(fLog) << "Data was sent successfully";
}

void TcpClient::receiveData()
{
    LOG4CPP_INFO_S(cLog) << "Waiting for host response";
    LOG4CPP_DEBUG_S(fLog) << "Receiving data";

    std::stringstream serverResponse;
    char buf[BUF_SIZE] = "";

    int bytesReceived = 0;
    do
    {
        bytesReceived = recv(m_Socket, buf, BUF_SIZE, 0);
        if (bytesReceived > 0)
        {
            buf[bytesReceived] = '\0';
            serverResponse << buf;
        }
        else if (bytesReceived < 0)
            LOG4CPP_ERROR_S(fLog) << "Data receiving failed. Error code: " << WSAGetLastError();

    } while (bytesReceived > 0);

    LOG4CPP_INFO_S(cLog) << "Received \"" << serverResponse.str() << "\" from host";
}

void TcpClient::disconnect()
{
    LOG4CPP_INFO_S(cLog) << "Disconnecting from host";
    LOG4CPP_DEBUG_S(fLog) << "Disconnecting from host";

    int result = closesocket(m_Socket);
    if (result == SOCKET_ERROR)
        LOG4CPP_DEBUG_S(fLog) << "Socket closing error. Error code: " << WSAGetLastError();

    LOG4CPP_DEBUG_S(fLog) << "Disconnection successfull";
}

void TcpClient::sleep()
{
    const int seconds = rand() % (10 - 1) + 2;

    LOG4CPP_INFO_S(cLog) << "Sleeping for " << seconds << " seconds";
    LOG4CPP_DEBUG_S(fLog) << "Sleeping for " << seconds << " seconds";

    Sleep((DWORD)(seconds * 1e3));
}

std::string TcpClient::generateRandomString(size_t length)
{
    auto randchar = []() -> char
    {
        const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const size_t maxIndex = (sizeof(charset) - 1);
        return charset[rand() % maxIndex];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}
