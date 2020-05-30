#pragma once

#include <tuple>
#include <string>
#include <vector>
#include "../Interfaces/AlgorithmRegistration.h"
#include "AlgorithmsBaseClass.h"
using std::vector;
using std::tuple;
using std::string;

class _308394642_b : public AlgorithmsBaseClass {
public:
    void getUnloadingInstructions(vector<INSTRUCTION>& instructions, int currPortIndex) override;
    void unloadToPort(Container& container, vector<INSTRUCTION>& instructions, Port& port) override;
    std::tuple<int,int,int> findEmptySpot(int x, int y);
};
