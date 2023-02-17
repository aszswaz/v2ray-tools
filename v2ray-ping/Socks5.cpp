#include "Socks5.hpp"
#include "error-handler.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define SOCKS_VERSION 5

#define SEND(buff, len) STAND_ERROR(send(this->sock, buff, len, 0) == -1);
#define READ(buff, len) STAND_ERROR(recv(this->sock, buff, len, 0) == -1);

namespace v2ray_ping {
    /**
     * socks 5 authentication method.
     */
    enum SOCKS5_AUTH {
        // No authentication
        NO_AUTH = 0,
        // GSSAPI, https://datatracker.ietf.org/doc/html/rfc1961
        GSSAPI = 1,
        // Username/password, https://datatracker.ietf.org/doc/html/rfc1929
        USER_PWD = 2,
        // Challenge-Handshake Authentication Protocol
        CHAP = 3,
        // Unassigned
        UNASSIGNED = 4,
        // Challenge-Response Authentication Method
        CRAM = 5,
        // Secure Sockets Layer
        SECURE_SOCK = 6,
        // NDS Authentication
        NDS = 7,
        // Multi-Authentication Framework
        MULTI = 8,
        // JSON
        JSON = 9
    };

    enum cmd {
        // establish a TCP/IP stream connection
        TCP = 1,
        // establish a TCP/IP port binding
        TCP_PORT = 2,
        // associate a UDP port
        UDP = 3
    };

    /**
     * Client greeting
     */
    struct greeting {
        // socks version
        const char version = SOCKS_VERSION;
        // The number of authentication methods, one byte for each authentication method.
        const char nauth = 1;
        // verification method.
        const char auth = NO_AUTH;
    };

    /**
     * The server's handshake response.
     */
    struct server_choice {
        char version;
        // The authentication method selected by the server.
        char cauth;
    };

    /**
     * Error code information for socks5.
     */
    static const char *status_message(int code) {
        switch(code) {
        case 0:
            return "request granted";
        case 1:
            return "general failure";
        case 2:
            return "connection not allowed by ruleset";
        case 3:
            return "network unreachable";
        case 4:
            return "host unreachable";
        case 5:
            return "connection refused by destination host";
        case 6:
            return "TTL expired";
        case 7:
            return "command not supported / protocol error";
        case 8:
            return "address type not supported";
        default:
            return "unknown mistake.";
        }
    }

    void Socks5::setAddr(string addr) {
        this->addr = addr;
        char *buff[INET6_ADDRSTRLEN];
        // Probe address type.
        if (inet_pton(AF_INET, addr.c_str(), buff) == 1) {
            this->addrtype = IPV4;
        } else if (inet_pton(AF_INET6, addr.c_str(), buff)) {
            this->addrtype = IPV6;
        } else {
            this->addrtype = DOMAINNAME;
        }
    };

    void Socks5::setPort(uint16_t port) {
        // The byte order is converted to network byte order, that is, little endian.
        this->port = htons(port);
    };

    int Socks5::getSocket() {
        return this->sock;
    };

    std::shared_ptr<Socks5> Socks5::unix_conn(string unix_path) {
        shared_ptr<Socks5> socks5 = make_shared<Socks5>();
        int sock, code;
        struct sockaddr_un addr {
            AF_UNIX, ""
        };

        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        STAND_ERROR(sock == -1);
        socks5->sock = sock;
        strcpy(addr.sun_path, unix_path.c_str());
        code = connect(sock, (sockaddr *)(&addr), sizeof(addr));
        STAND_ERROR(code == -1);
        return socks5;
    };

    void Socks5::sock_conn() {
        struct greeting g {};
        struct server_choice choice {};
        char buff[BUFSIZ];
        int i;
        int code;

        SEND(&g, sizeof(greeting));
        READ(&choice, sizeof(server_choice));
        if (choice.version != g.version || choice.cauth != g.auth) {
            MESSAGE_ERROR("socks5 handshake failed");
        }

        // Send a proxy request.
        i = 0;
        memset(buff, 0, BUFSIZ);
        buff[i++] = SOCKS_VERSION, buff[i++] = TCP, buff[i++] = 0, buff[i++] = addrtype;
        switch (addrtype) {
        case IPV4:
            code = inet_pton(AF_INET, addr.c_str(), (void *)&buff[i]);
            if (code) {
                MESSAGE_ERROR("invalid IPV4 address");
            } else if (code == -1) {
                MESSAGE_ERROR(strerror(errno));
            }
            i += INET_ADDRSTRLEN;
            break;
        case DOMAINNAME:
            buff[i++] = addr.length();
            memcpy((void *)(&buff[i]), addr.c_str(), addr.length());
            i += addr.length();
            break;
        case IPV6:
            code = inet_pton(AF_INET6, addr.c_str(), (void *)&buff[i]);
            if (code == 0) {
                MESSAGE_ERROR("invalid IPV6 address");
            } else if (code == -1) {
                MESSAGE_ERROR(strerror(errno));
            }
            i += INET6_ADDRSTRLEN;
            break;
        default:
            MESSAGE_ERROR("Unknown address type.");
        }
        *((uint16_t *)&buff[i]) = port;
        i += sizeof(uint16_t);
        SEND(buff, i);

        // Read the response and check whether the connection to the target server was successfully established.
        i = 0;
        memset(buff, 0, BUFSIZ);
        READ(buff, BUFSIZ);
        if (buff[i++] != SOCKS_VERSION) MESSAGE_ERROR("Unknown socks protocol version.");
        char status = buff[i++];
        if (status != 0) MESSAGE_ERROR(status_message(status));
    }

    Socks5::~Socks5() {
        close(this->sock);
    }
}
