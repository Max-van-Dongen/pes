#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
  // Create a socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("socket");
    return 1;
  }

  // Specify the server address
  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(16789);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Connect to the server
  if (connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
    perror("connect");
    close(sockfd);
    return 1;
  }

  while (true) {
    // Get user input
    std::string message;
    std::cout << "Enter message: ";
    std::getline(std::cin, message);
    message += '\n';
    // Send the message to the server
    int bytesSent = send(sockfd, message.c_str(), message.length(), 0);
    if (bytesSent == -1) {
      perror("send");
      break;
    }

    // Check for termination message
    if (message == "exit") {
      break;
    }
  }

  // Close the socket
  close(sockfd);

  return 0;
}
