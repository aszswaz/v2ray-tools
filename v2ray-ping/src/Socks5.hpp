#ifndef V2RAY_PROXY_H
#define V2RAY_PROXY_H

#include <string>
#include <memory>

using string = std::string;

namespace v2ray_ping {
    enum address_type {
        IPV4 = 1,
        DOMAINNAME = 3,
        IPV6 = 4
    };

    class Socks5 {
        private:
            /**
             * Socket with socks5 server.
             */
            int sock;
            /**
             * Target server address.
             */
            string addr;
            /**
             * Address type, IPV4, domain name or IPV6.
             */
            address_type addrtype;
            /**
             * Target server port.
             */
            uint16_t port;

        public:
            ~Socks5();
            void setAddr(string addr);
            void setPort(uint16_t port);
            int getSocket();
        public:
            /**
             * Establish a socks5 connection over a domain socket.
             */
            static std::shared_ptr<Socks5> unix_conn(string unix_path);
            /**
             * Send socks5 proxy request.
             */
            void sock_conn();
    };
}
#endif
