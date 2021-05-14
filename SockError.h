#pragma once
#include <stdexcept>
#include "SockPlatform.h"

class SockError : public std::runtime_error {
public:
    const int error_code;
    const void* const func;

    template <typename Func>
    SockError(const char* what, Func func, int error_code)
        : runtime_error(std::string(what) + " [" + std::to_string(error_code) + "]"),
          error_code(SockPlatform::get_errno()),
          func((void*)func) { }
};

