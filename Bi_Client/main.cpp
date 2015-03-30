#include <iostream>
#include <cstring>
#include <ctime>
#include <cstdlib>

#if defined(WIN32)
#include <WinSock2.h>
#include <WS2tcpip.h>
#if defined (_MSC_VER)
    #pragma comment(lib, "Ws2_32.lib")
#endif
#elif defined(__linux__)
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define PORT_NUMBER 54321
#define DATA_SIZE 1024 * 4
#define TIMEOUT_SEC 2
#define TIMEOUT_USEC 250

#define PACKETS_TO_SEND 100


//this address needs to be changed based on what the server address actually is!!!
#if defined (WIN32)
#define SERVER_ADDRESS __TEXT("192.168.1.115")
#else
#define SERVER_ADDRESS "192.168.1.143"
#endif

/*
 * there are three ways to get the IP of the server address.
 * inet_pton(AF_INET, SERVER_ADDRESS, &server_addr.sin_addr)
 * or
 * using a hostent struct and copying a member from h_addr_list[] into server_addr.sin_addr
 * - hostent *host = gethostbyname(COMPUTER_NAME);
 * - memcpy(&server_addr.sin_addr, host->h_addr_list[0], host->h_length);
 * or
 * - hostent *host = gethostbyaddr(SERVER_ADDRESS, 15, AF_INET)
 * - memcpy(&server_addr.sin_addr, host->h_addr_list[0], host->h_length);
 *
 * I went with inet_pton... seems less messy
*/

using namespace std;

int main()
{
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

#if defined (WIN32)
    SOCKET client_sock;
    WSADATA WSAdata;

    int res = WSAStartup(MAKEWORD(2, 2), &WSAdata);
    if (res != 0)
    {
        cout << "couldn't start up WSA for windows sockets" << endl;
        exit(-1);
    }
#elif defined (__linux__)
    int client_sock;
#endif

    //create socket
    client_sock = socket(AF_INET, SOCK_DGRAM, 0);
#if defined (WIN32)
    if (client_sock == INVALID_SOCKET)
    {
        cout << "ERROR creating server socket" << endl;
        exit(-1);
    }
#elif defined (__linux__)
    if (client_sock < 0)
    {
        cout << "ERROR creating server socket" << endl;
        exit(-1);
    }
#endif

    socklen_t addr_len = sizeof(sockaddr);

#if defined (WIN32)

    //create server address from string and copy it to server_addr
    if (WSAStringToAddress(SERVER_ADDRESS, AF_INET, NULL, (sockaddr *)&server_addr, &addr_len) != 0)
    {
        cout << "error getting address" << endl;
        exit(-1);
    }

#elif defined (__linux__)
    inet_pton(AF_INET, SERVER_ADDRESS, &server_addr.sin_addr);
#endif

#if defined (WIN32)
    DWORD timeout = TIMEOUT_SEC * 1000 + TIMEOUT_USEC;
    if (setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout)) < 0)
    {
        std::cout << "couldn't set sockopt" << std::endl;
        exit(-1);
    }
    if (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0)
    {
        std::cout << "couldn't set sockopt" << std::endl;
        exit(-1);
    }
#else
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = TIMEOUT_USEC;
    if (setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        std::cout << "couldn't set sockopt" << std::endl;
        exit(-1);
    }
    if (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        std::cout << "couldn't set sockopt" << std::endl;
        exit(-1);
    }
#endif

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUMBER);

    char *data_send = new char[DATA_SIZE];
    int replies_recvd = 0;

    clock_t total_runtime = clock();
    for (int i = 0; i < PACKETS_TO_SEND; i++)
    {
        //if the server is not running, Windows will recvfrom() an error packet
        //from the failed sendto. This doesn't happen in Linux.
        sendto(client_sock, data_send, DATA_SIZE, 0, (sockaddr *)&server_addr, addr_len);

        int n_recv = recvfrom(client_sock, data_send, DATA_SIZE, 0, (sockaddr *)&server_addr, &addr_len);

        if (n_recv > 0)
            replies_recvd++;
    }

    total_runtime = clock() - total_runtime;

    delete[] data_send;

    cout << "received " << replies_recvd << " replies out of " << PACKETS_TO_SEND << " packets sent" << endl;
    cout << "average RTT " << (double)total_runtime / CLOCKS_PER_SEC / PACKETS_TO_SEND << endl;

    return 0;
}
