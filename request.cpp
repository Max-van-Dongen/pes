#include "request.h"

request::request(std::unique_ptr<char> raw_in):raw(raw_in) {
}
