#pragma once

#include <tuple>
#include <string>
#include <vector>
#include "../Interfaces/AlgorithmRegistration.h"
#include "AlgorithmsBaseClass.h"
using std::vector;
using std::tuple;
using std::string;

class _308394642_a : public AlgorithmsBaseClass {
public:
    void getUnloadingInstructions(vector<INSTRUCTION>& instructions, int currPortIndex) override;
    void unloadToPort(Container& container, vector<INSTRUCTION>& instructions, Port& port) override;
};
