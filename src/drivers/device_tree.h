#pragma once

#include <memory>
#include <unordered_map>
#include "device.h"

namespace drivers
{

    class DeviceTree
    {
        static inline std::unordered_map<std::string, std::shared_ptr<Device>> registered_{};

       public:
        static void add(const std::string& name, std::shared_ptr<Device> dev) { registered_[name] = dev; }

        static auto registered() { return registered_; }
    };

}  // namespace drivers