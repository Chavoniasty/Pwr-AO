#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <string>
#include <vector>

#include "Node.h"

std::vector<std::vector<double>> loadData(std::string filename);

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

void displaycostMatrix(std::vector<std::vector<double>> costMatrix);

void saveResult(std::vector<int> bestPath, double cost, std::string title, std::string fileLocation);

void saveResultForThread(std::vector<std::vector<double>> result, const std::string& fileLocation);
#endif
