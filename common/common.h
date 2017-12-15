#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>

const int WINSOCK_MINOR_VER = 2;
const int WINSOCK_MAJOR_VER = 2;

const char IP_LOCAL_ADDRESS[] = "127.0.0.1";
const unsigned IP_PORT = 5233;

const unsigned BUF_SIZE = 512;