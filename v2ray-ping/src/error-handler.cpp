#include "error-handler.hpp"
#include <stdexcept>

using string = std::string;

void message_error(const char *filename, int line, string msg) {
    string err_msg(filename);
    err_msg = err_msg + " " + std::to_string(line) + ": " + msg;
    throw std::runtime_error(err_msg);
}
