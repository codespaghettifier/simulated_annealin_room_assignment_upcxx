#include "../include/AnnealingTask.hpp"

#include <cstring>

AnnealingTask::AnnealingTask(const AnnealingTask& other) :
    lowestCost(other.lowestCost),
    numRooms(other.numRooms),
    worseSolutionAcceptanceProbability(other.worseSolutionAcceptanceProbability),
    worseSolutionAcceptanceProbabilityRange(other.worseSolutionAcceptanceProbabilityRange),
    worseSolutionAcceptanceProbabilityDecreaseRate(other.worseSolutionAcceptanceProbabilityDecreaseRate),
    iterationsPerAcceptanceProbability(other.iterationsPerAcceptanceProbability),
    randomGeneratorSeed(other.randomGeneratorSeed)
{
    if(other.solution)
    {
        solution = std::make_unique<RoomsAssignment>(*other.solution);
    }
    if(other.costMatrix)
    {
        costMatrix = std::make_unique<CostMatrix>(*other.costMatrix);
    }
}

bool AnnealingTask::isRunnable() const
{
    bool runnable = true;
    runnable = runnable && (costMatrix || solution);
    runnable = runnable && !(costMatrix && numRooms * 2 < costMatrix->getSize() - 1);
    runnable = runnable && !(solution && numRooms != solution->getNumRooms() && !costMatrix);
    runnable = runnable && (worseSolutionAcceptanceProbability >= worseSolutionAcceptanceProbabilityRange.first && worseSolutionAcceptanceProbability <= worseSolutionAcceptanceProbabilityRange.second);
    return runnable;
}

AnnealingTask& AnnealingTask::operator=(AnnealingTask&& other)
{
    if(this == &other) return *this;

    lowestCost = std::move(other.lowestCost);
    numRooms = std::move(other.numRooms);
    solution = std::move(other.solution);
    costMatrix = std::move(other.costMatrix);
    worseSolutionAcceptanceProbability = std::move(other.worseSolutionAcceptanceProbability);
    worseSolutionAcceptanceProbabilityRange = std::move(other.worseSolutionAcceptanceProbabilityRange);
    worseSolutionAcceptanceProbabilityDecreaseRate = std::move(other.worseSolutionAcceptanceProbabilityDecreaseRate);
    iterationsPerAcceptanceProbability = std::move(other.iterationsPerAcceptanceProbability);
    randomGeneratorSeed = std::move(other.randomGeneratorSeed);

    return *this;
}

void AnnealingTask::print(std::ostream& stream) const
{
    stream << "{";

    stream << "lowestCost: " << lowestCost;

    stream << ", \nnumRooms: " << numRooms;
    
    stream << ", \nsolution: ";
    if(solution) stream << *solution.get();
    else stream << "nullptr";

    stream << ", \ncostMatrix: ";
    if(costMatrix) stream << *costMatrix.get();
    else stream << "nullptr";
    
    stream << ", \nworseSolutionAcceptanceProbability: " << worseSolutionAcceptanceProbability;

    stream << ", \nworseSolutionAcceptanceProbabilityRange: (" << worseSolutionAcceptanceProbabilityRange.first
            << ", " << worseSolutionAcceptanceProbabilityRange.second << ")";
 
    stream << ", \nworseSolutionAcceptanceProbabilityDecreaseRate: " << worseSolutionAcceptanceProbabilityDecreaseRate;
    
    stream << ", \niterationsPerAcceptanceProbability: " << iterationsPerAcceptanceProbability;

    stream << ", \nrandomGeneratorSeed: " << randomGeneratorSeed;

    stream << "}";
}


