#ifndef PROCESS_H
#define PROCESS_H

#include <unistd.h>
#include <memory>

using namespace std;

namespace v2ray_ping {
    class V2rayProcess {
        private:
            pid_t process_id;

        public:
            ~V2rayProcess();
            static shared_ptr<V2rayProcess> start(string config_path, string workdir);
            void stop();
    };
}

#endif
