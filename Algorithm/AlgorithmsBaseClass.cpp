#include "AlgorithmsBaseClass.h"
#include <filesystem>
#include <string>
#define PORT_SYMBOL_LENGTH 5
#define SEPARATOR std::filesystem::path::preferred_separator
using std::string;
using std::cout;
using std::vector;
using std::tuple;
using std::endl;
using std::list;
using std::get;

int AlgorithmsBaseClass::setWeightBalanceCalculator(WeightBalanceCalculator& _calculator){
    this->calculator = _calculator;
    return 0;
}

int AlgorithmsBaseClass::calcVisitNum (const string& input_full_path_and_file_name){
    string visitNumString;
    if(input_full_path_and_file_name.substr(input_full_path_and_file_name.size() - 3) == "txt")
        visitNumString = input_full_path_and_file_name.substr(input_full_path_and_file_name.size() -
                                                              string("x.cargo_data.txt").size(), 1);
    else // ends with .cargo_data
        visitNumString = input_full_path_and_file_name.substr(input_full_path_and_file_name.size() -
                                                              string("x.cargo_data").size(), 1);
    return stoi(visitNumString);
}

int AlgorithmsBaseClass::getInstructionsForCargo(const std::string& input_full_path_and_file_name, const std::string& output_full_path_and_file_name){
    string portSymbol = input_full_path_and_file_name.substr(input_full_path_and_file_name.find_last_of(SEPARATOR) + 1, PORT_SYMBOL_LENGTH);
    int visitNum = calcVisitNum (input_full_path_and_file_name);
    size_t currPortIndex = findCurrPortIndex(shipRoute, portSymbol, visitNum);
    vector<INSTRUCTION> instructions;

    getUnloadingInstructions(instructions, currPortIndex);

    bool isFinalPort = currPortIndex == shipRoute.getPortsList().size() - 1;
    vector<Container> containersAwaitingAtPort;
    errors |= readContainersAwaitingAtPort(input_full_path_and_file_name, isFinalPort, containersAwaitingAtPort,
                                           shipPlan, shipRoute, currPortIndex);

    getLoadingInstructions(instructions, containersAwaitingAtPort, currPortIndex);

    writeInstructionsToFile(instructions, output_full_path_and_file_name);
    return errors;
}

tuple<int,int,int> AlgorithmsBaseClass::findLoc (const string& containerID){
    for (int x = 0; x < shipPlan.getPivotXDimension(); x++)
        for (int y = 0; y < shipPlan.getPivotYDimension(); y++)
            for (int floor = 0; floor < shipPlan.getFloorsNum(); floor++)
                if(shipPlan.getContainers()[x][y][floor] != nullptr && shipPlan.getContainers()[x][y][floor]->getId() == containerID)
                    return {x, y, floor};
    return {-1, -1, -1};
}

void AlgorithmsBaseClass::getLoadingInstructions(vector<INSTRUCTION>& instructions, vector<Container>& containersAwaitingAtPort
        , int currPortIndex){
    for (auto& container : containersAwaitingAtPort){
        string portDest = container.getDestination();
        if (findPortIndex(this->shipRoute, portDest, currPortIndex) == NOT_IN_ROUTE)
            instructions.emplace_back('R', container.getId(), NOT_IN_ROUTE, NOT_IN_ROUTE, NOT_IN_ROUTE, -1, -1, -1);
    }
    vector<Container> sortedContainersAwaitingAtPort;
    orderContainersByDest(containersAwaitingAtPort, sortedContainersAwaitingAtPort, shipRoute, currPortIndex);
    for (auto& container : sortedContainersAwaitingAtPort){
        if (container.isRejected()){
            instructions.emplace_back('R', container.getId(), NOT_IN_ROUTE, NOT_IN_ROUTE, NOT_IN_ROUTE, -1, -1, -1);
            continue;
        }
        loadToShip(container, instructions, currPortIndex);
    }
}

void AlgorithmsBaseClass::loadToShip(Container& container, vector<INSTRUCTION>& instructions, int currPortIndex){
    string portDest = container.getDestination();
    if (findPortIndex(shipRoute, portDest, currPortIndex) == NOT_IN_ROUTE) {
        instructions.emplace_back('R', container.getId(), NOT_IN_ROUTE, NOT_IN_ROUTE, NOT_IN_ROUTE, -1, -1, -1);
        return;
    }

    // locate containers on ship
    for (int x = 0; x < shipPlan.getPivotXDimension(); x++){
        for (int y = 0; y < shipPlan.getPivotYDimension(); y++){
            for (int floor = 0; floor < shipPlan.getFloorsNum(); floor++){
                if(shipPlan.getContainers()[x][y][floor] == nullptr &&
                   calculator.tryOperation('L', container.getWeight(), x, y) == WeightBalanceCalculator::APPROVED){
                    instructions.emplace_back('L', container.getId(), floor, x, y, -1, -1, -1);
                    shipPlan.setContainers(x, y, floor, container);
                    (const_cast<Port&>(shipRoute.getPortsList()[findPortIndex(shipRoute, portDest, currPortIndex)])).addContainerToUnloadToPort(container);
                    return;
                }
            }
        }
    }
}
