#pragma once

#include "common.h"

#include <mutex>
#include <vector>
#include <map>
#include <csignal>

#include "log4cpp\Category.hh"
#include "log4cpp\convenience.h"

LOG4CPP_LOGGER_N(fLog, "file")
LOG4CPP_LOGGER_N(cLog, "console")


struct ClientSocketInfo
{
    std::stringstream message;

    bool messageReceived = false;
    bool messageSent = false;
    bool disconnected = false;

    int sendPosition = 0;
};


class TcpServer
{
public:
    TcpServer();
    ~TcpServer();

    static std::string getClientAddress(const sockaddr* sockaddr);
    static std::string getClientAddress(SOCKET clientSocket);
    std::string getServerAddress() const;

    std::string getCurrentTime(bool withMilliseconds = true) const;

    bool start(const char* ipAddress, const u_short ipPort);

    void addMessage(const std::string& msg);

    static void handleInterruption(int signum);

private:
    bool bindSocket(const char* ipAddress, const u_short ipPort);

    void handleConnectedClients();

    void acceptNewClient();
    void disconnectClient(SOCKET clientSocket);
    
    void receiveClientData(SOCKET clientSocket);
    void sendClientData(SOCKET clientSocket);

    void fdSetAllSockets(fd_set& set) const;

    std::string getThreadId() const;

    void timer();

    void dumpLog();

private:
    static TcpServer* m_ServerInstance;
    bool m_isRunning;
    std::thread::id m_TimerThreadId;

    SOCKET m_Socket;

    std::mutex m_Mutex;
    std::mutex m_TimerMutex;
    std::vector<std::string> m_MessageBuffer;

    std::map<SOCKET, ClientSocketInfo> m_Clients;
};
