#include <memory>
#include <vector>

enum request_type {
    // example over the line
    // ~> /temp/out P\r\n20\r\n\r\n
    // ~> /rfid P\r\n20\r\n\r\n
    ONELINER = 0,

    //more complex for quering multiple values. generate
    //I think i would implement something like HTTP1/1 so you could curl it...
    //It would be probable be used to send sensor history of a hour...
    Request_FILE = 1,
};

enum request_method {
    GET = 1,
    POST = 2,
    DELETE = 3
};

class request {
public:
    request(std::unique_ptr<char> raw);
    ~request();

    //bool checkValue()

private:
    enum request_type type;
    char *path;
    int path_len;

    char *method;
    char *start_of_body;
    std::unique_ptr<char> raw;
    int raw_len;
};
