#include "TcpServer.h"
#include "ClientConnection.h"
#include "common.h"
#include <WS2tcpip.h>
#include <iostream>
#include <sstream>
#include <thread>


TcpServer::TcpServer()
    : m_Socket(INVALID_SOCKET)
{
    LOG4CPP_DEBUG_SD() << "Creating server socket";
    m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_Socket == INVALID_SOCKET)
        LOG4CPP_ERROR_SD() << "Can not create server socket. Error: " << WSAGetLastError();
}

TcpServer::~TcpServer()
{
}

void TcpServer::start()
{
    // let the server select it's host address

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

    if (!bindSocket())
        return;

    do
    {
        LOG4CPP_DEBUG_SD() << "Listening to pending connections";

        int result = listen(m_Socket, SOMAXCONN);
        if (result == SOCKET_ERROR)
        {
            LOG4CPP_ERROR_SD() << "Listening failed. Error code: " << WSAGetLastError();
            continue;
        }

        SOCKET clientSocket = INVALID_SOCKET;
        struct sockaddr clientAddress;
        int addressSize = sizeof(clientAddress);

        LOG4CPP_DEBUG_SD() << "Accepting new connection";

        clientSocket = accept(m_Socket, &clientAddress, &addressSize);
        if (clientSocket == INVALID_SOCKET)
        {
            LOG4CPP_ERROR_SD() << "Accepting failed. Error code: " << WSAGetLastError();
            continue;
        }

        handleConnection(clientSocket, clientAddress);

    } while (true);
}

void TcpServer::handleConnection(SOCKET clientSocket, sockaddr clientAddress)
{
    sockaddr_in* clientAddress_in = (sockaddr_in*)&clientAddress;

    ClientConnection* connection = new ClientConnection(this, clientSocket, *clientAddress_in);

    LOG4CPP_DEBUG_SD() << "Handling new connection with address " << connection->getClientAddress();

    std::thread connectionThread(&ClientConnection::handleClient, connection);
    connectionThread.detach();
}

void TcpServer::addMessage(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_MessagePool.push_back(msg);
    std::cout << msg << std::endl;
}

bool TcpServer::bindSocket()
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(IP_PORT);
    address.sin_addr.S_un.S_addr = inet_addr(IP_LOCAL_ADDRESS);

    LOG4CPP_DEBUG_SD() << "Binding socket to address: " << IP_LOCAL_ADDRESS << ":" << IP_PORT;

    int result = bind(m_Socket, (sockaddr*)&address, sizeof(address));
    if (result == SOCKET_ERROR)
    {
        LOG4CPP_ERROR_SD() << "Binding socket to host " << IP_LOCAL_ADDRESS << ":" << IP_PORT
            << " failed. Error code: " << WSAGetLastError();
        return false;
    }

    return true;
}
