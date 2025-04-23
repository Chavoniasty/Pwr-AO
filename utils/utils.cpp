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
    //   std::cout << "Loading file: " << filename << std::endl;
    std::string filePath = std::filesystem::current_path().string() + "/" + filename;

    // std::cout << "Full file path: " << filePath << std::endl;

    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error: file not found at " << filePath << std::endl;
        return {};
    }

    std::vector<Node> nodes;
    std::string line;
    bool readingNodes = false;
    while (std::getline(file, line)) {
        if (line == "NODE_COORD_SECTION") {
            readingNodes = true;
            continue;
        }
        if (line == "EOF") {
            break;
        }
        if (readingNodes) {
            std::istringstream iss(line);
            int tempId;
            double tempX, tempY;
            if (!(iss >> tempId >> tempX >> tempY)) {
                std::cerr << "Error: malformed line: " << line << std::endl;
                continue;
            }
            nodes.push_back(Node(tempId, tempX, tempY));
        }
    }

    std::vector<std::vector<double>> costMatrix(nodes.size(), std::vector<double>(nodes.size(), 0));
    for (int i = 0; i < nodes.size(); i++) {
        for (int j = 0; j < nodes.size(); j++) {
            costMatrix[i][j] = calculateDistanceBetweenNodes(nodes[i], nodes[j]);
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

void displaycostMatrix(std::vector<std::vector<double>> costMatrix) {
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