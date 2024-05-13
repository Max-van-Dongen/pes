#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

const int PORT = 16789;

// Global map to keep track of clients
std::map<int, int> client_sockets;

void display_clients() {
    std::cout << "Currently connected clients:" << std::endl;
    for (const auto& pair : client_sockets) {
        std::cout << "Client ID: " << pair.first << ", Socket FD: " << pair.second << std::endl;
    }
}

void handle_client(int client_sock) {

    // char buffer[1024];
    int clientId = -1; // Store the client ID
    // while (true) {
    //     memset(buffer, 0, sizeof(buffer));
    //     int bytes_read = read(client_sock, buffer, sizeof(buffer) - 1);
    //     if (bytes_read <= 0) {
    //         std::cout << "Client disconnected or read error\n";
    //     client_sockets.erase(clientId);
    //     std::cout << "Client " << clientId << " removed from registry.\n";
    //     display_clients();
    //         break;
    //     }

    //     std::string msg(buffer);
    //     std::cout << "Received: " << msg << std::endl;

    //     if (msg.substr(0, 7) == "Client:") {
    //         clientId = std::stoi(msg.substr(7));
    //         int id = std::stoi(msg.substr(7));
    //         client_sockets[id] = client_sock;
    //         std::cout << "Client " << id << " registered.\n";
    //         display_clients();
    //     } else if (msg.substr(0, 5) == "Send:") {
    //         size_t pos = msg.find(':');
    //         size_t next_pos = msg.find(':', pos + 1);
    //         int target_id = std::stoi(msg.substr(pos + 1, next_pos - pos - 1));
    //         std::string data = msg.substr(next_pos + 1);

    //         if (client_sockets.find(target_id) != client_sockets.end()) {
    //             int target_sock = client_sockets[target_id];
    //             send(target_sock, data.c_str(), data.size(), 0);
    //             std::cout << "Data sent to client " << target_id << std::endl;
    //         } else {
    //             std::cout << "Target client not found\n";
    //         }
    //     }
    // }
    
    std::string buffer;  // Use a std::string to accumulate data
    while (true) {
        char temp[1024] = {};  // Temporary buffer to store incoming data
        int bytes_read = read(client_sock, temp, sizeof(temp) - 1);
        if (bytes_read <= 0) {
            std::cout << "Client disconnected or read error\n";
            client_sockets.erase(client_sock);
            std::cout << "Client socket " << client_sock << " removed from registry.\n";
            display_clients();
            break;
        }

        buffer.append(temp, bytes_read);  // Append new data to the buffer

        size_t pos;
        while ((pos = buffer.find('\n')) != std::string::npos) {  // Process each message separated by '\n'
            std::string msg = buffer.substr(0, pos);  // Extract the message up to '\n'
            std::cout << "Received: " << msg << std::endl;
            buffer.erase(0, pos + 1);  // Remove the processed message from buffer

            if (msg.substr(0, 7) == "Client:") {
                int clientId = std::stoi(msg.substr(7));
                client_sockets[clientId] = client_sock;
                std::cout << "Client " << clientId << " registered.\n";
                display_clients();
            } else if (msg.substr(0, 5) == "Send:") {
                // size_t colon_pos = msg.find(':', 5);
                // size_t next_colon_pos = msg.find(':', colon_pos + 1);
                // if (colon_pos != std::string::npos && next_colon_pos != std::string::npos) {
                //     int target_id = std::stoi(msg.substr(colon_pos + 1, next_colon_pos - colon_pos - 1));
                //     std::string data = msg.substr(next_colon_pos + 1);

                //     if (client_sockets.find(target_id) != client_sockets.end()) {
                //         int target_sock = client_sockets[target_id];
                //         send(target_sock, data.c_str(), data.size(), 0);
                //         std::cout << "Data sent to client " << target_id << std::endl;
                //     } else {
                //         std::cout << "Target client not found\n";
                //     }
                // }
                    size_t pos = msg.find(':');
                    size_t next_pos = msg.find(':', pos + 1);
                    int target_id = std::stoi(msg.substr(pos + 1, next_pos - pos - 1));
                    std::string data = msg.substr(next_pos + 1);

                    if (client_sockets.find(target_id) != client_sockets.end()) {
                        int target_sock = client_sockets[target_id];
                        send(target_sock, data.c_str(), data.size(), 0);
                        std::cout << "Data sent to client " << target_id << std::endl;
                    } else {
                        std::cout << "Target client not found\n";
                    }
            }
        }
    }

    if (clientId != -1) {
        client_sockets.erase(clientId);
        std::cout << "Client " << clientId << " removed from registry.\n";
        display_clients();
    }
    // Clean up when client disconnects
    close(client_sock);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    listen(server_sock, 5);
    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            std::cerr << "Failed to accept client\n";
            continue;
        }

        std::thread(handle_client, client_sock).detach();
    }

    close(server_sock);
    return 0;
}
