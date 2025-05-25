#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <string>
#include <vector>

#include "Node.h"

std::vector<std::vector<double>> loadData(std::string filename);

std::vector<std::vector<double>> loadTimeWindows(std::string filename);

template <typename T>
T calculateCost(const std::vector<int>& permutation,
                const std::vector<std::vector<T>>& costMatrix,
                const std::vector<std::vector<double>>& timeWindows) {
    T cost = 0;
    T currentTime = 0;

    for (size_t i = 0; i < permutation.size(); ++i) {
        int from = permutation[i];
        int to = permutation[(i + 1) % permutation.size()];

        T travelTime = costMatrix[from][to];
        currentTime += travelTime;
        cost += travelTime;

        double earliest = timeWindows[to][0];
        if (currentTime < earliest) {
            T waitTime = static_cast<T>(earliest - currentTime);
            currentTime = earliest;
            cost += waitTime;
        }
    }

    return cost;
}

template <typename T>
T calculateCost(const std::vector<int>& permutation,
                const std::vector<std::vector<T>>& costMatrix) {
    T cost = 0;
    for (int i = 0; i < permutation.size() - 1; ++i) {
        cost += costMatrix[permutation[i]][permutation[i + 1]];
    }
    cost += costMatrix[permutation.back()][permutation[0]];
    return cost;
}

std::vector<int> initialGuess(int size);

void displayCostMatrix(std::vector<std::vector<double>> costMatrix);

void saveResult(std::vector<int> bestPath, double cost, std::string title, std::string fileLocation);

void saveResultForThread(std::vector<std::vector<double>> result, const std::string& fileLocation);
#endif
