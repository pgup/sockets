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

//this address needs to be changed based on what the server address actually is!!!
#define SERVER_ADDRESS __TEXT("192.168.1.143")

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

    WSADATA WSAdata;

    int res = WSAStartup(MAKEWORD(2, 2), &WSAdata);
    if (res != 0)
    {
        cout << "couldn't start up WSA for windows sockets" << endl;
        exit(-1);
    }

    //create socket
    SOCKET client_sock = socket(AF_INET, SOCK_DGRAM, 0);
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

    //set timeouts for packets
    DWORD timeout = TIMEOUT_SEC * 1000000 + TIMEOUT_USEC;
    if (setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO,
                   (const char *)&timeout, sizeof(timeout)) < 0)
    {
        std::cout << "couldn't set sockopt" << std::endl;
        exit(-1);
    }
    if (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO,
                   (const char *)&timeout, sizeof(timeout)) < 0)
    {
        std::cout << "couldn't set sockopt" << std::endl;
        exit(-1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUMBER);

    char *data_send = new char[DATA_SIZE];
    memset(data_send, 0, DATA_SIZE);
    int replies_recvd = 0;

    clock_t total_runtime = clock();
    for (int i = 0; i < PACKETS_TO_SEND; i++)
    {
        //if the server is not running, Windows will recvfrom() an error packet
        //from the failed sendto. This doesn't happen in Linux.
//        int n_sent = 0;
////        while (n_sent != DATA_SIZE)
//            n_sent += sendto(client_sock, data_send, DATA_SIZE, 0,
//                            (sockaddr *)&server_addr, addr_len);


//        int n_recv = 0;
////        do
////        {
//            n_recv += recvfrom(client_sock, data_send, DATA_SIZE, 0,
//                              (sockaddr *)&server_addr, &addr_len);
////        } while (n_recv > 0 && n_recv < DATA_SIZE);

//            cout << "sent " << n_sent << endl;
//            cout << "recv " << n_recv << endl;


        int total_sent = 0;
        while (total_sent < DATA_SIZE)
        {
            int n_sent = sendto(client_sock, data_send + total_sent,
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
            int n_recv = recvfrom(client_sock, data_send + total_recv,
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

    total_runtime = clock() - total_runtime;

    delete[] data_send;

    cout << "received " << replies_recvd <<
            " replies out of " << PACKETS_TO_SEND << " packets sent" << endl;
    cout << "average RTT " <<
            (double)total_runtime / CLOCKS_PER_SEC / PACKETS_TO_SEND << endl;

    return 0;
}
