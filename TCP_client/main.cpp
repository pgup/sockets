#include <iostream>
#include <ctime>

#include <WinSock2.h>
#include <WS2tcpip.h>

#if defined (_MSC_VER)
    #pragma comment(lib, "Ws2_32.lib")
#endif

#define PORT_NUMBER 54321
#define DATA_SIZE 1024 * 4
#define TIMEOUT_SEC 2
#define TIMEOUT_USEC 250

#define PACKETS_TO_SEND 100

#define SERVER_ADDRESS __TEXT("192.168.1.115")

using namespace std;

int main()
{
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    WSADATA WSAdata;

    int res = WSAStartup(MAKEWORD(2, 2), &WSAdata);
    if (res != 0)
    {
        cout << "couldn't start up WSA for windows sockets" << endl;
        exit(-1);
    }

    //create socket
    SOCKET client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == INVALID_SOCKET)
    {
        cout << "ERROR creating server socket" << endl;
        exit(-1);
    }

    socklen_t addr_len = sizeof(sockaddr);

    //create server address from string and copy it to server_addr
    if (WSAStringToAddress(SERVER_ADDRESS, AF_INET, NULL,
                           (sockaddr *)&server_addr, &addr_len) != 0)
    {
        cout << "error getting address" << endl;
        exit(-1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUMBER);

    if (connect(client_sock, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        cout << "error connecting" << endl;
        exit(-1);
    }


    char *data_send = new char[DATA_SIZE];
    memset(data_send, 0, DATA_SIZE);
    clock_t total_runtime = clock();

    for (int i = 0; i < PACKETS_TO_SEND; i++)
    {
        if (send(client_sock, data_send, DATA_SIZE, 0) < 0)
            cout << "tcp error" << endl;

        recv(client_sock, data_send, DATA_SIZE, 0);
    }

    total_runtime = clock() - total_runtime;

    delete[] data_send;
    closesocket(client_sock);

    cout << "average RTT " <<
            (double)total_runtime / CLOCKS_PER_SEC / PACKETS_TO_SEND << endl;
}

