#include <cstring>
#include <iostream>
#include <stdio.h>
#include "socket_class.h"
#include <optional>
#include <strings.h>



int main(int argc, char *argv[])
{

    socket_class socket = socket_class(6789);


    while(true) {
        auto newsocket =  socket.accept_new_host();

        std::cout << newsocket.get_msg().value();
        newsocket.send_response('2','1', "Hi peter", 9 );

        // newsocket goes out of scope. destructor is called. 
    }

    return 0;
}
