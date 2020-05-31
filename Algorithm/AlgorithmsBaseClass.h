#pragma once

#include <tuple>
#include <string>
#include <vector>
#include "../Interfaces/AbstractAlgorithm.h"
#include "../Common/Parser.h"
#include "../Interfaces/WeightBalanceCalculator.h"
using std::vector;
using std::tuple;
using std::string;

class AlgorithmsBaseClass : public AbstractAlgorithm {
protected:
    ShipPlan shipPlan;
    ShipRoute shipRoute;
    WeightBalanceCalculator calculator;
    int errors;

public:
    AlgorithmsBaseClass() : shipPlan(), shipRoute(), calculator() {}

    int readShipPlan(const std::string& full_path_and_file_name) override{
        errors |= Parser::readShipPlan(shipPlan, full_path_and_file_name);
        return errors;
    }

    int readShipRoute(const std::string& full_path_and_file_name) override{
        errors |= Parser::readShipRoute(shipRoute, full_path_and_file_name);
        return errors;
    }

    int calcVisitNum (const string& input_full_path_and_file_name);
    tuple<int,int,int> findLoc (const string& containerID);
    int setWeightBalanceCalculator(WeightBalanceCalculator& calculator) override;
    int getInstructionsForCargo(const std::string& input_full_path_and_file_name, const std::string& output_full_path_and_file_name) override;
    virtual void getUnloadingInstructions(vector<INSTRUCTION>& instructions, int currPortIndex) = 0;
    virtual void unloadToPort(Container& container, vector<INSTRUCTION>& instructions, Port& port) = 0;
    void getLoadingInstructions(vector<INSTRUCTION>& instructions, vector<Container>& containersAwaitingAtPort, int currPortIndex);
    void loadToShip(Container& container, vector<INSTRUCTION>& instructions, int currPortIndex);
};
