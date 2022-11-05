#ifndef BP_TREE_DISK_CUSTOM_EXCEPTIONS_H
#define BP_TREE_DISK_CUSTOM_EXCEPTIONS_H

#include <cstring>
#include <cerrno>
#include <exception>
#include <string>

class unix_sys_error : public std::exception {
  std::string msg_;
public:
  explicit unix_sys_error(const std::string &msg = "")
      : msg_(msg + ": " + strerror(errno)) {}

  const char *what() const noexcept override { return msg_.c_str(); }
};

#endif //BP_TREE_DISK_CUSTOM_EXCEPTIONS_H
