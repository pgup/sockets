#include <iostream>
#include <ctime>

#include <WinSock2.h>
#include <WS2tcpip.h>

#if defined (_MSC_VER)
    #pragma comment(lib, "Ws2_32.lib")
#endif

#define PORT_NUMBER 54321
#define DATA_SIZE 64000
#define TIMEOUT_SEC 2
#define TIMEOUT_USEC 250

#define PACKETS_TO_SEND 100

#define SERVER_ADDRESS __TEXT("192.168.1.143")

using namespace std;

sockaddr_in server_addr;
char data_send[DATA_SIZE];

DWORD WINAPI send_UDP(void *client_sock)
{
    SOCKET *client = (SOCKET *)client_sock;

    int replies_recvd = 0;
    socklen_t addr_len = sizeof(server_addr);

    for (int i = 0; i < PACKETS_TO_SEND; i++)
    {
        //if the server is not running, Windows will recvfrom() an error packet
        //from the failed sendto. This doesn't happen in Linux.

        int total_sent = 0;
        while (total_sent < DATA_SIZE)
        {
            int n_sent = sendto(*client, data_send + total_sent,
                                DATA_SIZE - total_sent, 0,
                                (sockaddr *)&server_addr, addr_len);

            if (n_sent < 0)
            {
                cerr << "error while sending data" << endl;
                break;
            }

            total_sent += n_sent;
        }

        int total_recv = 0;
        while (total_recv < DATA_SIZE)
        {
            int n_recv = recvfrom(*client, data_send + total_recv,
                                  DATA_SIZE - total_recv, 0,
                                  (sockaddr *)&server_addr, &addr_len);

            if (n_recv < 0)
            {
                cerr << "error while receiving data" << endl;
                break;
            }

            total_recv += n_recv;
        }

        if (total_recv > 0)
            replies_recvd++;
    }

    return replies_recvd;
}

DWORD WINAPI send_TCP(void *client_sock)
{
    SOCKET *client = (SOCKET *)client_sock;

    memset(data_send, 0, DATA_SIZE);

    for (int i = 0; i < PACKETS_TO_SEND; i++)
    {
        //make sure the whole 'packet' is sent and recv by looping
        //the kernel may not necessarily send the whole packet in one send
        //and also may not receive the whole packet with one recv.
        //This probably isn't the best way to achieve the desired result
        int total_sent = 0;
        while (total_sent < DATA_SIZE)
        {
            int n_sent = send(*client, data_send + total_sent,
                              DATA_SIZE - total_sent, 0);

            if (n_sent < 0)
            {
                cerr << "error while sending data" << endl;
                break;
            }

            total_sent += n_sent;

        }

        int total_recv = 0;
        while (total_recv < DATA_SIZE)
        {
            int n_recv = recv(*client, data_send + total_recv,
                              DATA_SIZE - total_recv, 0);

            if (n_recv < 0)
            {
                cerr << "error while receiving data" << endl;
                break;
            }

            total_recv += n_recv;
        }
    }

    return 0;
}

int main()
{
    memset(&server_addr, 0, sizeof(server_addr));

    //start up Winsock
    WSADATA WSAdata;

    if (WSAStartup(MAKEWORD(2, 2), &WSAdata) != 0)
    {
        cout << "couldn't start up WSA for windows sockets" << endl;
        exit(-1);
    }

    SOCKET udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock == INVALID_SOCKET)
    {
        cout << "ERROR creating udp socket" << endl;
        exit(-1);
    }

    //create socket
    SOCKET tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock == INVALID_SOCKET)
    {
        cout << "ERROR creating tcp socket" << endl;
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

    if (connect(tcp_sock, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        cout << "error connecting" << endl;
        exit(-1);
    }


    clock_t total_runtime = clock();
    HANDLE udp_thread = CreateThread(0, 0, send_UDP, (void *)&udp_sock, 0, 0);

    HANDLE tcp_thread = CreateThread(0, 0, send_UDP, (void *)&tcp_sock, 0, 0);

    HANDLE threads[] = { udp_thread,
                         tcp_thread };
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    total_runtime = clock() - total_runtime;

    DWORD udp_packets;
    GetExitCodeThread(udp_thread, &udp_packets);
    cout << "received " << udp_packets << " udp packets" << endl;

    CloseHandle(udp_thread);
    CloseHandle(tcp_thread);

    closesocket(udp_sock);
    closesocket(tcp_sock);

    cout << "average RTT " <<
            (double)total_runtime / CLOCKS_PER_SEC / PACKETS_TO_SEND << endl;
}

