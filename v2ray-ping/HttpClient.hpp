#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <memory>
#include "Socks5.hpp"
#include <curl/curl.h>

using namespace std;

namespace v2ray_ping {

    class HttpClient {
        private:
            shared_ptr<Socks5> socks5;
            CURL *curl;

        public:
            HttpClient();
            ~HttpClient();
            void setSocks5(shared_ptr<Socks5> socks5);
            void GET(string url);
    };

}

#endif
