#include <iostream>
#include "socket_class.h"
#include <optional>



int main(int argc, char *argv[])
{
    // make a listening port at 0.0.0.0 with port (int)
    socket_class socket(4000); 

    std::optional<socket_class> first = socket.accept_new_host();

    // TODO 
    // Check if there is an other server up.
    // If yes connect to server.

    while(true) {

        // TODO make it non-blocking AND make it return a class
        std::optional<std::string> in_opt = first->get_msg();

        if (in_opt.has_value() ) {
            std::string in_val = in_opt.value();
            std::cout << in_val;
            first->send_response(200, in_val.c_str(), in_val.length());
        }

    }

    return 0;
}
