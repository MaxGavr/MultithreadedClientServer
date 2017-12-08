#include "ClientConnection.h"
#include "common.h"
#include <sstream>



ClientConnection::ClientConnection(TcpServer* server, SOCKET clientSocket, sockaddr_in clientAddress)
    : m_Server(server),
      m_Socket(clientSocket),
      m_ClientAddress(clientAddress)
{
}

ClientConnection::~ClientConnection()
{
}

void ClientConnection::handleClient()
{
    // post initial message
    std::stringstream message;
    message << getThreadId() << " Client " << getClientAddress() << " connected.";
    m_Server->addMessage(message.str());

    // enable nonblocking mode
    bool enable = true;
    ioctlsocket(m_Socket, FIONBIO, (u_long*)&enable);

    std::string clientMessage = receiveClientMessage();
    //sendMessage(clientMessage);
}

std::string ClientConnection::receiveClientMessage()
{
    std::stringstream clientMessage;
    char buf[BUF_SIZE] = "";

    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    fd_set readSet;

    int bytesReceived = 0;
    int result = 0;
    do
    {
        FD_ZERO(&readSet);
        FD_SET(m_Socket, &readSet);

        result = select(0, &readSet, NULL, NULL, &timeout);
        if (result == SOCKET_ERROR)
        {
            LOG4CPP_ERROR_SD() << "Select failed. Error code: " << WSAGetLastError();
            return clientMessage.str();
        }
        else if (result > 0 && FD_ISSET(m_Socket, &readSet))
        {
            bytesReceived = recv(m_Socket, buf, BUF_SIZE, 0);

            // received client data
            if (bytesReceived > 0)
            {
                //LOG4CPP_DEBUG_SD() << "Receive " << bytesReceived << " bytes from " << getClientAddress();

                buf[bytesReceived] = '\0';
                clientMessage << buf;
            }
            // receiving completed
            else if (bytesReceived == 0)
            {
                break;
            }
            else if (bytesReceived == SOCKET_ERROR)
            {
                // will receive later
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    LOG4CPP_ERROR_SD() << "Receive failed. Error code: " << WSAGetLastError();
                    break;
                }
            }
        }
    } while (true);

    m_Server->addMessage(clientMessage.str());
    return clientMessage.str();
}

std::string ClientConnection::getClientAddress() const
{
    std::stringstream address;
    address << inet_ntoa(m_ClientAddress.sin_addr) << ":" << ntohs(m_ClientAddress.sin_port);
    return address.str();
}

std::string ClientConnection::getThreadId() const
{
    std::stringstream id;
    id << "[" << std::this_thread::get_id << "]";
    return id.str();
}
