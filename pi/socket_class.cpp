#include "socket_class.h"

//printing to stdout
#include <iostream>
#include <stdio.h>

#include <string.h>

// for close()
#include <unistd.h>

// for socket and ip things
// also includes sys/socket.h
// man://ip(7)
#include <netinet/ip.h>

// if syscalls fails you need te separately.
// see:
// man://errno(3) 
#include <errno.h>

// for translating to net byte order
// documentation: man://inet(3)
#include <arpa/inet.h>

// but i want to use <expected> but that is in c++ 23 :(
#include <optional>


socket_class::socket_class(int port) {
    char ip[] = "0.0.0.0";

    // inet_addr see `man://inet(3)`
    uint bin_ip_addr = inet_addr(ip);

    if (bin_ip_addr == INADDR_NONE) 
    {
        throw std::runtime_error("please enter a valid ip addr");
    }

    // sockaddr_in man://sockaddr(3type)
    struct sockaddr_in dest = {
        AF_INET,
        htons(port),
        bin_ip_addr
    };

    int sockfp = socket(AF_INET,SOCK_STREAM,0);

    if (sockfp == 0) {
        char err[50];
        snprintf(err, 49, "Failed to create socket. Errno: %d", errno);
        throw std::runtime_error(err);
    }

    if ( bind(sockfp, (struct sockaddr*) &dest, sizeof dest) ) {
        char err[50];
        snprintf(err, 49, "failed to bind addr. Errno: %d", errno);
        throw std::runtime_error(err);
    }

    strcpy(ipaddr, ip);
    this->port = port;
    this->fp = sockfp;

    listen(fp, 9);
    printf("(socket) listening at %s:%d with fd %d\r\n", ipaddr, port, sockfp);
}

socket_class::socket_class(char ip[20], int port) {
    uint bin_ip_addr = inet_addr(ip);

    if (bin_ip_addr == INADDR_NONE) 
    {
        throw std::runtime_error("please enter a valid ip addr");
    }

    struct sockaddr_in dest = {
        AF_INET,
        htons(port), // Socket in network byte order (I think big endian)
        bin_ip_addr
    };

    int sockfp = socket(AF_INET,SOCK_STREAM,0);

    if (sockfp == -1) {
        char err[40];
        snprintf(err, 39, "failed to create socket. Errno: %d", errno);
        throw std::runtime_error(err);
    }


    int status = connect(sockfp, (struct sockaddr*) &dest, sizeof(dest));              

    if (status) {
        char resp[50];

        switch (errno) {
            case ECONNREFUSED:
                snprintf(resp, 49, "No listening port fount at %s:%d",ip,port);
                throw std::runtime_error(resp);
                break;

            default:
                snprintf(resp, 49, "connection failed with errno %d", errno);
                throw std::runtime_error(resp);
                break;
        }
    }

    this->fp = sockfp;

    printf("(socket) connected with %s:%d with fd %d\r\n", ip, port, sockfp);
}


socket_class socket_class::accept_new_host() noexcept {
    uint fd = accept(fp, NULL, NULL);

    if (fd == -1) {
        switch(errno) {
            default:
                printf("unknown error errno: %i \r\n", errno);
                break;
            case EBADF:
                printf("Socket closed :(\r\n");
                exit(3);
                break;
        }
    }

    return socket_class(fd, ESTABLISHED);
}

std::optional<std::string> socket_class::get_msg() noexcept {
	char buff[600];
	int bytes_received = recv(this->fp, buff, 99, 0);

	if (bytes_received > 0) {
		buff[bytes_received] = '\0'; // Null-terminate the string
		return  std::string(buff);
	}
	return  std::nullopt;
}

int socket_class::send_response(char self, char datatype = 0, const void *buff = 0, int len = 0) noexcept{
    std::string data = std::string();
    data.append({self});

    if (len == 0) {
        data.append("1");

    } else {
        data.append("2");
        data.append( (char *) buff);
        data.append({0});
    }

    int status = send(fp,data.c_str(),data.length(),0);
    printf("(socket) send %s\r\n", (char *) buff);
    return status;
}

socket_class::~socket_class() {
    printf("(socket) closing %i\r\n", fp);
    close(this->fp);
}

void socket_class::debug_print() {
    printf("Debug socket %d\r\n", fp);
}
