#include <json/reader.h>
#include <json/value.h>

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
    std::ifstream file(filename, std::ifstream::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(file, root, false)) {
        throw std::runtime_error("Failed to parse JSON");
    }

    if (!root.isMember("travel_times") || !root["travel_times"].isArray()) {
        throw std::runtime_error("\"travel_times\" field is missing or not an array");
    }

    std::vector<std::vector<double>> costMatrix;

    const Json::Value& travelArray = root["travel_times"];
    for (const auto& row : travelArray) {
        std::vector<double> travelRow;
        for (const auto& val : row) {
            travelRow.push_back(val.asDouble());
        }
        costMatrix.push_back(travelRow);
    }

    return costMatrix;
}

std::vector<std::vector<double>> loadTimeWindows(std::string filename) {
    std::ifstream file(filename, std::ifstream::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(file, root, false)) {
        throw std::runtime_error("Failed to parse JSON");
    }

    if (!root.isMember("travel_times") || !root["travel_times"].isArray()) {
        throw std::runtime_error("\"travel_times\" field is missing or not an array");
    }

    std::vector<std::vector<double>> timeWindows;
    const Json::Value& timeWindowsArray = root["time_windows"];
    for (const auto& row : timeWindowsArray) {
        std::vector<double> timeWindowsRow;
        for (const auto& val : row) {
            timeWindowsRow.push_back(val.asDouble());
        }
        timeWindows.push_back(timeWindowsRow);
    }
    return timeWindows;
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