#include <csignal>
#include <cstring>
#include <thread>
#include<unistd.h>
#include <iostream>
#include "socket_class.h"
#include <optional>
#include <strings.h>
#include <vector>

#include "requestStore.h"

void teardown(int);
std::vector<std::thread> processes;

void handleClient(socket_class *fun) {
        
    while (true) {

        std::optional<std::string> opt = fun->get_msg();

        if (!opt.has_value()) {
            continue;;
        }

        std::string msg = opt.value();

        char requestor = msg[0];
        char dest = msg[1];
        const char *data = 2+msg.c_str();

        printf("Got '%s' for '%c' from '%c'\r\n", data, dest, requestor);

        requestMap.at(dest).append(data);

        const char *resp = requestMap.at(requestor).c_str();
        
        fun->send_response('1', '2', resp, strlen(resp));

        requestMap.at(requestor).clear();
    }
}

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

    } else {
        std::cout << "invalid args\r\n";
    }


    while(true) {

        socket_class *newsocket(new  (socket_class) (  socket_c->accept_new_host() ) );

        newsocket->debug_print();
        try {
            std::thread(handleClient, newsocket).detach();
        } catch (std::exception err) {
            std::cout << "err:" << err.what() << std::endl;
        }

        // newsocket goes out of scope. so socket get closed
    }

    delete socket_c;
    return 0;
}


void teardown(int signal) {
    printf("\r\nGot sigint. Exiting...\r\n");
    delete socket_c;

    exit(0);
}
