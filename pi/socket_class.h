#include <cstdint>
#include <optional>
#include <exception>
#include <string>

typedef char msg;

//deprecated
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

    // connect to a "server"
    socket_class(char ip[20], int port);
    
    // create a tcp listening port to all available ip addresses
    socket_class(int port);

    socket_class accept_new_host() noexcept;

    // get the next msg from the kernel
    std::optional<std::string> get_msg(); 

    int send_response(char self, char requestType, const void *buff, int len) noexcept;

    ~socket_class();

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
