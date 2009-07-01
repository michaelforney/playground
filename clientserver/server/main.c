// main.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char ** argv)
{
    struct sockaddr clientAddress; // Create a sockaddr for the client address
    socklen_t clientAddressSize; // The length of the client address
    struct addrinfo addressHints, * addressInfo; // Create two addinfo structures, one with hints about the connection, the other with the information that gets resolved
    int listenSocket, connectionSocket; // Create two sockets, one for listening, and one for communicating with the client

    addressHints.ai_family = AF_INET; // Use IPv4 (most commonly used, IPv6 isn't mainstream yet)
    addressHints.ai_socktype = SOCK_STREAM; // We are using a stream socket
    addressHints.ai_flags = AI_PASSIVE; // This is a listening connection, so we let getaddrinfo() fill in our own ip

    int error = getaddrinfo(NULL, "5001", &addressHints, &addressInfo); // Get address info for ourself. We leave the IP blank, but specify the port we want to listen on
    if (error) // If there was an error with getaddrinfo()
    {
        printf("Failed to get address info for the server\n");
        return 1; // Exit with error status code
    }
    
    listenSocket = socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol); // Create a socket that we will use for listening
    if (listenSocket == -1) // If there was an error with socket()
    {
        printf("There was an error creating the socket for listening\n");
        return 1;
    }
    
    error = bind(listenSocket, addressInfo->ai_addr, addressInfo->ai_addrlen); // Bind that socket to port 35246 on the local IP
    if (error == -1) // If there was an error with bind()
    {
        printf("Failed to bind the socket to port 35246 (is there another server running on this port?)\n");
        return 1; // Exit with error status code
    }
    
    error = listen(listenSocket, 0); // Start listening for connections, and allow 0 queued connections, this function returns when there was an incoming connection
    if (error == -1) // If there was an error with listen()
    {
        printf("There was an error listening for incoming connections\n");
        return 1; // Exit with error status code
    }
    
    clientAddressSize = sizeof(clientAddress); // Set the client address size to the size of the address
    connectionSocket = accept(listenSocket, &clientAddress, &clientAddressSize); // Accept the incoming connection, and create a socket from it
    if (error == -1) // If there was an error with accept()
    {
        printf("Failed to accept the incoming connection\n");
        return 1; // Exit with error status code
    }

    char message[128]; // Define a buffer for the incoming message
    int bytesRead = recv(connectionSocket, message, 128, 0); // Receive a maximum of 128 bytes, and store it in our string
    if (bytesRead == -1) // If there was an error reading from the socket
    {
        printf("There was an error reading bytes from the client\n");
        return 1;
    }

    printf("Received message: %s\n", message); // Print the message
    return 0;
}