#include <csignal>
#include <cstring>
#include<unistd.h>
#include <iostream>
#include "socket_class.h"
#include <optional>
#include <strings.h>

#include "requestStore.h"

void teardown(int);

socket_class *socket_c = 0;
int main(int argc, char **argv) {
    signal(SIGINT, *teardown); // handle <ctrl-c>


    // argc == 1 is no args given
    // argv[0] is the command used to call this app 
    if (argc == 1) {
        socket_c = new socket_class(6789);
    } else if (argc == 2) {

        // server demo
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
        auto newsocket =  socket_c->accept_new_host();

        std::optional<std::string> opt = newsocket.get_msg();

        if (!opt.has_value()) {
            continue;
        }

        std::string msg = opt.value();

        char requestor = msg[0];
        char dest = msg[1];
        const char *data = 2+msg.c_str();

        printf("Got '%s' for '%c' from '%c'\r\n", data, dest, requestor);

        requestMap.at(dest).append(data);

        const char *resp = requestMap.at(requestor).c_str();
        
        newsocket.send_response('1', '2', resp, strlen(resp));

        requestMap.at(requestor).clear();


        // newsocket goes out of scope. so socket get closed
    }

    delete socket_c;
    return 0;
}

void teardown(int signal) {
    printf("\r\nGot sigint. Exiting...\r\n");
    delete socket_c;

    usleep(2000); // In microseconds (1000000 is 1 sec)
                  // Needed because otherwise linux thinks the port is still listening
                  // while the server isn't running...
    exit(0);
}
