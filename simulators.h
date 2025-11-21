#ifndef SIMULATORS_H
#define SIMULATORS_H

#include <vector>

struct SimulationStep {
    std::vector<int> frames; // -1 for empty
    int reference = -1;
    bool hit = false;
};

struct ResultSummary {
    int page_faults = 0;
    int page_hits = 0;
    std::vector<SimulationStep> steps;
};

class FIFO_Simulator {
public:
    FIFO_Simulator(int frames, const std::vector<int> &ref) : framesCount(frames), reference(ref) {}
    ResultSummary run();
private:
    int framesCount;
    std::vector<int> reference;
};

class LRU_Simulator {
public:
    LRU_Simulator(int frames, const std::vector<int> &ref) : framesCount(frames), reference(ref) {}
    ResultSummary run();
private:
    int framesCount;
    std::vector<int> reference;
};

class Optimal_Simulator {
public:
    Optimal_Simulator(int frames, const std::vector<int> &ref) : framesCount(frames), reference(ref) {}
    ResultSummary run();
private:
    int framesCount;
    std::vector<int> reference;
};

#endif // SIMULATORS_H
