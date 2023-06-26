#include "../include/RoomsAssignment.hpp"

#include <cstring>

RoomsAssignment::RoomsAssignment(unsigned numRooms)
: numRooms(numRooms)
{
    assignments = std::make_unique<Room[]>(numRooms);
    for(unsigned i = 0; i < numRooms; i++)
    {
        assignments[i] = Room(0, 0);
    }
}

RoomsAssignment::RoomsAssignment(unsigned numRooms, std::unique_ptr<CostMatrix> costMatrix, bool calculateNaive)
: RoomsAssignment(numRooms)
{
    setCostMatrix(std::move(costMatrix));

    if(calculateNaive) calculateNaiveSolution();
}

RoomsAssignment::RoomsAssignment(const RoomsAssignment& other)
: numRooms(other.numRooms)
, cost(other.cost)
{
    assignments = std::make_unique<Room[]>(numRooms);
    std::copy(other.assignments.get(), other.assignments.get() + numRooms, assignments.get());

    if(other.costMatrix)
    {
        costMatrix = std::make_unique<CostMatrix>(*other.costMatrix);
    }
}


void RoomsAssignment::calculateNaiveSolution()
{
    if(costMatrix.get() == nullptr) throw std::logic_error("Cost matrix not initlized.");

    for(unsigned i = 0; i < numRooms; i++)
    {
        // Person with index 0 represents an empty slot.
        const unsigned firstPerson = i * 2 + 1;
        if(firstPerson >= costMatrix->getSize()) break;
        assignments[i].first = firstPerson;

        const unsigned secondPerson = i * 2 + 2;
        if(secondPerson >= costMatrix->getSize()) break;
        assignments[i].second = secondPerson;
    }
    calculateCost();
}

int RoomsAssignment::getSwapCost(unsigned roomA, bool personAFirst, unsigned roomB, bool personBFirst) const
{
    const int costBefore = costMatrix->getCost(assignments[roomA].first, assignments[roomA].second) + costMatrix->getCost(assignments[roomB].first, assignments[roomB].second);
    const unsigned newRoomAFirst = personAFirst ? (personBFirst ? assignments[roomB].first : assignments[roomB].second) : assignments[roomA].first;
    const unsigned newRoomASecond = personAFirst ? assignments[roomA].second : (personBFirst ? assignments[roomB].first : assignments[roomB].second);
    const unsigned newRoomBFirst = personBFirst ? (personAFirst ? assignments[roomA].first : assignments[roomA].second) : assignments[roomB].first;
    const unsigned newRoomBSecond = personBFirst ? assignments[roomB].second : (personAFirst ? assignments[roomA].first : assignments[roomA].second);
    const int costAfter = costMatrix->getCost(newRoomAFirst, newRoomASecond) + costMatrix->getCost(newRoomBFirst, newRoomBSecond);
    return costAfter - costBefore;
}

void RoomsAssignment::swap(unsigned roomA, bool personAFirst, unsigned roomB, bool personBFirst)
{
    const int costBefore = costMatrix->getCost(assignments[roomA].first, assignments[roomA].second) + costMatrix->getCost(assignments[roomB].first, assignments[roomB].second);
    
    const unsigned newRoomAFirst = personAFirst ? (personBFirst ? assignments[roomB].first : assignments[roomB].second) : assignments[roomA].first;
    const unsigned newRoomASecond = personAFirst ? assignments[roomA].second : (personBFirst ? assignments[roomB].first : assignments[roomB].second);
    const unsigned newRoomBFirst = personBFirst ? (personAFirst ? assignments[roomA].first : assignments[roomA].second) : assignments[roomB].first;
    const unsigned newRoomBSecond = personBFirst ? assignments[roomB].second : (personAFirst ? assignments[roomA].first : assignments[roomA].second);
    
    // Case when roomA == roomB
    if(newRoomAFirst == newRoomASecond || newRoomBFirst == newRoomBSecond) return;
    
    assignments[roomA].first = newRoomAFirst;
    assignments[roomA].second = newRoomASecond;
    assignments[roomB].first = newRoomBFirst;
    assignments[roomB].second = newRoomBSecond;

    const int costAfter = costMatrix->getCost(assignments[roomA].first, assignments[roomA].second) + costMatrix->getCost(assignments[roomB].first, assignments[roomB].second);
    cost += costAfter - costBefore;
}

void RoomsAssignment::print(std::ostream& stream) const
{
    stream << "{";

    stream << "numRooms: " << numRooms;

    stream << ", \nrooms: ";
    stream << "[";
    for(unsigned i = 0; i < numRooms; i++)
    {
        if(i != 0) stream << ",\n";
        stream << "[";
        stream << assignments[i].first;
        stream << ", " << assignments[i].second;
        stream << "]";
    }
    stream << "]";

    stream << ", \ncost: " << cost;

    if(costMatrix.get() == nullptr)
    {
        stream << ", \ncostMatrix: nullptr";
    }
    else
    {
        stream << ", \ncostMatrix: " << *costMatrix.get();
    }

    stream << "}";
}

std::pair<std::unique_ptr<char[]>, unsigned> RoomsAssignment::serialize()
{
    if(!costMatrix) costMatrix = std::make_unique<CostMatrix>();
    std::pair<std::unique_ptr<char[]>, unsigned> serializedCostMatrix = costMatrix->serialize();

    const unsigned dataSize = sizeof(numRooms)
            + numRooms * (sizeof(assignments[0].first) + sizeof(assignments[0].second))
            + serializedCostMatrix.second
            + sizeof(cost);
    std::unique_ptr<char[]> data = std::make_unique<char[]>(dataSize);
    char* start = data.get();

    std::memcpy(start, &numRooms, sizeof(numRooms));
    start += sizeof(numRooms);

    for(unsigned i = 0; i < numRooms; i++)
    {
        std::memcpy(start, &assignments[i].first, sizeof(assignments[i].first));
        start += sizeof(assignments[i].first);
        std::memcpy(start, &assignments[i].second, sizeof(assignments[i].second));
        start += sizeof(assignments[i].second);
    }

    std::memcpy(start, serializedCostMatrix.first.get(), serializedCostMatrix.second);
    start += serializedCostMatrix.second;

    std::memcpy(start, &cost, sizeof(cost));
    start += sizeof(cost);

    return std::make_pair(std::move(data), dataSize);
}

unsigned RoomsAssignment::deserialize(const char* data)
{
    const char* start = data;

    std::memcpy(&numRooms, start, sizeof(numRooms));
    start += sizeof(numRooms);

    assignments = std::make_unique<Room[]>(numRooms);
    for (unsigned i = 0; i < numRooms; i++)
    {
        std::memcpy(&assignments[i].first, start, sizeof(assignments[i].first));
        start += sizeof(assignments[i].first);
        std::memcpy(&assignments[i].second, start, sizeof(assignments[i].second));
        start += sizeof(assignments[i].second);
    }

    costMatrix = std::make_unique<CostMatrix>();
    start += costMatrix->deserialize(start);

    std::memcpy(&cost, start, sizeof(cost));
    start += sizeof(cost);

    return start - data;
}

void RoomsAssignment::calculateCost()
{
    cost = 0;
    for(unsigned i = 0; i < numRooms; i++)
    {
        cost += costMatrix->getCost(assignments[i].first, assignments[i].second);
    }
}

std::ostream& operator<<(std::ostream& stream, const RoomsAssignment& roomsAssignment)
{
    roomsAssignment.print(stream);
    return stream;
}