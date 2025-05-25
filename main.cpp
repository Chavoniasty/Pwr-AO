#include <omp.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#include "./utils/Node.h"
#include "./utils/utils.h"

template <typename T>
bool isValid(const std::vector<std::vector<T>> costMatrix, std::vector<int>& path, std::vector<std::vector<double>> timeWindows) {
    double currentCost = 0;
    for (int i = 0; i < path.size(); i++) {
        int from = path[i];
        int to = path[(i + 1) % path.size()];

        currentCost += costMatrix[from][to];
        double earliest = timeWindows[to][0];
        double latest = timeWindows[to][1];
        if (currentCost > latest) {
            std::cout << "bad" << std::endl;
            return false;
        }
    }

    return true;
}

template <typename T>
void anneal(const std::vector<std::vector<T>> costMatrix, std::vector<int>& path, std::vector<int>& bestPath, std::vector<int>& globalBestPath, T& temperature, std::vector<std::vector<double>>& results, T alpha, std::vector<std::vector<double>> timeWindows) {
    std::uniform_real_distribution<> dist(0.0f, 1.0f);
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<> indexDist(0, path.size() - 1);

    bool acceptCondition = false;
    double iter = 1;
    T bestCost = calculateCost(bestPath, costMatrix, timeWindows);
    T prevCost = calculateCost(path, costMatrix, timeWindows);
    T currentCost = calculateCost(path, costMatrix, timeWindows);
    while (temperature > 0.1) {
        int index1 = indexDist(rng);
        int index2 = indexDist(rng);
        std::swap(path[index1], path[index2]);
        if (isValid(costMatrix, path, timeWindows)) {
            currentCost = calculateCost(path, costMatrix, timeWindows);

            if (currentCost > prevCost) {
                acceptCondition = dist(rng) >= std::exp((currentCost - prevCost) / temperature) ? true : false;
            }
            if (currentCost <= prevCost || acceptCondition) {
                prevCost = currentCost;

                if (bestCost > prevCost) {
                    bestCost = prevCost;
                    bestPath = path;
#pragma omp critical
                    {
                        if (bestCost < calculateCost(globalBestPath, costMatrix, timeWindows)) {
                            globalBestPath = bestPath;
                        }
                    }
                }
            }

        } else {
            std::swap(path[index1], path[index2]);
        }
        results.push_back(std::vector<double>{iter, calculateCost(path, costMatrix, timeWindows)});
        temperature *= alpha;
        iter++;
    }
}

template <typename T>
T getInitialTemperature(const std::vector<std::vector<T>>& costMatrix,
                        const std::vector<int>& path,
                        const std::vector<std::vector<double>>& timeWindows) {
    std::uniform_int_distribution<> indexDist(0, path.size() - 1);
    std::random_device rd;
    std::mt19937 rng(rd());
    std::vector<int> tmpPath = path;
    T sum = 0;
    int validSamples = 0;

    for (int i = 0; i < 1000 && validSamples < 100; i++) {
        int index1 = indexDist(rng);
        int index2 = indexDist(rng);
        std::swap(tmpPath[index1], tmpPath[index2]);
        if (isValid(costMatrix, tmpPath, timeWindows)) {
            sum += calculateCost(tmpPath, costMatrix, timeWindows);
            validSamples++;
        }
        std::swap(tmpPath[index1], tmpPath[index2]);
    }

    T mean = (validSamples > 0) ? sum / validSamples : static_cast<T>(10000);
    return (-1 * mean) / std::log(0.9);
}

template <typename T>
void startAnnealing(const std::vector<std::vector<T>> costMatrix, std::vector<int> path, std::vector<int>& bestPath, std::string destFolder, std::vector<std::vector<double>> timeWindows) {
    T temperature = getInitialTemperature(costMatrix, path, timeWindows);
    double temperatureStep = (0.9998 - 0.998) / 5;
    std::array<T, 6> alpha = {static_cast<T>(0.9998), static_cast<T>(0.9990), static_cast<T>(0.9992),
                              static_cast<T>(0.9994), static_cast<T>(0.9996), static_cast<T>(0.9998)};
    std::vector<std::vector<int>> allBestPaths(5);

    omp_set_num_threads(1);
#pragma omp parallel
    {
        T threadTemperature = temperature;
        std::vector<std::vector<double>> results;

        std::vector<int> localPath = bestPath;
        std::vector<int> localBestPath = bestPath;
        T threadAlpha = alpha[omp_get_thread_num()];

        anneal(costMatrix, localPath, localBestPath, bestPath,
               threadTemperature, results, threadAlpha, timeWindows);
        int threadNum = omp_get_thread_num();
        std::string fileLocation = destFolder + "/thread" + std::to_string(threadNum) + ".txt";
        saveResultForThread(results, fileLocation);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./a.out <string>" << std::endl;
        return 1;
    }

    std::vector<std::vector<double>> costMatrix = loadData(argv[1]);

    // Printing distance matrix for debbuging
    // for (const auto& row : costMatrix) {
    //     for (double val : row) {
    //         std::cout << val << " ";
    //     }
    //     std::cout << "\n";
    // }
    std::vector<std::vector<double>> timeWindows = loadTimeWindows(argv[1]);

    // Printing travel windows for debbuging
    // for (const auto& row : timeWindows) {
    //     for (double val : row) {
    //         std::cout << val << " ";
    //     }
    //     std::cout << std::endl;
    // }

    std::string destFolder = argv[2];

    int size = costMatrix.size();

    std::vector<int> path = initialGuess(size);
    for (const auto& index : path) {
        std::cout << index << " ";
    }
    std::vector<int> bestPath = path;
    std::cout << std::endl
              << "Initial cost: " << calculateCost(path, costMatrix, timeWindows) << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    startAnnealing(costMatrix, path, bestPath, destFolder, timeWindows);
    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // std::cout << "Duration: " << duration.count() << "ms" << std::endl;
    // // for (const auto& i : bestPath) {
    // //     std::cout << i << " ";
    // // }
    std::cout << calculateCost(bestPath, costMatrix, timeWindows);
    return 0;
}
