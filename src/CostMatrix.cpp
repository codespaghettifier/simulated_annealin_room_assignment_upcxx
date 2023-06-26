#include "../include/CostMatrix.hpp"

#include <cstring>

CostMatrix::CostMatrix(const CostMatrix& other)
: size(other.size)
{
    cost = std::make_unique<int[]>(size * size);
    std::copy(other.cost.get(), other.cost.get() + size * size, cost.get());
}

// void CostMatrix::loadFromJson(const Json::Value& peopleJson)
// {
//     // The first person is not a real person. It's used to mark an empty slot.
//     size = peopleJson.size() + 1;
//     cost = std::unique_ptr<int[]>(new int[size * size]);

//     cost[0] = 0;
//     for(unsigned i = 1; i < size; i++)
//     {
//         cost[i] = peopleJson[i - 1]["cost"][0].asInt();
//     }
    
//     for(unsigned i = 1; i < size; i++)
//     {
//         for(int j = 0; j < size; j++)
//         {
//             cost[i * size + j] = peopleJson[i - 1]["cost"][j].asInt();
//         }
//     }
// }

void CostMatrix::loadFromTxt(std::ifstream& stream)
{
    unsigned size;
    stream >> size;

    int** matrix = new int* [size];
    for (unsigned i = 0; i < size; i++) {
        matrix[i] = new int [size];
        for (int j = 0; j < size; j++) {
            stream >> matrix[i][j];
        }
    }

    loadFromArray(matrix, size);

    for (unsigned i = 0; i < size; i++)
    {
        delete[] matrix[i];
    }
    delete[] matrix;
}

void CostMatrix::print(std::ostream& stream) const
{
    stream << "[";
    for(unsigned i = 0; i < size; i++)
    {
        if(i != 0) stream << ",\n";
        stream << "[";
        for(unsigned j = 0; j < size; j++)
        {
            if(j != 0) stream << ", ";
            stream << cost[i * size + j];
        }
        stream << "]";
    }
    stream << "]";
}

std::pair<std::unique_ptr<char[]>, unsigned> CostMatrix::serialize() const
{
    const unsigned dataSize = size * size * sizeof(cost[0]) + sizeof(size);
    std::unique_ptr<char[]> data = std::make_unique<char[]>(dataSize);
    char* start = data.get();

    std::memcpy(start, &size, sizeof(size));
    start += sizeof(size);

    for(unsigned i = 0; i < size * size; i++)
    {
        std::memcpy(start, &cost[i], sizeof(cost[i]));
        start += sizeof(cost[i]);
    }

    return std::make_pair(std::move(data), dataSize);
}

unsigned CostMatrix::deserialize(const char* data)
{
    std::memcpy(&size, data, sizeof(size));
    const char* start = data + sizeof(size);

    cost = std::make_unique<int[]>(size * size);
    for (unsigned i = 0; i < size * size; i++)
    {
        std::memcpy(&cost[i], start, sizeof(cost[i]));
        start += sizeof(cost[i]);
    }

    return start - data;
}

void CostMatrix::loadFromArray(int** matrix, unsigned size)
{
    this->size = size;

    cost = std::unique_ptr<int[]>(new int[size * size]);
    
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            cost[i * size + j] = matrix[i][j];
        }
    }
}


std::ostream& operator<<(std::ostream& stream, const CostMatrix& costMatrix)
{
    costMatrix.print(stream);
    return stream;
}