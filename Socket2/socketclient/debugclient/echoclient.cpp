#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


int main() {
    int sock;
    struct sockaddr_in server_addr;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Could not create socket\n";
        return 1;
    }
    std::cout << "Socket created\n";

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP address of the server
    server_addr.sin_port = htons(6789);

    // Connect to remote server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connect failed. Error\n";
        return 1;
    }
    std::cout << "Connected to server\n";

    // Send client ID
    std::string registration = "Client:10";
    if (send(sock, registration.c_str(), registration.length(), 0) < 0) {
        std::cerr << "Send failed\n";
        return 1;
    }
    std::cout << "Registration message sent\n";

    // Receive data from the server
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read < 0) {
            std::cerr << "Read failed\n";
            break;
        } else if (bytes_read == 0) {
            std::cout << "Server disconnected\n";
            break;
        }
        std::cout << "Received data: " << buffer << std::endl;
    }

    close(sock);
    return 0;
}
