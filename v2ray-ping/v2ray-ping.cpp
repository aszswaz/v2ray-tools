#include "v2ray-ping.hpp"
#include "resource.hpp"
#include "error-handler.hpp"
#include <fstream>
#include <sys/wait.h>
#include "HttpClient.hpp"
#include "V2rayProcess.hpp"
#include <sys/time.h>

using json = nlohmann::json;
using string = std::string;
using runtime_error = std::runtime_error;

namespace v2ray_ping {

    /**
     * Test nodes using the vmess protocol.
     */
    void vmess_ping(json &outbound) {
        string node_ip;

        json nodes = outbound["settings"]["vnext"];
        json target_outbound = json(outbound);
        json vnext = json::array();
        json target_config = get_target_config();
        string conf_path = get_tmp_dir() + "/config.json";

        std::cout << "start testing the vmess node..." << std::endl;

        for(json::iterator item = nodes.begin(); item != nodes.end(); item++) {
            try {
                node_ip = (*item)["address"].get<string>();
                std::cout << "\033[92mnode ip: " << node_ip << "\033[0m" << std::endl;
                vnext[0] = *item;
                target_outbound["settings"]["vnext"] = vnext;
                target_config["outbound"] = target_outbound;
                string json_str = target_config.dump();

                std::ofstream conf_file(conf_path);
                conf_file.write(json_str.c_str(), json_str.length());
                conf_file.close();

                // start v2ray
                shared_ptr<V2rayProcess> process = V2rayProcess::start(conf_path, get_tmp_dir());
                // Wait for v2ray to start up.
                sleep(1);

                HttpClient client;
                string sock_path = target_config["inbound"]["streamSettings"]["dsSettings"]["path"];

                struct timeval tv {};
                uint64_t start_time{}, end_time{};
                STAND_ERROR(gettimeofday(&tv, nullptr) == -1);
                start_time = tv.tv_sec * 1000 * 1000 * 1000 + tv.tv_usec;

                client.setSocks5(Socks5::unix_conn(sock_path));
                client.GET("https://www.google.com");

                STAND_ERROR(gettimeofday(&tv, nullptr) == -1);
                end_time = tv.tv_sec * 1000 * 1000 * 1000 + tv.tv_usec;
                process->stop();
                STAND_ERROR(unlink(sock_path.c_str()) == -1);

                uint64_t second{}, millisecond{}, microseconds{}, nanosecond{};
                nanosecond = end_time - start_time;
                microseconds = nanosecond / 1000;
                millisecond = microseconds / 1000;
                second = millisecond / 1000;
                printf(
                    "\033[92mnode ip: %s, time cost: %lu/s %lu/ms %lu/μs %lu/ns.\033[0m\n",
                    node_ip.c_str(),
                    second, millisecond % 1000, microseconds % 1000, nanosecond % 1000
                );
            } catch (std::exception &e) {
                std::cout << "\033[91m" << "node ip: " << node_ip << "; " << e.what() << "\033[0m" << std::endl;
            }
        }
    }
}
