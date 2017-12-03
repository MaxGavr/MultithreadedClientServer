#include "common.h"
#include "TcpServer.h"
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    WORD winsockVersion = MAKEWORD(WINSOCK_MAJOR_VER, WINSOCK_MINOR_VER);

    if (int result = WSAStartup(winsockVersion, &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed with error: " << result << std::endl;
        return 1;
    }

    TcpServer server;
    server.start();

    WSACleanup();
    return 0;
}