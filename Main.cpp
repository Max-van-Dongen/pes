#include <cstring>
#include <iostream>
#include "socket_class.h"
#include <optional>



int main(int argc, char *argv[])
{
    char ip[20] = "0.0.0.0"; // i can't make an inline ip declaration because it things it a char pointer
    socket_class socket( ip, 4000); 

    std::optional<socket_class> first = socket.accept_new_host();

    while(true) {

        // TODO make it non-blocking
        std::optional<std::string> a = first->get_msg();

        if (a.has_value() ) {
            std::cout << a.value();

        }

        std::string resp = a.value_or("error at parsing");
        first->send_response(200, resp.c_str(), resp.length());

    }

    return 0;
}
