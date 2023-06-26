# pragma once

#include "AnnealingTask.hpp"

class AnnealingWorker
{
public:
    void setTask(AnnealingTask&& task);
    void run();
    inline AnnealingTask& getTask();

private:
    void onLowerCostSolutionFound();
    void onFinish();

    AnnealingTask task;
    std::unique_ptr<RoomsAssignment> currentSolution;
};

inline AnnealingTask& AnnealingWorker::getTask()
{
    return task;
}

