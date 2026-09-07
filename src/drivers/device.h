#pragma once
#include <atomic>
#include <string>

namespace drivers {

class Device {
  /// @brief file descriptor, just in case
  std::atomic_int fd_;
  /// @brief total fd number
  static inline std::atomic_int total_fd_ = 0;
  /// @brief name of the device
  const std::string name_;

protected:
  std::mutex mutex_;

public:
  Device(const std::string &name) : name_{name} {
    total_fd_++;
    fd_.store(total_fd_);
  }

  virtual ~Device() { total_fd_--; }

  std::mutex& mutex() { return mutex_; }

  int fd() const { return fd_; }
};

} // namespace drivers