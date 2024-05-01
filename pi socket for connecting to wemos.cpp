#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>

int main() {
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    char buffer[1024];
    int bytesRead;

    // Create the socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Configure settings of the server address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6789); // Port
    serverAddr.sin_addr.s_addr = INADDR_ANY; // IP address

    // Bind the address struct to the socket
    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

while (true) {
    // Listen on the socket, with 5 max connection requests queued
    if (listen(serverSocket, 5) == 0)
        std::cout << "Server gestart luisteren..." << std::endl;
    else
        std::cout << "Luisteren mislukt!" << std::endl;

    addr_size = sizeof(serverStorage);
    // Accept call creates a new socket for the incoming connection
    newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);

        // Ontvang bericht van de client
        bytesRead = recv(newSocket, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::cout << "Bericht van Wemos: " << buffer << std::endl;


	if (buffer[0] == '1') {
		const char* response = "1";
		send(newSocket, response, strlen(response), 0);
	}
	
	if (buffer[0] == '2') {
		const char* response = "2";
		send(newSocket, response, strlen(response), 0);
	}
	    // Stuur een antwoord terug naar de client
	    //const char* response = "1";
            //send(newSocket, response, strlen(response), 0);
        }
    

    // Sluit de sockets
    std::cout << "Luisteren beindigt" << std::endl;
    close(newSocket);
}
    close(serverSocket);

    return 0;
}
