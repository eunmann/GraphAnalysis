#pragma once

#include <chrono>
#include <string>

class Timer {
   private:
    std::string message;
    std::chrono::steady_clock::time_point s;
    std::chrono::steady_clock::time_point e;

   public:
    Timer();
    Timer(std::string message);

    void start();
    void end();
    int64_t getTimeElapsed();
    void print();
    void print(std::string message);
};