#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

const int PORT = 6789;

// Global map to keep track of clients
std::map<int, int> client_sockets;

void display_clients() {
    std::cout << "Currently connected clients:" << std::endl;
    for (const auto& pair : client_sockets) {
        std::cout << "Client ID: " << pair.first << ", Socket FD: " << pair.second << std::endl;
    }
}

void handle_client(int client_sock) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(client_sock, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            std::cout << "Client disconnected or read error\n";
            break;
        }

        std::string msg(buffer);
        std::cout << "Received: " << msg << std::endl;

        if (msg.substr(0, 7) == "Client:") {
            int id = std::stoi(msg.substr(7));
            client_sockets[id] = client_sock;
            std::cout << "Client " << id << " registered.\n";
            display_clients();
        } else if (msg.substr(0, 5) == "Send:") {
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
