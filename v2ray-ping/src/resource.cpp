#include "resource.hpp"
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <dirent.h>
#include <sys/wait.h>

using string = std::string;
using json = nlohmann::json;

namespace v2ray_ping {

    void rmdir_r(string &path) {
        DIR *dirp;
        struct dirent *sub_file;

        if (!(dirp = opendir(path.c_str()))) {
            perror(path.c_str());
            goto finally;
        }

        while((sub_file = readdir(dirp))) {
            if (
                !strcmp(sub_file->d_name, ".") ||
                !strcmp(sub_file->d_name, "..")
            ) continue;

            string sub_path = path + "/" + sub_file->d_name;
            if (sub_file->d_type == DT_DIR) {
                rmdir_r(sub_path);
            } else if (sub_file->d_type == DT_REG) {
                if (unlink(sub_path.c_str())) perror(sub_path.c_str());
            }
        }

    finally:
        if (dirp) closedir(dirp);
        if (rmdir(path.c_str()) == -1) perror(path.c_str());
    }

    static string tmp_dir;

    void exit_handler() {
        // After waiting for all child process to exit, clean up resources.
        wait(NULL);
        rmdir_r(tmp_dir);
    }

    string get_tmp_dir() {
        if (tmp_dir.length()) return tmp_dir;
        char template_str[TMP_MAX] = P_tmpdir "/v2ray-ping-XXXXXX";

        tmp_dir.append(mkdtemp(template_str));

        atexit(exit_handler);
        signal(SIGHUP, exit);
        signal(SIGINT, exit);
        signal(SIGQUIT, exit);
        signal(SIGSTOP, exit);
        signal(SIGTERM, exit);

        return tmp_dir;
    }

    json get_target_config() {
        json target_config = R"(
        {
          "log": {
            "loglevel": "info"
          },
          "inbound": {
            "port": 8080,
            "protocol": "socks",
            "settings": {
              "auth": "noauth"
            },
            "streamSettings": {
              "network": "domainsocket",
              "dsSettings": {
              }
            }
          }
        }
        )"_json;

        std::string tmp_sock = get_tmp_dir();
        tmp_sock.append("/v2ray.sock");
        target_config["inbound"]["streamSettings"]["dsSettings"]["path"] = tmp_sock;

        return target_config;
    }
}
