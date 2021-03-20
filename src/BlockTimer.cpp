#include "BlockTimer.hpp"

BlockTimer::BlockTimer(std::string message) : timer(message) {

}

BlockTimer::~BlockTimer() {
	timer.end();
	timer.print();
}