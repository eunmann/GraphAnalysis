#include "Timer.hpp"

#include <cstdio>
#include "FormatUtils.hpp"

Timer::Timer(std::string message) : message(message),
s(std::chrono::steady_clock::now()),
e(s) {}

Timer::Timer() : message(""),
s(std::chrono::steady_clock::now()),
e(s) {}

void Timer::start() {
	this->s = std::chrono::steady_clock::now();
}

void Timer::end() {
	this->e = std::chrono::steady_clock::now();
}

int64_t Timer::get_time_elapsed() const {
	return std::chrono::duration_cast<std::chrono::nanoseconds>(this->e - this->s).count();
}

void Timer::print() const {
	printf("[ %s | %s ]\n", this->message.c_str(), this->time_string().c_str());
}

void Timer::print(const std::string& message) const {
	printf("[ %s | %s ]\n", message.c_str(), this->time_string().c_str());
}

std::string Timer::time_string() const {
	return FormatUtils::format_time(this->get_time_elapsed());
}
