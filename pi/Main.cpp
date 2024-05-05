#include <csignal>
#include<unistd.h>
#include <iostream>
#include "socket_class.h"
#include <optional>
#include <strings.h>



int main(int argc, char **argv) {
    socket_class *socket = 0;


    // argc == 1 is no args given
    // argv[0] is the command used to call this app 
    if (argc == 1) {
        socket = new socket_class(6789);
    } else if (argc == 2) {
        while (true) {
            socket_class a = socket_class(argv[1], 6789);
            a.send_response('1','2',"abc", 4);
            std::cout << a.get_msg().value() << std::endl;
        }
        exit(0);

    } else if (argc == 3) {
    } else {
        std::cout << "invalid args\r\n";
    }


    while(true) {
        auto newsocket =  socket->accept_new_host();

        auto msg = newsocket.get_msg();

        if (msg.has_value()) {
            std::cout << msg.value() << std::endl;
            newsocket.send_response('2','1', "Hi peter", 9 );
        }
        // newsocket goes out of scope. so socket get closed
    }

    delete socket;
    return 0;
}
