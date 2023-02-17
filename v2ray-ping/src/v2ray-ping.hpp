#ifndef V2RAY_PING_H
#define V2RAY_PING_H

#include <iostream>
#include <unistd.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// v2ray protocol, https://www.v2fly.org/config/protocols/blackhole.html
#define V2RAY_BLACKHOLE     "blackhole"
#define V2RAY_DNS           "dns"
#define V2RAY_DOKODEMO_DOOR "dokodemo-door"
#define V2RAY_FREEDOM       "freedom"
#define V2RAY_HTTP          "http"
#define V2RAY_SOCKS         "socks"
#define V2RAY_VMESS         "vmess"
#define V2RAY_SHADOWSOCKS   "shadowsocks"
#define V2RAY_TROJAN        "trojan"
#define V2RAY_VLESS         "vless"
#define V2RAY_LOOPBACK      "loopback"

// v2ray protocol to ignore.
#define V2RAY_IGNORE(protocol) \
    protocol == V2RAY_BLACKHOLE || \
    protocol == V2RAY_DNS || \
    protocol == V2RAY_DOKODEMO_DOOR || \
    protocol == V2RAY_FREEDOM || \
    protocol == V2RAY_LOOPBACK

// According to the v2ray protocol, select the test method of the node.
#define V2RAY_PING(protocol, outbound) \
    if (protocol == V2RAY_VMESS) { \
        v2ray_ping::vmess_ping(outbound); \
    } else { \
        throw std::runtime_error("unsupported protocol: " + protocol); \
    }

namespace v2ray_ping {
    void vmess_ping(json & outbound);
}

#endif
