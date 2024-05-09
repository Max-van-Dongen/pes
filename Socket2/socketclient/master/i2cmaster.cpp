#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[1024] = {0};
    std::string lastData; // Use std::string to store the last data

    // Open I2C bus
    const char *i2cDevice = "/dev/i2c-1";
    int i2cFile;
    if ((i2cFile = open(i2cDevice, O_RDWR)) < 0) {
        perror("Failed to open the bus.\n");
        return 1;
    }

    // Specify the address of the I2C Slave
    int addr = 0x12;
    if (ioctl(i2cFile, I2C_SLAVE, addr) < 0) {
        perror("Failed to acquire bus access and/or talk to slave.\n");
        return 1;
    }

    // Create the socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error in socket creation" << std::endl;
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(16789);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return 1;
    }
    std::cout << "Connected to server!" << std::endl;

    char msgc[] = "Client:14";
    send(clientSocket, msgc, strlen(msgc), 0);
    std::cout << "Registered" << std::endl;

    while (true) {
        unsigned char command = 0x01;
        if (write(i2cFile, &command, 1) != 1) {
            perror("Failed to write to the I2C bus.\n");
            usleep(500000);
            continue;
        }
        std::cout << "Wrote Data Request" << std::endl;

        unsigned char data[128] = {0};
        if (read(i2cFile, data, 128) < 128) {
            perror("Failed to read from the I2C bus.\n");
            usleep(500000);
            continue;
        }
        
        std::string currentData(reinterpret_cast<char*>(data), 128);
        std::cout << "Got Data: " << currentData << std::endl;

        if (currentData != lastData || true) {// || TRUE IS TESTING, REMOVE IF ALWAYS SENDING THE SAME INFO
            lastData = currentData;

            send(clientSocket, lastData.c_str(), lastData.length(), 0);
            std::cout << "Data sent to server: " << lastData << std::endl;
        }

        usleep(500000);
    }

    close(clientSocket);
    close(i2cFile);
    return 0;
}
