#include "TcpServer.h"

#include <thread>
#include <fstream>
#include <sys\timeb.h>

TcpServer* TcpServer::m_ServerInstance = NULL;

TcpServer::TcpServer()
    : m_Socket(INVALID_SOCKET)
{
    m_ServerInstance = this;

    LOG4CPP_DEBUG_S(fLog) << "Creating server socket";
    m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_Socket == INVALID_SOCKET)
        LOG4CPP_ERROR_S(fLog) << "Can not create server socket. Error code: " << WSAGetLastError();
}

TcpServer::~TcpServer()
{
}

std::string TcpServer::getClientAddress(const sockaddr* sockaddr)
{
    const sockaddr_in* sock_in = (const sockaddr_in*)sockaddr;

    std::stringstream address;
    address << inet_ntoa(sock_in->sin_addr) << ":" << ntohs(sock_in->sin_port);
    return address.str();
}

std::string TcpServer::getClientAddress(SOCKET clientSocket)
{
    sockaddr clientAddress;
    int addressSize = sizeof(clientAddress);

    int result = getpeername(clientSocket, &clientAddress, &addressSize);
    if (result == SOCKET_ERROR)
    {
        // TODO: throw exception
        return std::string();
    }

    return getClientAddress(&clientAddress);
}

std::string TcpServer::getServerAddress() const
{
    sockaddr address;
    int addressSize = sizeof(address);

    int result = getsockname(m_Socket, &address, &addressSize);
    if (result == SOCKET_ERROR)
    {
        // TODO: throw exception
        return std::string();
    }

    return getClientAddress(&address);
}


std::string TcpServer::getCurrentTime(bool withMilliseconds) const
{
    time_t t = time(NULL);
    tm* t_m = localtime(&t);

    char time_s[64] = "";
    strftime(time_s, 64, "%d-%m-%Y %H:%M:%S", t_m);

    timeb tb;
    ::ftime(&tb);

    std::string timeStr(time_s);
    if (withMilliseconds)
        timeStr += std::to_string(tb.millitm);

    return timeStr;
}


std::string TcpServer::getThreadId() const
{
    std::stringstream id;
    id << "[" << std::this_thread::get_id << "]";
    return id.str();
}

void TcpServer::timer()
{
    while (m_isRunning)
    {
        addMessage("idle");
        Sleep(1e3);
    }
}

void TcpServer::handleInterruption(int signum)
{
    LOG4CPP_DEBUG_S(fLog) << "Catch SIGINT signal";

    m_ServerInstance->dumpLog();
}

void TcpServer::dumpLog()
{
    LOG4CPP_DEBUG_S(fLog) << "Dumping message log";

    CreateDirectory(L"tmp", NULL);

    std::stringstream dumpFileName;
    dumpFileName << ".\\tmp\\log_";

    time_t t_t = time(NULL);
    tm* t_m = localtime(&t_t);

    char time_s[64] = "";
    strftime(time_s, 64, "%d-%m-%Y_%H-%M-%S", t_m);

    dumpFileName << time_s << ".log";

    std::ofstream file;
    file.open(dumpFileName.str());

    std::lock_guard<std::mutex> lock(m_Mutex);

    for (std::string message : m_MessageBuffer)
        file << message << std::endl;

    file.close();

    std::vector<std::string>().swap(m_MessageBuffer);

    //system("cls");
    LOG4CPP_INFO_S(cLog) << "Dump file has been created: " << dumpFileName.str();
    LOG4CPP_DEBUG_S(fLog) << "Dump file has been created" << dumpFileName.str();

    if (signal(SIGINT, TcpServer::handleInterruption) == SIG_ERR)
        LOG4CPP_ERROR_S(fLog) << "Registering custom SIGINT handler failed";
}

