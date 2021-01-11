#pragma once

#include <chrono>
#include <string>

/**
 * Timer is a utility object to record and print elapsed times.
 */
class Timer {
public:
	/**
	 * Creates the timer and starts the timer
	 */
	Timer();

	/**
	 * Creates the timer and starts the timer.
	 *
	 * @param message A message to include when printing the elapsed time
	 */
	Timer(std::string message);

	/**
	 * Start the timer. If the timer has already started, the timer
	 * will overwrite the starting time.
	 */
	void start();

	/**
	 * Stop the timer.
	 */
	void end();

	/**
	 * @return The number of nanoseconds that have elapsed
	 */
	int64_t get_time_elapsed() const;

	/**
	 * Prints the elapsed time with a message if given in the constructor
	 */
	void print() const;

	/**
	 * Prints the elapsed time with the inputted message
	 *
	 * @param message The message to print along with the elapsed time
	 */
	void print(const std::string& message) const;

	/**
	 * Returns the time as a formated string(value with unit prefix)
	 */
	std::string time_string() const;

private:
	std::string message;
	std::chrono::steady_clock::time_point s;
	std::chrono::steady_clock::time_point e;
};