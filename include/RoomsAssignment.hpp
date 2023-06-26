#pragma once

#include "CostMatrix.hpp"

#include <stdexcept>
#include <iostream>

class RoomsAssignment
{
    using Room = std::pair<unsigned, unsigned>;

public:
    RoomsAssignment() = default;
    RoomsAssignment(unsigned numRooms);
    RoomsAssignment(unsigned numRooms, std::unique_ptr<CostMatrix> costMatrix, bool calculateNaive = true);
    RoomsAssignment(const RoomsAssignment& other);

    void calculateNaiveSolution();
    inline void setCostMatrix(std::unique_ptr<CostMatrix> costMatrix);
    inline unsigned getAssignment(unsigned person) const;
    int getSwapCost(unsigned roomA, bool personAFirst, unsigned roomB, bool personBFirst) const;
    void swap(unsigned roomA, bool personAFirst, unsigned roomB, bool personBFirst);
    inline unsigned getNumRooms();
    inline int getCost();
    void print(std::ostream& stream) const;
    std::pair<std::unique_ptr<char[]>, unsigned> serialize();
    unsigned deserialize(const char* data);
    void calculateCost();

private:
    unsigned numRooms;
    std::unique_ptr<Room[]> assignments;
    std::unique_ptr<CostMatrix> costMatrix;
    int cost;
};

std::ostream& operator<<(std::ostream& stream, const RoomsAssignment& costMatrix);

unsigned RoomsAssignment::getAssignment(unsigned person) const
{
    for(unsigned i = 0; i < numRooms; i++)
    {
        if(assignments[i].first == person || assignments[i].second == person) return i;
    }

    throw std::out_of_range("Person not assigned to any room.");
}

void RoomsAssignment::setCostMatrix(std::unique_ptr<CostMatrix> costMatrix)
{
    if(numRooms * 2 < costMatrix->getSize() - 1) throw std::logic_error("Not enough rooms to fit all the people.");

    this->costMatrix = std::move(costMatrix);
}

unsigned RoomsAssignment::getNumRooms()
{
    return numRooms;
}

int RoomsAssignment::getCost()
{
    return cost;
}
