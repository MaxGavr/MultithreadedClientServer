#include "TcpClient.h"
#include "common.h"
#include <iostream>
#include <sstream>


TcpClient::TcpClient()
    : m_Socket(INVALID_SOCKET)
{
    m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_Socket == INVALID_SOCKET)
        LOG4CPP_ERROR_SD() << "Can't create client socket. Error code: " << WSAGetLastError();

    LOG4CPP_DEBUG_SD() << "Client socket created successfully";
}

TcpClient::~TcpClient()
{
}

bool TcpClient::connectToServer(const char* ipAddress, const u_short ipPort)
{
    LOG4CPP_DEBUG_SD() << "Connecting to " << ipAddress << ":" << ipPort;

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_addr.S_un.S_addr = inet_addr(ipAddress);
    hint.sin_port = htons(ipPort);

    int result = connect(m_Socket, (sockaddr*) &hint, sizeof(hint));
    if (result == SOCKET_ERROR)
    {
        LOG4CPP_ERROR_SD() << "Can not connect to host: " << ipAddress << ":" << ipPort << ". Error code: " << WSAGetLastError();

        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;

        return false;
    }

    LOG4CPP_DEBUG_SD() << "Connection successfull";
    return true;
}

void TcpClient::exchangeData()
{
    sendData(m_Message);
    receiveData();
}

void TcpClient::sendData(const char* buf)
{
    LOG4CPP_DEBUG_SD() << "Sending data: " << buf;

    int result = send(m_Socket, buf, strlen(buf), 0);
    if (result == SOCKET_ERROR)
        LOG4CPP_ERROR_SD() << "Sending message failed. Error code: " << WSAGetLastError();

    LOG4CPP_DEBUG_SD() << "Data sending successfull";
}

void TcpClient::receiveData()
{
    LOG4CPP_DEBUG_SD() << "Receiving data";

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
            LOG4CPP_ERROR_SD() << "Data receiving failed. Error code: " << WSAGetLastError();

    } while (bytesReceived > 0);

    std::cout << serverResponse.str();
}

void TcpClient::disconnect()
{
    LOG4CPP_DEBUG_SD() << "Disconnecting from host";

    int result = closesocket(m_Socket);
    if (result == SOCKET_ERROR)
        LOG4CPP_DEBUG_SD() << "Socket closing error. Error code: " << WSAGetLastError();

    LOG4CPP_DEBUG_SD() << "Disconnection successfull";
}

void TcpClient::sleep()
{
    const int seconds = rand() % (10 - 1) + 2;
    Sleep(seconds * 1e3);
}