std::pair<std::unique_ptr<char[]>, unsigned> AnnealingTask::serialize()
{
    if(!solution) solution = std::make_unique<RoomsAssignment>();
    std::pair<std::unique_ptr<char[]>, unsigned> serializedSolution = solution->serialize();

    if(!costMatrix) costMatrix = std::make_unique<CostMatrix>();
    std::pair<std::unique_ptr<char[]>, unsigned> serializedCostMatrix = costMatrix->serialize();

    const unsigned dataSize = sizeof(lowestCost)
            + sizeof(numRooms)
            + serializedSolution.second
            + serializedCostMatrix.second
            + sizeof(worseSolutionAcceptanceProbability)
            + sizeof(worseSolutionAcceptanceProbabilityRange.first)
            + sizeof(worseSolutionAcceptanceProbabilityRange.second)
            + sizeof(worseSolutionAcceptanceProbabilityDecreaseRate)
            + sizeof(iterationsPerAcceptanceProbability)
            + sizeof(randomGeneratorSeed);
    std::unique_ptr<char[]> data = std::make_unique<char[]>(dataSize);
    char* start = data.get();

    std::memcpy(start, &lowestCost, sizeof(lowestCost));
    start += sizeof(lowestCost);

    std::memcpy(start, &numRooms, sizeof(numRooms));
    start += sizeof(numRooms);

    std::memcpy(start, serializedSolution.first.get(), serializedSolution.second);
    start += serializedSolution.second;

    std::memcpy(start, serializedCostMatrix.first.get(), serializedCostMatrix.second);
    start += serializedCostMatrix.second;

    std::memcpy(start, &worseSolutionAcceptanceProbability, sizeof(worseSolutionAcceptanceProbability));
    start += sizeof(worseSolutionAcceptanceProbability);

    std::memcpy(start, &worseSolutionAcceptanceProbabilityRange.first, sizeof(worseSolutionAcceptanceProbabilityRange.first));
    start += sizeof(worseSolutionAcceptanceProbabilityRange.first);
    std::memcpy(start, &worseSolutionAcceptanceProbabilityRange.second, sizeof(worseSolutionAcceptanceProbabilityRange.second));
    start += sizeof(worseSolutionAcceptanceProbabilityRange.second);

    std::memcpy(start, &worseSolutionAcceptanceProbabilityDecreaseRate, sizeof(worseSolutionAcceptanceProbabilityDecreaseRate));
    start += sizeof(worseSolutionAcceptanceProbabilityDecreaseRate);

    std::memcpy(start, &iterationsPerAcceptanceProbability, sizeof(iterationsPerAcceptanceProbability));
    start += sizeof(iterationsPerAcceptanceProbability);

    std::memcpy(start, &randomGeneratorSeed, sizeof(randomGeneratorSeed));
    start += sizeof(randomGeneratorSeed);

    return std::make_pair(std::move(data), dataSize);
}

unsigned AnnealingTask::deserialize(const char* data)
{
    const char* start = data;

    std::memcpy(&lowestCost, start, sizeof(lowestCost));
    start += sizeof(lowestCost);

    std::memcpy(&numRooms, start, sizeof(numRooms));
    start += sizeof(numRooms);

    solution = std::make_unique<RoomsAssignment>();
    start += solution->deserialize(start);

    costMatrix = std::make_unique<CostMatrix>();
    start += costMatrix->deserialize(start);

    std::memcpy(&worseSolutionAcceptanceProbability, start, sizeof(worseSolutionAcceptanceProbability));
    start += sizeof(worseSolutionAcceptanceProbability);

    std::memcpy(&worseSolutionAcceptanceProbabilityRange.first, start, sizeof(worseSolutionAcceptanceProbabilityRange.first));
    start += sizeof(worseSolutionAcceptanceProbabilityRange.first);
    std::memcpy(&worseSolutionAcceptanceProbabilityRange.second, start, sizeof(worseSolutionAcceptanceProbabilityRange.second));
    start += sizeof(worseSolutionAcceptanceProbabilityRange.second);

    std::memcpy(&worseSolutionAcceptanceProbabilityDecreaseRate, start, sizeof(worseSolutionAcceptanceProbabilityDecreaseRate));
    start += sizeof(worseSolutionAcceptanceProbabilityDecreaseRate);

    std::memcpy(&iterationsPerAcceptanceProbability, start, sizeof(iterationsPerAcceptanceProbability));
    start += sizeof(iterationsPerAcceptanceProbability);

    std::memcpy(&randomGeneratorSeed, start, sizeof(randomGeneratorSeed));
    start += sizeof(randomGeneratorSeed);

    return start - data;
}

std::ostream& operator<<(std::ostream& stream, const AnnealingTask& task)
{
    task.print(stream);
    return stream;
}