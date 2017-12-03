#include "common.h"
#include <WinSock2.h>
#include <iostream>
#include "TcpClient.h"

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Host IP address has not been specified" << std::endl;
        return 1;
    }

    WSADATA wsaData;
    WORD winsockVersion = MAKEWORD(WINSOCK_MAJOR_VER, WINSOCK_MINOR_VER);

    if (int result = WSAStartup(winsockVersion, &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed with error: " << result << std::endl;
        return 1;
    }

    TcpClient client;
    client.connectToServer(IP_LOCAL_ADDRESS, IP_PORT);
    client.sleep();
    client.sendMessage();
    client.sleep();
    client.disconnect();

    WSACleanup();
    return 0;
}