bool TcpServer::start(const char* ipAddress, const u_short ipPort)
{
    if (!bindSocket(ipAddress, ipPort))
        return false;

    LOG4CPP_DEBUG_S(fLog) << "Waiting for pending connections";

    int result = listen(m_Socket, SOMAXCONN);
    if (result == SOCKET_ERROR)
    {
        LOG4CPP_ERROR_S(fLog) << "Listening failed. Error code: " << WSAGetLastError();
        return false;
    }

    std::stringstream startMessage;
    startMessage << "Server is running on " << getServerAddress();
    addMessage(startMessage.str());

    // TODO: find right place
    m_isRunning = true;

    std::thread timerThread(&TcpServer::timer, this);
    timerThread.detach();

    do
    {
        fd_set readSet;
        FD_ZERO(&readSet);

        FD_SET(m_Socket, &readSet);

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        result = select(0, &readSet, NULL, NULL, &timeout);
        if (result == SOCKET_ERROR)
        {
            LOG4CPP_ERROR_S(fLog) << "Select error while accepting new clients. Error code: " << WSAGetLastError();
            continue;
        }
        else if (result > 0 && FD_ISSET(m_Socket, &readSet))
        {
            acceptNewClient();
        }

        handleConnectedClients();

    } while (true);

    LOG4CPP_DEBUG_S(fLog) << "Stopping server";

    m_isRunning = false;
    result = closesocket(m_Socket);
    if (result == SOCKET_ERROR)
    {
        LOG4CPP_ERROR_S(fLog) << "Server stopping failed. Error code: " << WSAGetLastError();
        return false;
    }
    LOG4CPP_INFO_S(cLog) << "Server stopped";

    return true;
}

void TcpServer::acceptNewClient()
{
    SOCKET clientSocket = INVALID_SOCKET;
    sockaddr clientAddress;
    int addressSize = sizeof(clientAddress);

    LOG4CPP_DEBUG_S(fLog) << "Accepting new pending connection";

    clientSocket = accept(m_Socket, &clientAddress, &addressSize);
    if (clientSocket == INVALID_SOCKET)
    {
        LOG4CPP_ERROR_S(fLog) << "Accepting new connection failed. Error code: " << WSAGetLastError();
        return;
    }

    // post initial message
    std::stringstream message;
    message << "Client " << getClientAddress(&clientAddress) << " connected.";
    addMessage(message.str());
    LOG4CPP_DEBUG_S(fLog) << message.str();

    // enable nonblocking mode
    bool enable = true;
    ioctlsocket(clientSocket, FIONBIO, (u_long*)&enable);

    m_Clients.insert({ clientSocket, ClientSocketInfo() });
}

void TcpServer::receiveClientData(SOCKET clientSocket)
{
    char buf[BUF_SIZE] = "";

    //LOG4CPP_DEBUG_S(fLog) << "Receiving data from " << getClientAddress(clientSocket);

    int bytesReceived = recv(clientSocket, buf, BUF_SIZE, 0);

    if (bytesReceived > 0) 
    {
        LOG4CPP_DEBUG_S(fLog) << "Receive " << bytesReceived << " bytes from " << getClientAddress(clientSocket);

        ClientSocketInfo& client = m_Clients.at(clientSocket);

        // TODO: possible overflow
        buf[bytesReceived] = '\0';
        client.message << buf;

        if (buf[bytesReceived - 1] == '\0')
        {
            client.messageReceived = true;

            std::stringstream logMsg;
            logMsg << "Received message from " << getClientAddress(clientSocket)
                   << ": \"" << client.message.str() << "\"";

            addMessage(logMsg.str());
            LOG4CPP_DEBUG_S(fLog) << logMsg.str();
        }
    }
    else if (bytesReceived == 0) // client socket has been closed
    {
        disconnectClient(clientSocket);
    }
    else if (bytesReceived == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            LOG4CPP_ERROR_S(fLog) << "Receive failed. Error code: " << WSAGetLastError();
            // TODO: return false?
            return;
        }
    }
}

