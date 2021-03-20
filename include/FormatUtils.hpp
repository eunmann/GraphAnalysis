#pragma once

#include <string>
#include <stdexcept>

namespace FormatUtils {
	std::string format_number(double num);

	template<typename ... Args>
	std::string format(const std::string& format, Args ... args) {
		int size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
		if (size <= 0) {
			throw std::runtime_error("String format error. size <= 0");
		}
		char buff[size];
		snprintf(buff, size, format.c_str(), args ...);
		return std::string(buff, buff + size - 1);
	}
}