#pragma once

#include "Timer.hpp"

/**
 * Starts a timer at creation and upon destruction it will print the time elapsed.
 */
class BlockTimer {
public:
	/**
	 * @param message A message to print along with the time elapsed
	 */
	BlockTimer(std::string message);
	~BlockTimer();
private:
	Timer timer;
};