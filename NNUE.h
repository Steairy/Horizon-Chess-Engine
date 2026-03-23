#pragma once
#include <vector>
#include <array>

class NNUE {
    private:
    std::array<std::array<float, 1024>, 772> W1;
    std::array<std::array<float, 32>, 1024> W2;
    std::array<float, 32> W3;

    std::array<float, 1024> B1;
    std::array<float, 32> B2;
    float B3;

    std::array<int, 70> previousInput = {};
    int previousCount = 0;
    std::array<float, 1024> out1{};
    void loadWeights();

    public:
    NNUE();
    int forward(std::array<int, 70>& activated, int count);
};