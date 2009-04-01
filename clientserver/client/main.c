// main.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char ** argv)
{
    struct addrinfo addressHints, * addressInfo; // Create two addinfo structures, one with hints about the connection, the other with the information that gets resolved
    int netSocket; // The networking socket

    addressHints.ai_family = AF_INET; // Use IPv4 (most commonly used, IPv6 isn't mainstream yet)
    addressHints.ai_socktype = SOCK_STREAM; // We are using a stream socket
    
    int error = getaddrinfo("localhost", "35246", &addressHints, &addressInfo); // Get info for localhost on port 35246, using the specified hints, and storing the result to addressInfo

    if (error) // If there was an error with getaddrinfo()
    {
        printf("Failed to get address info\n");
        return 1; // Exit with error status code
    }

    netSocket = socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol); // Create a socket with the results from getaddrinfo
    if (netSocket == -1) // If there was an error with socket()
    {
        printf("There was an error creating the socket\n");
        return 1;
    }

    error = connect(netSocket, addressInfo->ai_addr, addressInfo->ai_addrlen); // Connect to the socket, it can now be read from and written to with recv(), and send()
    if (error == -1) // If there was an error with connect()
    {
        printf("There was an error connecting to the socket\n");
        return 1;
    }

    char message[128]; // Create a buffer for a string to store the user's message;

    printf("Please enter a message: "); // Prompt the user for a message
    
    fgets(message, 128, stdin); // Read a string from the command line
    int bytesSent = send(netSocket, message, strlen(message), 0); // Send the string to the server
    printf("Sent %i bytes to the server\n", bytesSent);
    close(netSocket); // Close the socket
}