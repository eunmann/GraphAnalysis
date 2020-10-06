#pragma once
#include <inttypes.h>

#include <functional>

class Graph {
   private:
    uint32_t* arr;

   public:
    const uint32_t numOfNodes;

    Graph(const uint32_t numOfNodes);

    const uint32_t weight(const uint32_t i, const uint32_t j);
    uint32_t& operator()(const uint32_t i, const uint32_t j);
    void set(const uint32_t weight, const uint32_t i, const uint32_t j);
    void forEach(std::function<void(uint32_t& v, const uint32_t i, const uint32_t j)> func);

   private:
    const uint32_t index(const uint32_t i, const uint32_t j);
};
