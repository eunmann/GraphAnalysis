#include "Timer.hpp"

class BlockTimer {
public:
	BlockTimer(std::string message);
	~BlockTimer();
private:
	Timer timer;
};