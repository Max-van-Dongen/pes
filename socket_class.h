#include <cstdint>
#include <optional>
#include <exception>
#include <string>

typedef char msg;

enum socket_state {
    ESTABLISHED = 1,
    listening = 0,
    Error = -1,
};

class socket_class {
public:
    /// readable version of ip addr
    char ipaddr[20];
    int port;

    // create a tcp listening port
    socket_class(char ip[20], int port);
    
    // create a tcp listening port to available port av broadcast
    socket_class(int port);

    std::optional<socket_class> accept_new_host();

    // get the next msg from the kernel
    std::optional<std::string> get_msg() noexcept; 

    int send_response(uint8_t statuscode, const char *buff, int len);

    void read_msg();

    void debug_print();

private:
    int fp;
    socket_state state = listening;

    socket_class(int fp, socket_state s):
        state(s),
        fp(fp)
    {
     
    }
};
