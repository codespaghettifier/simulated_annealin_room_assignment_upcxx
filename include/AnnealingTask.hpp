#pragma once

#include "RoomsAssignment.hpp"

#include <limits>
#include <optional>

class AnnealingTask
{
public:
    AnnealingTask() = default;
    AnnealingTask(const AnnealingTask& other);

    int lowestCost = std::numeric_limits<int>::max();
    int numRooms;
    std::unique_ptr<RoomsAssignment> solution;
    std::unique_ptr<CostMatrix> costMatrix;
    float worseSolutionAcceptanceProbability;
    std::pair<float, float> worseSolutionAcceptanceProbabilityRange;
    float worseSolutionAcceptanceProbabilityDecreaseRate;
    long iterationsPerAcceptanceProbability;
    long randomGeneratorSeed;

    bool isRunnable() const;
    AnnealingTask& operator=(AnnealingTask&& other);
    void print(std::ostream& stream) const;
    std::pair<std::unique_ptr<char[]>, unsigned> serialize();
    unsigned deserialize(const char* data);
};

std::ostream& operator<<(std::ostream& stream, const AnnealingTask& costMatrix);
