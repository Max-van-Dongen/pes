#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <cstring> // Include for strerror
#define I2C_BUS "/dev/i2c-1" // Replace with the actual I2C bus device file path if different

int main() {
  int fd; // File descriptor for the I2C device
  uint8_t reg_address = 0x12; // Register address to read from
  uint8_t data_buffer[50]; // Buffer to store read data (adjust size as needed)

  // Open the I2C device file
  fd = open(I2C_BUS, O_RDWR);
  if (fd < 0) {
    std::cerr << "Error opening I2C device: " << strerror(errno) << std::endl;
    return 1;
  }

  // Set the slave address
  if (ioctl(fd, I2C_SLAVE, 0x12) < 0) {
    std::cerr << "Error setting slave address: " << strerror(errno) << std::endl;
    close(fd);
    return 1;
  }

  // Write the register address to be read
  if (write(fd, &reg_address, 1) != 1) {
    std::cerr << "Error writing register address: " << strerror(errno) << std::endl;
    close(fd);
    return 1;
  }

  // Read data from the register
  if (read(fd, data_buffer, sizeof(data_buffer)) != sizeof(data_buffer)) {
    std::cerr << "Error reading data: " << strerror(errno) << std::endl;
    close(fd);
    return 1;
  }

  // Process the received data
  std::cout << "Read " << sizeof(data_buffer) << " bytes from I2C device:" << std::endl;
  for (int i = 0; i < sizeof(data_buffer); ++i) {
    std::cout << std::hex << "0x" << (int)data_buffer[i] << " ";
  }
  std::cout << std::endl;

  // Close the I2C device file
  close(fd);

  return 0;
}