void TcpServer::sendClientData(SOCKET clientSocket)
{

    LOG4CPP_DEBUG_S(fLog) << "Sending data to " << getClientAddress(clientSocket);

    ClientSocketInfo& client = m_Clients.at(clientSocket);

    std::string messageStr = client.message.str();

    const char* messageBuf = messageStr.c_str();
    const int bytesLeftToSent = client.message.str().size() - client.sendPosition;

    int bytesSent = send(clientSocket, messageBuf + client.sendPosition, bytesLeftToSent, 0);
    if (bytesSent == SOCKET_ERROR)
    {
        LOG4CPP_ERROR_S(fLog) << "Send failed. Error code: " << WSAGetLastError();
        return;
    }

    // TODO: replace with = (>?)
    if (client.sendPosition + bytesSent >= client.message.str().size())
    {
        client.messageSent = true;
        shutdown(clientSocket, SD_SEND);
        LOG4CPP_DEBUG_S(fLog) << "Sending message to " << getClientAddress(clientSocket) << " completed";
    }
    else
    {
        client.sendPosition += bytesSent;
        LOG4CPP_DEBUG_S(fLog) << "Send " << bytesSent << " to " << getClientAddress(clientSocket);
    }
}

void TcpServer::disconnectClient(SOCKET clientSocket)
{
    std::string clientAddress = getClientAddress(clientSocket);

    int result = closesocket(clientSocket);
    if (result == SOCKET_ERROR)
    {
        LOG4CPP_ERROR_S(fLog) << "Closing client socket failed. Error code: " << WSAGetLastError();
        return;
    }
    LOG4CPP_DEBUG_S(fLog) << "Client " << clientAddress << " disconnected";

    m_Clients.at(clientSocket).disconnected = true;

    std::stringstream message;
    message << " Client " << clientAddress << " disconnected";
    addMessage(message.str());
}

void TcpServer::fdSetAllSockets(fd_set& set) const
{
    FD_ZERO(&set);
    for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
        FD_SET(it->first, &set);
}

void TcpServer::addMessage(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    LOG4CPP_INFO_S(cLog) << getThreadId() << " " << msg;

    std::stringstream formattedMessage;
    formattedMessage << getCurrentTime() << " " << getThreadId() << " " << msg;
    m_MessageBuffer.push_back(formattedMessage.str());
}

bool TcpServer::bindSocket(const char* ipAddress, const u_short ipPort)
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(ipPort);
    address.sin_addr.S_un.S_addr = inet_addr(ipAddress);

    LOG4CPP_DEBUG_S(fLog) << "Binding socket to address: " << ipAddress << ":" << ipPort;

    int result = bind(m_Socket, (sockaddr*)&address, sizeof(address));
    if (result == SOCKET_ERROR)
    {
        LOG4CPP_ERROR_S(fLog) << "Binding socket to host " << ipAddress << ":" << ipPort
            << " failed. Error code: " << WSAGetLastError();
        return false;
    }

    return true;
}

void TcpServer::handleConnectedClients()
{
    if (m_Clients.empty())
        return;

    fd_set readSet, writeSet;

    // TODO: filter sockets
    fdSetAllSockets(readSet);
    fdSetAllSockets(writeSet);

    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int result = select(0, &readSet, &writeSet, NULL, &timeout);

    if (result == SOCKET_ERROR)
    {
        LOG4CPP_ERROR_S(fLog) << "Selecting sockets for read and write error. Error code: " << WSAGetLastError();
        return;
    }
    else if (result > 0)
    {
        for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
        {
            SOCKET sock = it->first;
            const ClientSocketInfo& client = it->second;

            if (FD_ISSET(sock, &readSet))
                receiveClientData(sock);
            else if (FD_ISSET(sock, &writeSet) && client.messageReceived && !client.messageSent)
                sendClientData(sock);
        }

        // remove disconnected clients
        for (auto it = m_Clients.begin(); it != m_Clients.end(); )
        {
            if (it->second.disconnected)
                it = m_Clients.erase(it);
            else
                ++it;
        }
    }
}
