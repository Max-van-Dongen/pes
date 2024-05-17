#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#define BUFF_SIZE 32
char clientid[] = "Client:14\n";
int main() {
    int clientSocket;
    bool knownClientId = false;
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

    send(clientSocket, clientid, strlen(clientid), 0);
    std::cout << "Registered as " << clientid << std::endl;



    fcntl(clientSocket, F_SETFL, O_NONBLOCK);
    while (true) {


        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
        struct timeval tv;
        tv.tv_sec = 0;  // Set the timeout to 0 so select doesn't block
        tv.tv_usec = 0;
        if (select(clientSocket + 1, &readfds, NULL, NULL, &tv) > 0) {
            // If select() returns > 0, there is data to read on the socket
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead > 0) {
                // Process the data from the socket
                std::cout << "Received data from server: " << buffer << std::endl;

                // Prepare the data to be sent over the I2C bus
                unsigned char i2cData[BUFF_SIZE] = {0};
                memcpy(i2cData, buffer, bytesRead < BUFF_SIZE ? bytesRead : BUFF_SIZE);

                // Send the data back over the I2C bus
                if (write(i2cFile, i2cData, BUFF_SIZE) != BUFF_SIZE) {
                    perror("Failed to write to the I2C bus.\n");
                    usleep(500000);
                    continue;
                }
                std::cout << "Sent data back over I2C: " << buffer << std::endl;
            }
        }
        // if (!knownClientId) {
        //     unsigned char sendData[BUFF_SIZE] = "ClientIdRequest";
        //     if (write(i2cFile, sendData, BUFF_SIZE) != BUFF_SIZE) {
        //         perror("Failed to write to the I2C bus.\n");
        //         usleep(500000);
        //     }
        //     std::cout << "Wrote ClientIdRequest" << std::endl;
        //     knownClientId = true;
        //     unsigned char data[BUFF_SIZE] = {0};
        //     if (read(i2cFile, data, BUFF_SIZE) < BUFF_SIZE) {
        //         perror("Failed to read from the I2C bus.\n");
        //         usleep(500000);
        //         continue;
        //     }
        //     std::string currentData(reinterpret_cast<char*>(data), BUFF_SIZE);
        //     if (data[0] != 0x00) {
        //         std::cout << "Got Data from i2ccccc: " << currentData << "l: "<< currentData.length() <<std::endl;
        //     }
        // }
        unsigned char data[BUFF_SIZE] = {0};
        if (read(i2cFile, data, BUFF_SIZE) < BUFF_SIZE) {
            perror("Failed to read from the I2C bus.\n");
            usleep(500000);
            continue;
        }
        
        std::string currentData(reinterpret_cast<char*>(data), BUFF_SIZE);
        if (data[0] != 0x00) {
            std::cout << "Got Data from i2c: " << currentData << "l: "<< currentData.length() <<std::endl;
        //  }

        // if (currentData != lastData || (true && data[0] != 0x00)) {// || TRUE IS TESTING, REMOVE IF ALWAYS SENDING THE SAME INFO
            lastData = currentData + '\n';

            send(clientSocket, lastData.c_str(), lastData.length(), 0);
            std::cout << "Data sent to server: " << lastData << std::endl;
        }
        usleep(500000);
    }

    close(clientSocket);
    close(i2cFile);
    return 0;
}
