#include <fstream>
#include "v2ray-ping.hpp"
#include <fcntl.h>
#include "error-handler.hpp"

using json = nlohmann::json;

static void v2ray_ping_start(json &outbound) {
    std::string protocol = outbound["protocol"];
    if (V2RAY_IGNORE(protocol)) return;
    V2RAY_PING(protocol, outbound);
}

static void help(const char *command, int code) {
    const char *format = "%8s %s\n";
    printf("Usage: %s [option]\n", command);
    printf(format, "-h", "print help information.");
    printf(format, "-c", "v2ray configuration file.");
    exit(code);
}

int main(int argc, char *const *argv) {
    try {
        const char *command = argv[0];
        int opt;
        const char *v2rayConfig = "/etc/v2ray/all-config.json";

        opterr = 0;

        while ((opt = getopt(argc, argv, "c:h")) != -1) {
            switch(opt) {
            case 'h':
                help(command, EXIT_SUCCESS);
                break;
            case 'c':
                v2rayConfig = optarg;
                break;
            default:
                printf("Unkown option: -%c\n", opt);
                help(command, EXIT_FAILURE);
                break;
            }
        }

        if (access(v2rayConfig, F_OK | R_OK)) {
            perror(v2rayConfig);
            return EXIT_FAILURE;
        }
        json config = json::parse(std::ifstream(v2rayConfig));
        if (config.contains("outbound")) v2ray_ping_start(config["outbound"]);
        if (config.contains("outbounds")) {
            json outbounds = config["outbounds"];
            for (json::iterator item = outbounds.begin(); item != outbounds.end(); item++) v2ray_ping_start(*item);
        }
        return EXIT_SUCCESS;
    } catch (std::exception &e) {
        std::cout << "\033[91m" << e.what() << "\033[0m" << std::endl;
        return EXIT_FAILURE;
    }
}
