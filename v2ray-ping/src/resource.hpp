#ifndef RESOURCE_H
#define RESOURCE_H

#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace v2ray_ping {

    std::string get_tmp_dir();

    json get_target_config();
}

#endif
