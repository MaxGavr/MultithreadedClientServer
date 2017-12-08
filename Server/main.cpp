#include "common.h"
#include "TcpServer.h"

#include "log4cpp\PropertyConfigurator.hh"

#pragma comment(lib, "Ws2_32.lib")

bool configureLogger()
{
    try
    {
        log4cpp::PropertyConfigurator::configure("log4cpp_server.properties");
        return true;
    }
    catch (const log4cpp::ConfigureFailure& e)
    {
        std::cerr << e.what() << std::endl;

        log4cpp::Category::shutdown();
        return false;
    }
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
    if (!configureLogger())
        return 1;

    if (!initializeWinSock())
        return 1;

    TcpServer server;
    server.start();

    WSACleanup();
    log4cpp::Category::shutdown();
    return 0;
}