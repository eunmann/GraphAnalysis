#include "Timer.hpp"

#include <cstdio>

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
	char unit[][4] = { "ns", "us", "ms", "s", "min" };
	double ratio[] = { 1000, 1000, 1000, 60, 60 };

	double timeElapsed = this->get_time_elapsed();

	int i;
	for (i = 0; i < 4; i++) {
		if (timeElapsed > ratio[i]) {
			timeElapsed /= ratio[i];
		}
		else {
			break;
		}
	}

	char buff[32];
	snprintf(buff, sizeof(buff), "%.3f %s", timeElapsed, unit[i]);
	return std::string(buff);
}
