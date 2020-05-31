#pragma once

#include <vector>
#include <tuple>
#include <string>
#include <memory>
#include <functional>
#include <string>
#include "../Interfaces/WeightBalanceCalculator.h"
#include "../Interfaces/AbstractAlgorithm.h"
#include "../Common/Parser.h"
using std::vector;
using std::tuple;
using std::unique_ptr;

class Simulator{
    ShipPlan shipPlan;
    ShipRoute shipRoute;
    WeightBalanceCalculator calculator;
    vector<Travel> travelsVec;
    string errorsFileName;

public:
    static int algorithmActionsCounter;
    static size_t currPortIndex;

    Simulator() : shipPlan(), shipRoute() {}

    void setWeightBalanceCalculator(WeightBalanceCalculator& _calculator);

    void setErrorsFileName(const string& _errorsFileName){
        errorsFileName = _errorsFileName;
    }

    string getErrorsFileName(){
        return errorsFileName;
    }

    int getInput(const string& shipPlanFileName, const string& shipRouteFileName);

    int startTravel (AbstractAlgorithm* algorithm, const string& algName, Travel& travel, string& algorithmErrorString, const string& output);

    ShipPlan& getShipPlan();

    ShipRoute& getShipRoute();

    int freeSlotsInShip ();

    int checkAndCountAlgorithmActions(vector<Container>& containersAwaitingAtPort, const string& outputFileName, const string& portSymbol, string& algorithmErrorString);

    int checkLoadInstruction(int x, int y, int floor, Container& container, string& algorithmErrorString);

    int checkMoveInstruction(int x1, int y1, int floor1, int x2, int y2, int floor2, Container& container, string& algorithmErrorString);

    int checkUnloadInstruction(int x, int y, int floor, Container& container, string& algorithmErrorString);

    vector<Travel>& initTravelsVec(const string& travelsPath);

    void makeTravelError(int travelErrors, const string& output, tuple<string,vector<int>,int,int> algoTuple, int& numErrors);

    void createSimulationResults(ofstream& simulationResults, vector<tuple<string,vector<int>,int,int>> outputVector);
};

