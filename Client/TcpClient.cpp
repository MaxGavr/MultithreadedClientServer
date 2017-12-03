#include "TcpClient.h"
#include "common.h"
#include <iostream>
#include <sstream>


TcpClient::TcpClient()
    : m_Socket(INVALID_SOCKET)
{
    m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_Socket == INVALID_SOCKET)
        std::cerr << "Can't create client socket. Error code: " << WSAGetLastError() << std::endl;
}

TcpClient::~TcpClient()
{
}

void TcpClient::connectToServer(const char* ipAddress, const u_short ipPort)
{
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_addr.S_un.S_addr = inet_addr(ipAddress);
    hint.sin_port = htons(ipPort);

    int result = connect(m_Socket, (sockaddr*) &hint, sizeof(hint));
    if (result == SOCKET_ERROR)
    {
        std::cerr << "Can not connect to host: " << ipAddress << ":" << ipPort << std::endl;
        std::cerr << "Error: " << WSAGetLastError();

        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
    }
}

void TcpClient::sendMessage()
{
    sendData(m_Message);
    receiveData();
}

void TcpClient::sendData(const char* buf)
{
    std::cout << "Sending data" << std::endl;
    int result = send(m_Socket, buf, strlen(buf), 0);
    if (result == SOCKET_ERROR)
        std::cerr << "Sending message failed. Error: " << WSAGetLastError() << std::endl;
}

void TcpClient::receiveData()
{
    std::cout << "Receiving data" << std::endl;
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
            std::cerr << "Receive failed. Error: " << WSAGetLastError() << std::endl;

    } while (bytesReceived > 0);

    std::cout << serverResponse.str();
}

void TcpClient::disconnect()
{
    int result = closesocket(m_Socket);
    if (result == SOCKET_ERROR)
        std::cerr << "Socket closing error: " << WSAGetLastError() << std::endl;
}

void TcpClient::sleep()
{
    int seconds = rand() % (10 - 1) + 2;
    Sleep(seconds * 1e3);
}
