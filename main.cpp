#include <omp.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#include "./utils/Node.h"
#include "./utils/utils.h"

#define SAVE_RESULTS 0
#define SYNC_INTERVAL 1000

template <typename T>
void anneal(
    const std::vector<std::vector<T>>& costMatrix,
    std::vector<int>& path,
    std::vector<int>& bestPath,
    std::vector<int>& globalBestPath,
    T& temperature,
    std::vector<std::vector<double>>& results,
    T alpha,
    std::vector<std::vector<int>>& bestPaths,
    std::vector<T>& bestCosts,
    int numThreads,
    T& globalBestCost,
    int maxIterations
) { 
    std::uniform_real_distribution<> dist(0.0f, 1.0f);
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<> indexDist(0, path.size() - 1);

    int threadNum = omp_get_thread_num();

    bool acceptCondition = false;
    double iter = 1;
    T bestCost = calculateCost(bestPath, costMatrix);
    T prevCost = calculateCost(path, costMatrix);
    T currentCost = calculateCost(path, costMatrix);
    while (iter < maxIterations) {
        int index1 = indexDist(rng);
        int index2 = indexDist(rng);
        std::swap(path[index1], path[index2]);
        currentCost = calculateCost(path, costMatrix);

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
                    if (calculateCost(bestPath, costMatrix) < calculateCost(globalBestPath, costMatrix)) {
                        globalBestPath = bestPath;
                    }
                }
            }
        } else {
            std::swap(path[index1], path[index2]);
        }
        results.push_back(std::vector<double>{iter, prevCost});
        temperature *= alpha;

        if (static_cast<int>(iter) % SYNC_INTERVAL == 0) {

            // std::cout << "Thread: " << threadNum << ", Iteration: " << iter
            //      << ", Temperature: " << temperature
            //      << ", Best Cost: " << bestCost
            //      << ", Global Best Cost: " << globalBestCost
            //      << std::endl;
            #pragma omp critical
            {
                bestCosts[threadNum] = bestCost;
                bestPaths[threadNum] = bestPath;
            }

            #pragma omp barrier

            #pragma omp single
            {
                int bestThread = 0;
                T bestGlobalCost = bestCosts[0];
                for (int i = 1; i < bestCosts.size(); ++i) {
                    //std::cout << "Thread: " << i << ", Best Cost: " << bestCosts[i] << std::endl;
                    if (bestCosts[i] < bestGlobalCost) {
                        bestGlobalCost = bestCosts[i];
                        bestThread = i;
                    }
                }
                globalBestCost = bestGlobalCost;
                globalBestPath = bestPaths[bestThread];
            }

            #pragma omp barrier
            bestCost = calculateCost(globalBestPath, costMatrix);
            path = globalBestPath;
            bestPath = globalBestPath;
            prevCost = bestCost;
        }
        iter++;
    }
}

template <typename T>
T getInitialTemperature(const std::vector<std::vector<T>> costMatrix, std::vector<int> path) {
    std::uniform_int_distribution<> indexDist(0, path.size() - 1);
    std::random_device rd;
    std::mt19937 rng(rd());
    T sum = 0;

    for (int i = 0; i < 100; i++) {
        int index1 = indexDist(rng);
        int index2 = indexDist(rng);
        std::swap(path[index1], path[index2]);
        sum += calculateCost(path, costMatrix);
    }
    T mean = sum / 100;
    return (-1 * mean) / std::log(0.9);
}

template <typename T>
void startAnnealing(
    const std::vector<std::vector<T>> costMatrix,
    std::vector<int> path,
    std::vector<int>& bestPath,
    std::string destFolder,
    T& globalBestCostOut,
    std::vector<int>& globalBestPathOut,
    int maxIterations
) {
    T temperature = getInitialTemperature(costMatrix, path);
    double temperatureStep = (0.9998 - 0.998) / 5;
    std::array<T, 8> alpha = {static_cast<T>(0.9998), static_cast<T>(0.9990), static_cast<T>(0.9992),
                              static_cast<T>(0.9994), static_cast<T>(0.9996), static_cast<T>(0.9998), static_cast<T>(0.9996), static_cast<T>(0.9998)};
    std::vector<std::vector<int>> allBestPaths(8);

    int numThreads = 8;

    std::vector<std::vector<int>> bestPaths(numThreads, bestPath);
    std::vector<T> bestCosts(numThreads, calculateCost(bestPath, costMatrix));
    std::vector<int> globalBestPath = bestPath;
    T globalBestCost = calculateCost(bestPath, costMatrix);

    //omp_set_num_threads(6);
#pragma omp parallel shared(bestPaths, bestCosts, globalBestPath, globalBestCost, maxIterations)
    {
        #pragma omp single
        {
            std::cout << "Running: " << omp_get_num_threads() << " threads" << std::endl;
        }

        T threadTemperature = temperature;
        std::vector<std::vector<double>> results;

        std::vector<int> localPath = bestPath;
        std::vector<int> localBestPath = bestPath;
        T threadAlpha = alpha[omp_get_thread_num()];

        anneal(costMatrix, localPath, localBestPath, globalBestPath,
            threadTemperature, results, threadAlpha,
            bestPaths, bestCosts, numThreads, globalBestCost, maxIterations);

        int threadNum = omp_get_thread_num();

        #if SAVE_RESULTS == 1
        std::string fileLocation = destFolder + "/thread" + std::to_string(threadNum) + ".txt";
        saveResultForThread(results, fileLocation);
        #endif
    }

    bestPath = globalBestPath;
    globalBestCostOut = globalBestCost;
    globalBestPathOut = globalBestPath;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: ./a.out <input_file> <output_folder> [max_iterations]" << std::endl;
        return 1;
    }

    int maxIterations = 100000;
    if (argc >= 4) {
        maxIterations = std::stoi(argv[3]);
    }

    std::vector<std::vector<double>> costMatrix = loadData(argv[1]);

    std::string destFolder = argv[2];

    int size = costMatrix.size();

    std::vector<int> path = initialGuess(size);
    std::vector<int> bestPath = path;
    std::cout << std::endl
              << "Initial cost: " << calculateCost(path, costMatrix) << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<int> finalBestPath;
    double finalBestCost = 0.0;

    startAnnealing(costMatrix, path, bestPath, destFolder, finalBestCost, finalBestPath, maxIterations);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Duration: " << duration.count() << "ms" << std::endl;
    std::cout << calculateCost(bestPath, costMatrix) << std::endl;
    return 0;
}
