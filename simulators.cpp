#include "simulators.h"
#include <algorithm>
#include <limits>

// Helper: create empty frames vector
static std::vector<int> make_empty(int n) {
    return std::vector<int>(n, -1);
}

//
// ---------------------------- FIFO ----------------------------
//
ResultSummary FIFO_Simulator::run() {
    ResultSummary result;
    std::vector<int> frames = make_empty(framesCount);
    int pointer = 0;

    for (int page : reference) {
        SimulationStep step;
        step.reference = page;

        // Check HIT
        bool hit = false;
        for (int f : frames) {
            if (f == page) { hit = true; break; }
        }
        step.hit = hit;

        if (hit) {
            result.page_hits++;
        } else {
            result.page_faults++;

            // Replace using FIFO pointer
            frames[pointer] = page;
            pointer = (pointer + 1) % framesCount;
        }

        step.frames = frames;
        result.steps.push_back(step);
    }
    return result;
}

//
// ----------------------------- LRU -----------------------------
//
ResultSummary LRU_Simulator::run() {
    ResultSummary result;
    std::vector<int> frames = make_empty(framesCount);
    std::vector<int> lastUse(framesCount, -1);

    for (int i = 0; i < (int)reference.size(); ++i) {
        int page = reference[i];

        SimulationStep step;
        step.reference = page;

        bool hit = false;

        // Check HIT
        for (int j = 0; j < framesCount; j++) {
            if (frames[j] == page) {
                hit = true;
                lastUse[j] = i;
                break;
            }
        }
        step.hit = hit;

        if (hit) {
            result.page_hits++;
        } else {
            result.page_faults++;

            // Find empty frame
            int index = -1;
            for (int j = 0; j < framesCount; j++) {
                if (frames[j] == -1) {
                    index = j;
                    break;
                }
            }

            // If full → replace least recently used
            if (index == -1) {
                int lruIndex = 0;
                int minVal = lastUse[0];

                for (int j = 1; j < framesCount; j++) {
                    if (lastUse[j] < minVal) {
                        minVal = lastUse[j];
                        lruIndex = j;
                    }
                }
                index = lruIndex;
            }

            frames[index] = page;
            lastUse[index] = i;
        }

        step.frames = frames;
        result.steps.push_back(step);
    }

    return result;
}

//
// --------------------------- OPTIMAL ---------------------------
//
ResultSummary Optimal_Simulator::run() {
    ResultSummary result;
    std::vector<int> frames = make_empty(framesCount);

    for (int i = 0; i < (int)reference.size(); ++i) {
        int page = reference[i];

        SimulationStep step;
        step.reference = page;

        bool hit = false;
        for (int f : frames) {
            if (f == page) { hit = true; break; }
        }
        step.hit = hit;

        if (hit) {
            result.page_hits++;
        } else {
            result.page_faults++;

            // Find an empty frame
            int index = -1;
            for (int j = 0; j < framesCount; j++) {
                if (frames[j] == -1) {
                    index = j;
                    break;
                }
            }

            // If full → find optimal victim
            if (index == -1) {
                int farthest = -1;
                int farIndex = -1;

                for (int j = 0; j < framesCount; j++) {
                    int currentPage = frames[j];
                    int nextUse = std::numeric_limits<int>::max();

                    // Find future use
                    for (int k = i + 1; k < (int)reference.size(); k++) {
                        if (reference[k] == currentPage) {
                            nextUse = k;
                            break;
                        }
                    }

                    // Choose page used farthest in future
                    if (nextUse > farthest) {
                        farthest = nextUse;
                        farIndex = j;
                    }
                }

                index = farIndex;
            }

            frames[index] = page;
        }

        step.frames = frames;
        result.steps.push_back(step);
    }

    return result;
}
