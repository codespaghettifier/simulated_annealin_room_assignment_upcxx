#include "../include/AnnealingWorker.hpp"

#include <cstdlib>
#include <ctime>
#include <iostream>

void AnnealingWorker::setTask(AnnealingTask&& task)
{
    this->task = std::move(task);
}

void AnnealingWorker::run()
{
    if(!task.isRunnable()) throw std::logic_error("AnnealingWorker cannot run a task.");

    if (!task.solution || task.numRooms != task.solution->getNumRooms())
    {
        task.solution = std::make_unique<RoomsAssignment>(task.numRooms, std::make_unique<CostMatrix>(*task.costMatrix));
        task.solution->calculateNaiveSolution();
    }

    currentSolution = std::make_unique<RoomsAssignment>(*task.solution);

    std::srand(task.randomGeneratorSeed);

    while(task.worseSolutionAcceptanceProbability >= task.worseSolutionAcceptanceProbabilityRange.first)
    {
        long iteration = 0;
        while(iteration < task.iterationsPerAcceptanceProbability)
        {
            // Generate random swap
            // swapRoomA == swapRoomB is ok (has no effect)
            unsigned swapRoomA = std::rand() % task.numRooms;
            unsigned swapRoomB = std::rand() % task.numRooms;
            bool personAFirst = std::rand() % 2;
            bool personBFirst = std::rand() % 2;

            if(currentSolution->getSwapCost(swapRoomA, personAFirst, swapRoomB, personBFirst) <= 0 || static_cast<float>(std::rand()) / RAND_MAX <= task.worseSolutionAcceptanceProbability)
            {
                currentSolution->swap(swapRoomA, personAFirst, swapRoomB, personBFirst);
                if(currentSolution->getCost() < task.lowestCost) onLowerCostSolutionFound();
            }

            iteration++;
        }
        task.worseSolutionAcceptanceProbability -= task.worseSolutionAcceptanceProbabilityDecreaseRate;
    }

    if(task.worseSolutionAcceptanceProbability < task.worseSolutionAcceptanceProbabilityRange.first) onFinish();
}

void AnnealingWorker::onLowerCostSolutionFound()
{
    // Test
    // std::cout << "Lower cost solution found, cost: " << currentSolution->getCost() << std::endl;

    task.lowestCost = currentSolution->getCost();
    task.solution = std::make_unique<RoomsAssignment>(*currentSolution);
    task.solution->calculateCost();
}

void AnnealingWorker::onFinish()
{
    // // Test implementation
    // std::cout << "Task finished with solution: \n"
    //     << *currentSolution.get() << std::endl;

    // std::cout << "Task finished with best solution: \n"
    //     << *task.solution.get() << std::endl;

    // std::cout << "Task finished with lowest cost: " << task.lowestCost << std::endl;
}
