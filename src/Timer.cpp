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

int64_t Timer::get_time_elapsed() {
	return std::chrono::duration_cast<std::chrono::nanoseconds>(this->e - this->s).count();
}

void Timer::print() {
	char unit[][4] = { "ns", "us", "ms", "s", "min" };
	double ratio[] = { 1000, 1000, 1000, 60, 60 };

	double timeElapsed = this->get_time_elapsed();

	int i;
	for (i = 0; i < 4; i++) {
		if (timeElapsed >= ratio[i]) {
			timeElapsed /= ratio[i];
		}
		else {
			break;
		}
	}

	printf("[ %s | %3.3f %s ]\n", this->message.c_str(), timeElapsed, unit[i]);
}

void Timer::print(std::string message) {
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

	printf("[ %s | %3.3f %s ]\n", message.c_str(), timeElapsed, unit[i]);
}
