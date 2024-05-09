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
    unsigned char lastData = 0x00; // Initialize last data for comparison

    // Open I2C bus
    const char *i2cDevice = "/dev/i2c-1";  // Check your I2C bus (usually it's i2c-1 on Raspberry Pi)
    int i2cFile;
    if ((i2cFile = open(i2cDevice, O_RDWR)) < 0) {
        perror("Failed to open the bus.\n");
        return 1;
    }

    // Specify the address of the I2C Slave to communicate with
    int addr = 0x12;  // The I2C address of the peripheral
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

    // Configure settings of the server address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6789);  // Same port as server
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Server IP address

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return 1;
    }
    std::cout << "Connected to server!" << std::endl;


    char msgc[] = "Client:14";
    send(clientSocket, msgc, strlen(msgc), 0);
    std::cout << "Registered" << std::endl;

    while (true) {
        // Write to I2C device to initiate transaction
        unsigned char command = 0x01;  // Command to request funcToRun value
        if (write(i2cFile, &command, 1) != 1) {
            perror("Failed to write to the I2C bus.\n");
            usleep(500000);  // Wait for 0.5 seconds before next write
            continue;
        }
            std::cout << "Wrote Data Request" << std::endl;

        // Read response from I2C slave
        unsigned char data[1] = {0};
        if (read(i2cFile, data, 1) != 1) {
            perror("Failed to read from the I2C bus.\n");
            usleep(500000);  // Wait for 0.5 seconds before next read
            continue;
        }
        
            char msgd[4];
            sprintf(msgd, "%02X", data[0]);  // Send data as a hexadecimal string
            std::cout << "Got Data: " << msgd << std::endl;

        // Only send data if it's different from last data read
        if (data[0] != lastData) {
            lastData = data[0]; // Update lastData

            char msg[20];
            sprintf(msg, "Send:10:LOOOOONGTEST%d", data[0]);
            send(clientSocket, msg, strlen(msg), 0);
            std::cout << "Data sent to server: " << msg << std::endl;
        }

        usleep(500000);  // Wait for 0.5 seconds before next interaction
    }

    // Close the socket and I2C file
    close(clientSocket);
    close(i2cFile);
    return 0;
}
