#pragma once

#include <tuple>
#include <string>
#include <vector>
#include "../Interfaces/AlgorithmRegistration.h"
#include "../Common/Parser.h"
#include "../Interfaces/WeightBalanceCalculator.h"
using std::vector;
using std::tuple;
using std::string;
#define BEFORE_FIRST_PORT -1

class _308394642_a : public AbstractAlgorithm {
    ShipPlan shipPlan;
    ShipRoute shipRoute;
    WeightBalanceCalculator calculator;
    int errors;

public:
    _308394642_a() : shipPlan(), shipRoute(), calculator() {}

    int readShipPlan(const std::string& full_path_and_file_name) override{
        errors |= Parser::readShipPlan(this->shipPlan, full_path_and_file_name);
        return errors;
    }

    int readShipRoute(const std::string& full_path_and_file_name) override{
        errors |= Parser::readShipRoute(this->shipRoute, full_path_and_file_name);
        return errors;
    }

    int setWeightBalanceCalculator(WeightBalanceCalculator& calculator) override;

    int getInstructionsForCargo(const std::string& input_full_path_and_file_name, const std::string& output_full_path_and_file_name) override;

    void getUnloadingInstructions(vector<INSTRUCTION>& instructions, int currPortIndex);

    void getLoadingInstructions(vector<INSTRUCTION>& instructions, vector<Container*> containersAwaitingAtPort, int currPortIndex);

    void unloadToPort(Container* container, vector<INSTRUCTION>& instructions);

    void loadToShip(Container* container, vector<INSTRUCTION>& instructions, int currPortIndex);
};
