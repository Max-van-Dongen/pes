#include <iostream>
#include "socket_class.h"
#include <optional>



int main(int argc, char *argv[])
{
    char ip[20] = "0.0.0.0"; // i can't inline the ip declaration because it things it a char pointer
    socket_class socket( ip, 4000); 

    std::optional<socket_class> first = socket.accept_new_host();

    if (!first.has_value()) {
        std::cout << "got something";
    }

    while(true) {
        std::optional<std::string> a = first->get_msg();

	if (a.has_value() ) {
		std::cout << a.value();

	}

    }

    return 0;
}
