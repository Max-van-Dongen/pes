#include "socket_class.h"

#include <iostream>
#include <string.h>

// for socket and ip things
// also includes sys/socket.h
// man://ip(7)
#include <netinet/ip.h>

// if syscalls fails you need te separately.
// see:
// man://errno(3) 
//#include <errno.h>

// for translating to net byte order
// documentation: man://inet(3)
#include <arpa/inet.h>

// but i want to use <expected> but that is c++ 23 :(
#include <optional>


// documentation lookup
// inet_addr see `man://inet(3)`
// sockaddr_in man://sockaddr(3type)
//
socket_class::socket_class(int port) {
    char ip[20] = "0.0.0.0";
    socket_class( ip, 1222);
}

socket_class::socket_class(char ip[20], int port) {


   uint bin_ip_addr = inet_addr(ip);

    if (bin_ip_addr == INADDR_NONE) 
    {
        throw std::runtime_error("please enter a valid ip addr");
    }

    struct sockaddr_in dest = {
        AF_INET,
        htons(4000), // Socket in network byte order (I think big endian)
        bin_ip_addr
    };

    int sockfp = socket(AF_INET,SOCK_STREAM,0);

    if (sockfp == 0) {
        std::cout << "socket is down";
    }

    if (
        bind(sockfp, (struct sockaddr*) &dest, sizeof dest)
    )
    {
        throw std::runtime_error("failed to bind address");
    }

    strcpy(ipaddr, ip);
    this->port = port;
    this->fp = sockfp;

    listen(fp, 3);
}


// logic of accepting new clients
// TODO: politely close the socket at rejection. 
std::optional<socket_class> socket_class::accept_new_host() {
    uint fd = accept(fp, NULL, NULL);

    char buff[300] = {0};
    int rec = recv(fd, buff, 299, 0); // one removed because of trailing NULL

    if (0 > rec) {
        std::cout << "hi! I didn't got a msg while opening a connection";
        return {};
    }

    printf("%s", buff);

    return socket_class(fd, ESTABLISHED);
}

std::optional<std::string> socket_class::get_msg() noexcept {
	char buff[100];

	int bytes_received = recv(this->fp, buff, 99, 0);
	if (bytes_received > 0) {
		buff[bytes_received] = '\0'; // Null-terminate the string
		return  std::string(buff);
	}
	return  std::nullopt;
}

int socket_class::send_response(uint8_t statuscode, const char *buff, int len) {
    std::string msg = std::string("<~ ");
    msg.append(std::to_string(statuscode));
    msg.append("\r\n");
    msg.append(buff, len);
    msg.append("\r\n\r\n");

    return send(fp, msg.c_str(), msg.length() ,0);
}
