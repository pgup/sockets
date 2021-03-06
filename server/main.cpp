#include <iostream>

#include <WinSock2.h>
#include <WS2tcpip.h>

#if defined (_MSC_VER)
    #pragma comment(lib, "Ws2_32.lib")
#endif

#define PORT_NUMBER 54321
#define DATA_SIZE 64000
#define TIMEOUT_SEC 2
#define TIMEOUT_USEC 250

#define CLIENTS 2

using namespace std;

int main()
{
    //start up Winsock
    WSADATA WSAdata;

    int res = WSAStartup(MAKEWORD(2, 2), &WSAdata);
    if (res != 0)
    {
        cerr << "couldn't start up WSA for windows sockets" << endl;
        exit(-1);
    }

    //create LISTENER socket
    SOCKET listener_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_sock == INVALID_SOCKET)
    {
        cerr << "ERROR creating server socket" << endl;
        exit(-1);
    }

    //create UDP socket
    SOCKET udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock == INVALID_SOCKET)
    {
        cerr << "ERROR creating server socket" << endl;
        exit(-1);
    }

    sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUMBER);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    //bind LISTENER socket
    int reuse = 1;
    setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse));
    if (bind(listener_sock, (sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        cerr << "ERROR binding LISTENER socket" << endl;
        exit(-1);
    }
    listen(listener_sock, CLIENTS);

    //bind UDP socket
    if (bind(udp_sock, (sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        cerr << "ERROR binding UDP socket" << endl;
        exit (-1);
    }

    //set timout for udp socket
    DWORD timeout = TIMEOUT_SEC * 1000000 + TIMEOUT_USEC;
    if (setsockopt(udp_sock, SOL_SOCKET, SO_SNDTIMEO,
                   (const char *)&timeout, sizeof(timeout)) < 0)
    {
        std::cout << "couldn't set sockopt" << std::endl;
        exit(-1);
    }
    if (setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO,
                   (const char *)&timeout, sizeof(timeout)) < 0)
    {
        std::cout << "couldn't set sockopt" << std::endl;
        exit(-1);
    }

    fd_set client_socks;
    fd_set read_socks;

    FD_ZERO(&client_socks);
    FD_SET(listener_sock, &client_socks);
    FD_SET(udp_sock, &client_socks);

    socklen_t sock_len = sizeof(sockaddr_in);
    char *data_recv = new char[DATA_SIZE];

    while (true)
    {
//        read_socks = client_socks;
        //clear the sockets to read from and reestablish the clients
        //that are already connected as well as the listener and udp sockets
        FD_ZERO(&read_socks);
        for (unsigned int i = 0; i < client_socks.fd_count; i++)
        {
            FD_SET(client_socks.fd_array[i], &read_socks);
        }

        if (select(0, &read_socks, NULL, NULL, NULL) < 0)
        {
            cerr << "problem calling select to read from sockets" << endl;
            exit(-1);
        }

        //communication on the listener aka:
        //there is a client awaiting connection
        if (FD_ISSET(listener_sock, &read_socks))
        {
            //received a new TCP connection request
            sockaddr_in new_client_addr;
            SOCKET new_client = accept(listener_sock,
                                       (sockaddr *)&new_client_addr,
                                       &sock_len);
            FD_SET(new_client, &client_socks);
        }
        //communication on the udp socket
        else if (FD_ISSET(udp_sock, &read_socks))
        {
//            int n_recv = 0;
//            do
//            {
//                n_recv += recv(udp_sock, data_recv, DATA_SIZE, 0);
//            } while(n_recv > 0 && n_recv < DATA_SIZE);

//            int n_sent = 0;
//            while (n_sent != DATA_SIZE)
//                n_sent += send(udp_sock, data_recv, DATA_SIZE, 0);
            int n_recv = recvfrom(udp_sock, data_recv, DATA_SIZE, 0,
                                  (sockaddr *) &client_addr, &sock_len);

            if (n_recv > 0)
            {
                sendto(udp_sock, data_recv, n_recv, 0,
                       (sockaddr *) &client_addr, sock_len);
            }
        }
        //the communication occured from one of the connected TCP clients
        else
            for (unsigned int i = 0; i < read_socks.fd_count; i++)
            {
                SOCKET client = read_socks.fd_array[i];
                if (FD_ISSET(client, &read_socks))
                {

                    //same for the client...
                    //loop and make sure the whole packet is sent and recv'd
                    int total_recv = 0;

                    while(total_recv < DATA_SIZE)
                    {
                        int n_recv = recv(client, data_recv + total_recv,
                                          DATA_SIZE - total_recv, 0);

                        //error of some kind, or connection closed -> break out
                        if (n_recv <= 0)
                        {
                            total_recv = n_recv;
                            break;
                        }

                        //increment position and continue recv()ing
                        total_recv += n_recv;
                    }

                    if (total_recv == SOCKET_ERROR)
                    {
                        int error = WSAGetLastError();

                        if (error == WSAECONNRESET)
                        {
                            closesocket(client);
                            FD_CLR(client, &client_socks);
                        }
                    }
                    //the connection was closed
                    if (total_recv == 0)
                    {
                        FD_CLR(client, &client_socks);
                        closesocket(client);
                    }
                    else
                    {
                        int total_sent = 0;
                        while (total_sent < DATA_SIZE)
                        {
                            int n_sent = send(client, data_recv + total_sent,
                                              DATA_SIZE - total_sent, 0);

                            //error of some kind, break out
                            if (n_sent < 0)
                                break;

                            total_sent += n_sent;
                        }
                    }
                }
            }

    }

    delete[] data_recv;


    return 0;
}

