#include "HttpClient.hpp"
#include "error-handler.hpp"
#include "Socks5.hpp"

using namespace std;

#define CURL_SET_OPT(opt, value) \
    code = curl_easy_setopt(this->curl, opt, value); \
    if (code != CURLE_OK) { \
        MESSAGE_ERROR(curl_easy_strerror(code)); \
    }

#define CURL_URL_GET(opt, value) \
    CURLUcode code = curl_url_get(this->url, opt, value, 0); \
    if (code != CURLUE_OK) { \
        MESSAGE_ERROR(curl_url_strerror(code)); \
    }


namespace v2ray_ping {
    class URLParser {
        private:
            CURLU *url;

        public:
            URLParser(string url) {
                CURLUcode code;
                this->url = curl_url();
                code = curl_url_set(this->url, CURLUPART_URL, url.c_str(), 0);
                if (code != CURLUE_OK) MESSAGE_ERROR(curl_url_strerror(code));
            }
            ~URLParser() {
                curl_url_cleanup(this->url);
            }
            string getHost() {
                char *host;
                CURL_URL_GET(CURLUPART_HOST, &host);
                return string(host);
            }
            uint16_t getPort() {
                char *port;
                CURLUcode code;
                char *protocol;

                code = curl_url_get(this->url, CURLUPART_PORT, &port, 0);
                if (code == CURLUE_OK) {
                    return (uint16_t) strtol(port, nullptr, 10);
                } else if (code == CURLUE_NO_PORT) {
                    CURL_URL_GET(CURLUPART_SCHEME, &protocol);
                    if (!strcmp(protocol, "http"))
                        return 80;
                    else if (!strcmp(protocol, "https"))
                        return 443;
                    else
                        MESSAGE_ERROR((string) "Unknown protocol: " + protocol);
                } else {
                    MESSAGE_ERROR(curl_url_strerror(code));
                }
                return -1;
            }
    };

    static curl_socket_t opensocket_callback(
        void *clientp, curlsocktype purpose, struct curl_sockaddr *address
    ) {
        Socks5 *socks5 = (Socks5 *) clientp;
        return (curl_socket_t) socks5->getSocket();
    }

    static int sockopt_callback(
        void *clientp, curl_socket_t curlfd, curlsocktype purpose
    ) {
        return CURL_SOCKOPT_ALREADY_CONNECTED;
    }

    static int closecb(void *clientp, curl_socket_t item) {
        return 0;
    }

    size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
        // fwrite(ptr, size, nmemb, stdout);
        return size * nmemb;
    }

    HttpClient::HttpClient() {
        this->curl = curl_easy_init();
    }

    HttpClient::~HttpClient() {
        curl_easy_cleanup(this->curl);
    }

    void HttpClient::setSocks5(shared_ptr<Socks5> socks5) {
        this->socks5 = socks5;
    }

    void HttpClient::GET(string url) {
        CURLcode code;

        CURL_SET_OPT(CURLOPT_URL, url.c_str());
        CURL_SET_OPT(CURLOPT_HTTPGET, 1L);
        // Set the timeout time in seconds.
        CURL_SET_OPT(CURLOPT_TIMEOUT, 20L);
        CURL_SET_OPT(CURLOPT_NOPROGRESS, 1L);
        CURL_SET_OPT(CURLOPT_NOPROXY, "*");
        CURL_SET_OPT(CURLOPT_WRITEFUNCTION, write_callback);
        CURL_SET_OPT(CURLOPT_WRITEDATA, nullptr);

        URLParser parser(url);
        this->socks5->setAddr(parser.getHost());
        this->socks5->setPort(parser.getPort());
        this->socks5->sock_conn();

        // Pass the socket to curl.
        CURL_SET_OPT(CURLOPT_OPENSOCKETFUNCTION, opensocket_callback);
        CURL_SET_OPT(CURLOPT_OPENSOCKETDATA, this->socks5.get());
        CURL_SET_OPT(CURLOPT_SOCKOPTFUNCTION, sockopt_callback);
        CURL_SET_OPT(CURLOPT_CLOSESOCKETFUNCTION, closecb);
        CURL_SET_OPT(CURLOPT_CLOSESOCKETDATA, this->socks5.get());

        code = curl_easy_perform(this->curl);
        if (code != CURLE_OK) MESSAGE_ERROR(curl_easy_strerror(code));
    }
}
