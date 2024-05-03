#include <cstring>
#include<unistd.h>
#include <iostream>
#include <stdio.h>
#include "socket_class.h"
#include <optional>
#include <strings.h>



int main(int argc, char **argv) {
    socket_class *socket = 0;

    printf("%i\r\n", argc);

    if (argc == 1) {
        socket = new socket_class(6789);
    } else if (argc == 2) {
        std::cout << "aaa\r\n";
        while (true) {
            socket_class a = socket_class(argv[1], 6789);
            std::cout << "kwaa\r\n";
            a.send_response('1','2',"abc", 4);
            usleep(4000000);
        }
        exit(0);

    } else if (argc == 3) {
    } else {
        std::cout << "invalid args\r\n";
    }


    while(true) {
        auto newsocket =  socket->accept_new_host();

        std::cout << newsocket.get_msg().value();
        newsocket.send_response('2','1', "Hi peter", 9 );

        // newsocket goes out of scope. so socket get closed
    }

    return 0;
}
