#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT_NUMBER 54321
#define DATA_SIZE 1024 * 1

#define PACKETS_TO_SEND 1000


//this address needs to be changed based on what the server address actually is!!!
#define SERVER_ADDRESS "192.168.200.149"

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
    int client_sock;
    sockaddr_in server_addr;

    //create socket
    if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
            cout << "ERROR creating server socket" << endl;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUMBER);

    //create server address from string and copy it to server_addr
    inet_pton(AF_INET, SERVER_ADDRESS, &server_addr.sin_addr);

    socklen_t addr_len = sizeof(sockaddr_in);
    char *data_send = new char[DATA_SIZE];

    int replies_recvd = 0;
    for (int i = 0; i < PACKETS_TO_SEND; i++)
    {
        sendto(client_sock, data_send, DATA_SIZE, 0, (sockaddr *)&server_addr, addr_len);

        int n_recv = recvfrom(client_sock, data_send, DATA_SIZE, 0, (sockaddr *)&server_addr, &addr_len);

        if (n_recv > 0)
        {
            cout << "received back: " << data_send << endl;
            replies_recvd++;
        }
    }

    delete[] data_send;

    cout << "received " << replies_recvd << " replies out of " << PACKETS_TO_SEND << " packets sent" << endl;

    return 0;
}
