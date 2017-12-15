#include "TcpClient.h"
#include <ctime>

#include "log4cpp\PropertyConfigurator.hh"

#pragma comment(lib, "Ws2_32.lib")


bool configureLogger()
{
    try
    {
        log4cpp::PropertyConfigurator::configure("log4cpp_client.properties");
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
    srand(time(NULL));

    if (argc < 2)
    {
        std::cerr << "Host IP address has not been specified!" << std::endl;
        return 1;
    }

    if (!configureLogger())
        return 1;

    if (!initializeWinSock())
        return 1;

    TcpClient client;
    if (client.connectToServer(argv[1], IP_PORT))
    {
        client.sleep();
        client.exchangeData();
        client.sleep();
        client.disconnect();
    }

    WSACleanup();

    log4cpp::Category::shutdown();
    
    return 0;
}