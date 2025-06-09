#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "Node.h"

double calculateDistanceBetweenNodes(Node a, Node b) {
    return std::sqrt(std::pow(a.getX() - b.getX(), 2) + std::pow(a.getY() - b.getY(), 2));
}

std::vector<std::vector<double>> loadData(std::string filename) {
    std::string filePath = std::filesystem::current_path().string() + "/" + filename;
    std::cout << "Full file path: " << filePath << std::endl;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: file not found at " << filePath << std::endl;
        return {};
    }

    std::string line;
    int dimension = -1;
    bool inMatrix = false;
    std::vector<double> values;

    while (std::getline(file, line)) {
        // Trim whitespace from line
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;

        if (line.find("DIMENSION") != std::string::npos) {
            size_t pos = line.find_first_of(":=");
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                value.erase(0, value.find_first_not_of(" \t"));  // Trim leading spaces
                dimension = std::stoi(value);
                std::cout << "Parsed DIMENSION: " << dimension << std::endl;
            }
        } else if (line.find("EDGE_WEIGHT_SECTION") != std::string::npos) {
            inMatrix = true;
            continue;
        } else if (line.find("EOF") != std::string::npos) {
            break;
        }

        if (inMatrix) {
            std::istringstream iss(line);
            double num;
            while (iss >> num) {
                values.push_back(num);
            }
        }
    }

    if (dimension == -1) {
        throw std::runtime_error("DIMENSION not found in file.");
    }

    if (values.size() != static_cast<size_t>(dimension * dimension)) {
        throw std::runtime_error("Matrix size mismatch: expected " +
                                 std::to_string(dimension * dimension) + ", got " +
                                 std::to_string(values.size()));
    }

    std::vector<std::vector<double>> costMatrix(dimension, std::vector<double>(dimension));
    for (int i = 0; i < dimension; ++i) {
        for (int j = 0; j < dimension; ++j) {
            costMatrix[i][j] = values[i * dimension + j];
        }
    }

    return costMatrix;
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

std::vector<int> initialGuess(int size) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    std::random_device rd;
    std::mt19937 rng(rd());
    std::vector<int> permutation(size);
    std::iota(permutation.begin(), permutation.end(), 0);
    std::shuffle(permutation.begin(), permutation.end(), rng);

    return permutation;
};

void displayCostMatrix(std::vector<std::vector<double>> costMatrix) {
    for (const auto& row : costMatrix) {
        for (const auto& value : row) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
}

void saveResult(std::vector<int> bestPath, double cost, std::string title, std::string fileLocation) {
    std::ofstream file;
    file.open(fileLocation, std::ios_base::app);

    if (!file.is_open()) {
        std::cerr << "Error: file not found" << std::endl;
        return;
    }

    file << title << std::endl
         << cost << std::endl;

    for (int i = 0; i < bestPath.size(); ++i) {
        file << bestPath.at(i) << " ";
    }
    file << std::endl;
}

void saveResultForThread(std::vector<std::vector<double>> result, const std::string& fileLocation) {
    // std::cout << "Results saved to " << fileLocation << std::endl;
    std::ofstream file;
    std::cout << fileLocation << std::endl;
    file.open(fileLocation, std::ios_base::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: file not found" << std::endl;
        return;
    }
    for (const auto& row : result) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) file << " ";
        }
        file << std::endl;
    }

    file.close();
}