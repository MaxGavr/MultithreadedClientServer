#include "ClientConnection.h"
#include <sstream>



ClientConnection::ClientConnection(TcpServer* server)
    : m_Server(server)
{
}

ClientConnection::~ClientConnection()
{
}

void ClientConnection::handleClient(SOCKET clientSocket, sockaddr_in clientAddress)
{
    m_Socket = clientSocket;

    std::stringstream message;
    message << "[" << std::this_thread::get_id << "]" << "Client " << inet_ntoa(clientAddress.sin_addr) << " connected." << std::endl;
    m_Server->addMessage(message.str());
}
