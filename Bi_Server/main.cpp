#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define PORT_NUMBER 54321
#define DATA_SIZE 1024

using namespace std;

int main()
{
    int server_sock;

    //create socket
    if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
            cout << "ERROR creating server socket" << endl;

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

