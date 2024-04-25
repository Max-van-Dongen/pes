#include <cstring>
#include <iostream>
#include <stdio.h>
#include "socket_class.h"
#include <optional>
#include <strings.h>



int main(int argc, char *argv[])
{
    socket_class *socketptn;

    if (argc > 1) {
        // connect to the ip addr given as the first cli arg
        socketptn = new socket_class(argv[1],4000);
        char why_am_i_here_today[] =  "Among the mist I see a stretching path,\r\nreaches out so far, beyond what I can see\r\nHow far would I be walking 'til I reach the end,\r\nthe end of the path that's to be seen?\r\n\r\nI would keep thinking and questioning to myself\r\nwhy do I have to walk this path, oh why?\r\nI can't seem to find the real meaning to this path that I take, fate to accept, and my goal seems so far away";
        socketptn->send_response(2,why_am_i_here_today, strlen(why_am_i_here_today));
    
    } else {
        // listen to port 4000 at ip 0.0.0.0 and accept the first host ignoring errors
        socketptn = new (socket_class) (socket_class(4000).accept_new_host().value());
    }

    socket_class socket = *socketptn;

    while(true) {

        std::optional<std::string> in_opt = socket.get_msg();

        if (in_opt.has_value() == false &&
                errno != 0) {
            printf("sending errored with errno %d\r\n", errno);
            socket.debug_print();
            while (true) {}
        }

        if (in_opt.has_value() == false) {
            continue;
        }

        std::string in_val = in_opt.value();

        std::cout << in_val;
        socket.send_response(200, in_val.c_str(), in_val.length());

    }

    return 0;
}
