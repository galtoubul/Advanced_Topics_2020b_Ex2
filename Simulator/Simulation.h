#pragma once

#include <vector>
#include <tuple>
#include <string>
#include <memory>
#include <functional>
#include "../Common/Parser.h"
#include <string>
#include "../Interfaces/WeightBalanceCalculator.h"
#include "../Interfaces/AbstractAlgorithm.h"
//#include "Registrar.h"

using std::vector;
using std::tuple;
using std::unique_ptr;

class Simulator{
    ShipPlan shipPlan;
    ShipRoute shipRoute;
    WeightBalanceCalculator calculator;
    string errorsFileName;

public:
    static int algorithmActionsCounter;

    static size_t currPortIndex;

    Simulator() : shipPlan(), shipRoute() {}

    void initSimulation (const std::function<unique_ptr<AbstractAlgorithm>()>& algorithmFactory, const string& algName, int travelNum);

    void setWeightBalanceCalculator(WeightBalanceCalculator& _calculator);

    int getInput(const string& shipPlanFileName, const string& shipRouteFileName);

    int startTravel (AbstractAlgorithm* algorithm, const string& travelName, string& algorithmErrorString);

    friend std::ostream& operator<<(std::ostream& out, const Simulator& simulator);

    const ShipPlan& getShipPlan () const;

    const ShipRoute& getShipRoute() const;

    int freeSlotsInShip ();

    int checkAndCountAlgorithmActions(vector<Container*>& containersAwaitingAtPort, const string& outputFileName, const string& portSymbol, string& algorithmErrorString);

    int checkLoadInstruction(int x, int y, int floor, Container* container, string& algorithmErrorString);

    int checkUnloadInstruction(int x, int y, int floor, Container* container,
                               vector<Container*>& containersAwaitingAtPort, string& algorithmErrorString);

    void writeNotLegalOperation(const string&);
};

