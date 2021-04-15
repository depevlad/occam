#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <stdexcept>

class BException final : public std::exception {
  public:
  	std::string msg;

  	template <typename ...Args>
  	BException(const char *fmt, Args const& ...args) noexcept {
  		char *buf;

  		msg = asprintf(&buf, fmt, args...) != -1
  			? std::string(buf)
  			: std::string("Fatal - cannot format exception message since "
  						  "asprintf returned -1. This is an issue with "
  						  "BException, not your code.");

  		free(buf);
  	}

    const char* what() const noexcept override {
        return msg.c_str();
    }
};