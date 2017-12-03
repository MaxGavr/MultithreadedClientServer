#include "TcpServer.h"
#include "common.h"
#include <WS2tcpip.h>
#include <iostream>
#include <sstream>
#include <thread>


TcpServer::TcpServer()
    : m_Socket(INVALID_SOCKET)
{
    m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_Socket == INVALID_SOCKET)
        std::cerr << "Can not create server socket. Error: " << WSAGetLastError() << std::endl;
}

TcpServer::~TcpServer()
{
}

void TcpServer::start()
{
    /*
    struct addrinfo* serverAddressInfo = NULL;

    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int res = getaddrinfo(NULL, IP_PORT_STR, &hints, &serverAddressInfo);
    if (res != 0)
        std::cerr << "getaddrinfo failed. Error: " << WSAGetLastError() << std::endl;
    */

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(IP_PORT);
    address.sin_addr.S_un.S_addr = inet_addr(IP_LOCAL_ADDRESS);

    int result = bind(m_Socket, (sockaddr*)&address, sizeof(address));
    if (result == SOCKET_ERROR)
        std::cerr << "Can not bind server socket. Error: " << WSAGetLastError() << std::endl;

    do
    {
        result = listen(m_Socket, SOMAXCONN);
        if (result == SOCKET_ERROR)
            std::cerr << "Listening failed. Error: " << WSAGetLastError() << std::endl;

        SOCKET clientSocket = INVALID_SOCKET;
        struct sockaddr clientAddress;
        int addressSize = sizeof(clientAddress);
        clientSocket = accept(m_Socket, &clientAddress, &addressSize);
        if (clientSocket == INVALID_SOCKET)
            std::cerr << "Accepting failed. Error: " << WSAGetLastError() << std::endl;
    } while ()
}

void TcpServer::receiveData(SOCKET sock)
{
    std::stringstream clientMessage;

    char buf[BUF_SIZE] = "";

    int bytesReceived = 0;
    do
    {
        bytesReceived = recv(sock, buf, BUF_SIZE, 0);
        std::cout << "Bytes received: " << bytesReceived << std::endl;
        if (bytesReceived > 0)
        {
            buf[bytesReceived] = '\0';
            clientMessage << buf;
        }
        else if (bytesReceived < 0)
            std::cerr << "Receive failed. Error: " << WSAGetLastError() << std::endl;

    } while (bytesReceived > 0);

    std::cout << clientMessage.str();
}

void TcpServer::addMessage(std::string msg)
{
    m_Mutex.lock();
    m_Log.push_back(msg);
    m_Mutex.unlock();
}
