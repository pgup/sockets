#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <cstdlib>

#if defined(WIN32)
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#elif defined(__linux__)
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define PORT_NUMBER 54321
#define DATA_SIZE 1024

using namespace std;

int main()
{
#if defined (WIN32)
    SOCKET server_sock;
    WSADATA WSAdata;

    int res = WSAStartup(MAKEWORD(2, 2), &WSAdata);
    if (res != 0)
    {
        cout << "couldn't start up WSA for windows sockets" << endl;
        exit(-1);
    }
#elif defined (__linux__)
    int server_sock;
#endif

    //create socket
    server_sock = socket(AF_INET, SOCK_DGRAM, 0);
#if defined(WIN32)
    if (server_sock == INVALID_SOCKET)
    {
        cout << "ERROR creating server socket" << endl;
        exit(-1);
    }
#elif defined(__linux__)
    if (server_sock < 0)
    {
        cout << "ERROR creating server socket" << endl;
        exit(-1);
    }
#endif

    sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUMBER);
    server_addr.sin_addr.s_addr = INADDR_ANY;


    //bind socket
    if (bind(server_sock, (sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        cout << "ERROR binding socket" << endl;


    socklen_t sock_len = sizeof(sockaddr_in);
    char *data_recv = new char[DATA_SIZE];
    int messages_recvd = 0;
    while (true)
    {
        int n_recv = recvfrom(server_sock, data_recv, DATA_SIZE, 0, (sockaddr *) &client_addr, &sock_len);

        if (n_recv > 0)
        {
            sendto(server_sock, data_recv, DATA_SIZE, 0, (sockaddr *) &client_addr, sock_len);
            messages_recvd++;
            cout << "message number " << messages_recvd << " receieved, bytes: " << n_recv << endl;
        }
    }

    delete[] data_recv;


    return 0;
}

