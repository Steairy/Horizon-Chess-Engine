#include "NNUE.h"
#include <fstream>
#include <algorithm>
#include <iterator>

void NNUE::loadWeights() {
    std::ifstream f;

    f.open("weights/W1.bin", std::ios::binary);
    f.read(reinterpret_cast<char*>(W1.data()), sizeof(float) * 772 * 256);
    f.close();

    f.open("weights/B1.bin", std::ios::binary);
    f.read(reinterpret_cast<char*>(B1.data()), sizeof(float) * 256);
    f.close();

    f.open("weights/W2.bin", std::ios::binary);
    f.read(reinterpret_cast<char*>(W2.data()), sizeof(float) * 256 * 32);
    f.close();

    f.open("weights/B2.bin", std::ios::binary);
    f.read(reinterpret_cast<char*>(B2.data()), sizeof(float) * 32);
    f.close();

    f.open("weights/W3.bin", std::ios::binary);
    f.read(reinterpret_cast<char*>(W3.data()), sizeof(float) * 32);
    f.close();

    f.open("weights/B3.bin", std::ios::binary);
    f.read(reinterpret_cast<char*>(&B3), sizeof(float));
    f.close();
}

NNUE::NNUE(){
    loadWeights();
}

int NNUE::forward(std::array<int, 70>& activated, int count){
    std::array<float, 32> out2{};

    // Input -> Layer1
    std::sort(activated.begin(), activated.begin() + count);

    std::array<int, 70> additions;
    std::array<int, 70> subtractions;

    int additionCount = 0; int subtractionCount = 0;
    int prevIndex = 0; int currentIndex = 0;

    while(prevIndex < previousCount && currentIndex < count){
        int prevValue = previousInput[prevIndex]; int currentValue = activated[currentIndex];
        if(currentValue == prevValue){
            prevIndex++; currentIndex++;
        }
        else if(currentValue > prevValue){
            subtractions[subtractionCount++] = prevValue;
            prevIndex++;
        }
        else{
            additions[additionCount++] = currentValue;
            currentIndex++;
        }
    }

    while(currentIndex < count)
        additions[additionCount++] = activated[currentIndex++];

    while(prevIndex < previousCount)
        subtractions[subtractionCount++] = previousInput[prevIndex++];

    for(int i = 0; i < additionCount; i++){
        for(int neuron = 0; neuron < 256; neuron++){
            out1[neuron] += W1[additions[i]][neuron];
        }
    }

    for(int i = 0; i < subtractionCount; i++){
        for(int neuron = 0; neuron < 256; neuron++){
            out1[neuron] -= W1[subtractions[i]][neuron];
        }
    }

    std::array<float, 256> l1Activated{};
    for(int neuron = 0; neuron < 256; neuron++){
        l1Activated[neuron] = std::max(out1[neuron] + B1[neuron], 0.0f);
    }

    previousInput = activated;
    previousCount = count;

    // Layer1 -> Layer2
    for(int ind = 0; ind < 256; ind++){
        for(int neuron = 0; neuron < 32; neuron++){
            out2[neuron] += W2[ind][neuron] * l1Activated[ind];
        }
    }

    for(int neuron = 0; neuron < 32; neuron++){
        out2[neuron] = std::max(out2[neuron] + B2[neuron], 0.0f);
    }

    // Layer2 -> output
    float result = B3;
    for(int i = 0; i < 32; i++){
        result += W3[i] * out2[i];
    }
   
    return (int)result;
}
