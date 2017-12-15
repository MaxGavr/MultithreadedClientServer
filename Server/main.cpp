#include "TcpServer.h"

#include "log4cpp\PropertyConfigurator.hh"

#pragma comment(lib, "Ws2_32.lib")

bool configureLogger()
{
    try
    {
        log4cpp::PropertyConfigurator::configure("log4cpp_server.properties");
    }
    catch (const log4cpp::ConfigureFailure& e)
    {
        std::cerr << e.what() << std::endl;

        log4cpp::Category::shutdown();
        return false;
    }

    LOG4CPP_DEBUG_S(fLog) << "###---Logger initialized---####################################";
}

bool initializeWinSock()
{
    WSADATA wsaData;
    WORD winsockVersion = MAKEWORD(WINSOCK_MAJOR_VER, WINSOCK_MINOR_VER);

    if (int result = WSAStartup(winsockVersion, &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed with error: " << result << std::endl;
        log4cpp::Category::shutdown();
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Host IP address has not been specified!" << std::endl;
        return 1;
    }

    if (!configureLogger())
        return 1;

    if (!initializeWinSock())
        return 1;

    if (signal(SIGINT, TcpServer::handleInterruption) == SIG_ERR)
        LOG4CPP_ERROR_S(fLog) << "Registering custom SIGINT handler failed";

    TcpServer server;
    server.start(argv[1], IP_PORT);

    WSACleanup();
    log4cpp::Category::shutdown();

    return 0;
}