#include "include/Constants.hpp"
#include "include/CostMatrix.hpp"
#include "include/RoomsAssignment.hpp"
#include "include/AnnealingWorker.hpp"
#include "Buffer.hpp"

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <upcxx/upcxx.hpp>

int main(int argc, char *argv[])
{   
    
    // std::ifstream file("MatrixCost.dat");
    // std::ifstream file("testData10People.dat");
    // std::ifstream file("testData100People.dat");
    std::ifstream file("testData1000People.dat");
    if (!file.is_open()) {
        std::cerr << "Unable to open file.\n";
        return 1;
    }


    upcxx::init();
    int myid = upcxx::rank_me();
    int numprocs = upcxx::rank_n();
    int root;

    // upcxx::dist_object<RoomsAssignment> lowestCostSolution;
    AnnealingTask currentTask;
    AnnealingWorker worker;

    if (myid == 0)
    {
        std::unique_ptr<CostMatrix> costMatrixToSend = std::make_unique<CostMatrix>();
        costMatrixToSend->loadFromTxt(file);

        // currentTask.numRooms = 7;
        // currentTask.numRooms = 55;
        currentTask.numRooms = 1020;
        currentTask.costMatrix = std::move(costMatrixToSend);
        currentTask.worseSolutionAcceptanceProbability = 0.99;
        currentTask.worseSolutionAcceptanceProbabilityRange = std::pair<float, float>(0.75, 1.0);
        currentTask.worseSolutionAcceptanceProbabilityDecreaseRate = 0.01;
        currentTask.iterationsPerAcceptanceProbability = 10000;
        currentTask.randomGeneratorSeed = 42;

        // std::cout<<currentTask<<std::endl<<std::endl;
    }

    while(true)
    {
        // Create local serialized task object
        std::pair<std::unique_ptr<char[]>, unsigned> serializedTask;
        // Serialize task in master
        if (myid == 0)
        {
            serializedTask = currentTask.serialize();
        }

        upcxx::dist_object<unsigned> serializedTaskSize(serializedTask.second);
        if (myid != 0)
        {
            // Fetch serialized task size from master
            *serializedTaskSize = serializedTaskSize.fetch(0).wait();
            serializedTask.second = *serializedTaskSize;

            // Allocate memory for serialized task in workers
            serializedTask.first = std::make_unique<char[]>(serializedTask.second);
        }
        upcxx::barrier();
        
        unsigned receivedDataSize = 0;
        upcxx::dist_object<Buffer> buffer(0);
        char* dataChunkBegin = serializedTask.first.get();
        while(receivedDataSize < serializedTask.second)
        {
            const unsigned chunkSize = Buffer::BUFER_SIZE < serializedTask.second - receivedDataSize ? Buffer::BUFER_SIZE : serializedTask.second - receivedDataSize;
            if (myid == 0)
            {
                // Copy a chunk of serialized data to the buffer in master
                std::memcpy((*buffer).data, dataChunkBegin, chunkSize);
            }
            upcxx::barrier();
            
            if(myid != 0)
            {
                // Fetch a chunk of data from master
                *buffer = buffer.fetch(0).wait();
                std::memcpy(dataChunkBegin, (*buffer).data, chunkSize);
            }

            receivedDataSize += chunkSize;
            dataChunkBegin += chunkSize;
            upcxx::barrier();
        }

        upcxx::dist_object<int> lowestCost(std::numeric_limits<int>::max());
        if(myid != 0)
        {
            // Run workers
            currentTask.deserialize(serializedTask.first.get());
            currentTask.randomGeneratorSeed = myid * 2137;
            worker.setTask(std::move(currentTask));
            worker.run();

            // Set lowestCost with the cost of the lowest cost lution found by worker
            currentTask = AnnealingTask(worker.getTask());
            *lowestCost = currentTask.lowestCost;
        }
        upcxx::barrier();

        // Set lowest cost solution worker id in master
        upcxx::dist_object<int> lowestCostWorkerId(0);
        if(myid == 0)
        {
            for(int i = 1; i < numprocs; i++)
            {
                int workerLowestCost = lowestCost.fetch(i).wait();
                if(workerLowestCost < *lowestCost)
                {
                    *lowestCost = workerLowestCost;
                    *lowestCostWorkerId = i;
                }
            }
        }
        upcxx::barrier();

        // Fetch lowest cost solution worker id in workers
        if(myid != 0)
        {
            *lowestCostWorkerId = lowestCostWorkerId.fetch(0).wait();
        }
        upcxx::barrier();

        // Serialize solution in lowest cost solution worker
        std::pair<std::unique_ptr<char[]>, unsigned> serializedSolution;
        if(myid == *lowestCostWorkerId)
        {
            serializedSolution = currentTask.solution->serialize();
        }

        upcxx::dist_object<unsigned> serializedSolutionSize(serializedSolution.second);
        // Fetch serialized solution size from lowest cost solution worker
        *serializedSolutionSize = serializedSolutionSize.fetch(*lowestCostWorkerId).wait();
        serializedSolution.second = *serializedSolutionSize;
        
        if(myid == 0)
        {
            // Allocate memory for serialized solution in master
            serializedSolution.first = std::make_unique<char[]>(serializedSolution.second);
        }
        upcxx::barrier();

        receivedDataSize = 0;
        if(myid == 0 || myid == *lowestCostWorkerId)
        {
            dataChunkBegin = serializedSolution.first.get();
        }

        while(receivedDataSize < serializedSolution.second)
        {
            const unsigned chunkSize = Buffer::BUFER_SIZE < serializedSolution.second - receivedDataSize ? Buffer::BUFER_SIZE : serializedSolution.second - receivedDataSize;
            
            if(myid == *lowestCostWorkerId)
            {
                // Copy a chunk of serialized data to the buffer in lowest cost solution worker
                std::memcpy((*buffer).data, dataChunkBegin, chunkSize);
            }
            upcxx::barrier();

            if(myid == 0)
            {
                // Fetch a chunk of serialized data from lowest cost solution worker
                *buffer = buffer.fetch(*lowestCostWorkerId).wait();
                std::memcpy(dataChunkBegin, (*buffer).data, chunkSize);
            }

            receivedDataSize += chunkSize;
            dataChunkBegin += chunkSize;
            upcxx::barrier();
        }

        // Set lowest cost solution and lowest cost in master
        upcxx::dist_object<bool> breakMainLoop(false);

        if(myid == 0)
        {
            currentTask.solution->deserialize(serializedSolution.first.get());
            currentTask.lowestCost = *lowestCost;

            std::cout<<"All workers finished tasks with worse solution acceptance probability range: ["<<currentTask.worseSolutionAcceptanceProbabilityRange.first<< ", " <<currentTask.worseSolutionAcceptanceProbabilityRange.second<<"]\t Best solution: cost: "<<currentTask.lowestCost<<"\tworker: "<<*lowestCostWorkerId<<std::endl;

            // Update task
            currentTask.worseSolutionAcceptanceProbability = currentTask.worseSolutionAcceptanceProbabilityRange.first;
            if(currentTask.worseSolutionAcceptanceProbability >= 0.75)
            {
                currentTask.worseSolutionAcceptanceProbabilityRange = std::pair<float, float>(0.5, 0.75);
                currentTask.worseSolutionAcceptanceProbabilityDecreaseRate = 0.005;
                currentTask.iterationsPerAcceptanceProbability = 100000;
            }
            else if(currentTask.worseSolutionAcceptanceProbability >= 0.5)
            {
                currentTask.worseSolutionAcceptanceProbabilityRange = std::pair<float, float>(0.25, 0.5);
                currentTask.worseSolutionAcceptanceProbabilityDecreaseRate = 0.0025;
                currentTask.iterationsPerAcceptanceProbability = 100000;
            }
            else if(currentTask.worseSolutionAcceptanceProbability >= 0.25)
            {
                currentTask.worseSolutionAcceptanceProbabilityRange = std::pair<float, float>(0.1, 0.25);
                currentTask.worseSolutionAcceptanceProbabilityDecreaseRate = 0.001;
                currentTask.iterationsPerAcceptanceProbability = 10000;
            }
            else if(currentTask.worseSolutionAcceptanceProbability >= 0.1)
            {
                currentTask.worseSolutionAcceptanceProbabilityRange = std::pair<float, float>(0.01, 0.1);
                currentTask.worseSolutionAcceptanceProbabilityDecreaseRate = 0.0001;
            }
            else
            {
                *breakMainLoop = true;
            }
        }

        if(myid != 0)
        {
            *breakMainLoop = breakMainLoop.fetch(0).wait();
        }
        upcxx::barrier();

        if(*breakMainLoop) break;
    }

    if(myid == 0)
    {
        std::cout<<"Program finished. Best solution cost: "<<currentTask.lowestCost<<std::endl;
        // std::cout<<"Best solution: "<<*currentTask.solution.get()<<std::endl;
    }

    upcxx::finalize();

    return 0;
}