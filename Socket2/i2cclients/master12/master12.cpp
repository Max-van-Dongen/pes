#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sstream>

#define BUFF_SIZE 16

char clientid[] = "Client:12\n";
int addr = 0x12;

// Function to get current timestamp with milliseconds as a string
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_as_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    long duration = (long) std::chrono::duration_cast<std::chrono::milliseconds>(value).count();
    long ms = duration % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_as_time_t), "[%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms << "]";
    return ss.str();
}
void perrorWithTimestamp(const char* message) {
  std::cerr << getCurrentTimestamp() << " - " << message << std::endl;
  perror(""); // Per standard, perror also prints to stderr
}

int main() {
    int clientSocket;
    bool knownClientId = false;
    struct sockaddr_in serverAddr;
    char buffer[1024] = {0};
    std::string lastData;

    const char *i2cDevice = "/dev/i2c-1";
    int i2cFile;
    if ((i2cFile = open(i2cDevice, O_RDWR)) < 0) {
        perrorWithTimestamp("Failed to open the bus.\n");
        return 1;
    }

    if (ioctl(i2cFile, I2C_SLAVE, addr) < 0) {
        perrorWithTimestamp("Failed to acquire bus access and/or talk to slave.\n");
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << getCurrentTimestamp() << " - Error in socket creation" << std::endl;
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(16789);
    serverAddr.sin_addr.s_addr = inet_addr("10.0.0.3");

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << getCurrentTimestamp() << " - Connection Failed" << std::endl;
        return 1;
    }
    std::cout << getCurrentTimestamp() << " - Connected to server!" << std::endl;

    send(clientSocket, clientid, strlen(clientid), 0);
    std::cout << getCurrentTimestamp() << " - Registered as " << clientid << std::endl;

    fcntl(clientSocket, F_SETFL, O_NONBLOCK);

    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        if (select(clientSocket + 1, &readfds, NULL, NULL, &tv) > 0) {
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead > 0) {
                std::cout << getCurrentTimestamp() << " - SOCK DATA: " << buffer << std::endl;

                unsigned char i2cData[BUFF_SIZE] = {0};
                memcpy(i2cData, buffer, bytesRead < BUFF_SIZE ? bytesRead : BUFF_SIZE);

                if (write(i2cFile, i2cData, BUFF_SIZE) != BUFF_SIZE) {
                    perrorWithTimestamp("Failed to write to the I2C bus.");
                    usleep(500000);
                    continue;
                }
                std::cout << getCurrentTimestamp() << " - SOCK > I2C: " << buffer << std::endl;
            }
        }

        unsigned char data[BUFF_SIZE] = {0};
        if (read(i2cFile, data, BUFF_SIZE) < BUFF_SIZE) {
            perrorWithTimestamp("Failed to read from the I2C bus.");
            usleep(500000);
            continue;
        }

        std::string currentData(reinterpret_cast<char*>(data), BUFF_SIZE);
        if (data[0] != 0x00) {
            std::cout << getCurrentTimestamp() << " - I2C DATA: " << currentData << " - L: " << currentData.length() << std::endl;

            lastData = currentData + '\n';
            send(clientSocket, lastData.c_str(), lastData.length(), 0);
            std::cout << getCurrentTimestamp() << " - I2C > SOCK: " << lastData << std::endl;
        }
        usleep(100000);
    }

    close(clientSocket);
    close(i2cFile);
    return 0;
}
