#include "V2rayProcess.hpp"
#include "error-handler.hpp"
#include <signal.h>

using namespace std;

namespace v2ray_ping {
    V2rayProcess::~V2rayProcess() {
        this->stop();
    }

    shared_ptr<V2rayProcess> V2rayProcess::start(string config_path, string workdir) {
        shared_ptr<V2rayProcess> process = make_shared<V2rayProcess>();
        pid_t sub_pid;

        sub_pid = fork();
        STAND_ERROR(sub_pid == -1);

        if (sub_pid == 0) {
            STAND_ERROR(chdir(workdir.c_str()) == -1);
            STAND_ERROR(execlp("v2ray", "v2ray", "run", "-config", config_path.c_str(), NULL));
        } else {
            process->process_id = sub_pid;
        }

        return process;
    }

    void V2rayProcess::stop() {
        if (this->process_id > 0) {
            STAND_ERROR(kill(this->process_id, SIGTERM) == -1);
        }
    }
}
