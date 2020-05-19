#include <iostream>
#include <filesystem>
#include <string>
#include "../Interfaces/WeightBalanceCalculator.h"
#include "_308394642_a.h"
#define PORT_SYMBOL_LENGTH 6
#define PORT_VISIT_NUM_START_IND_AFTER_SLASH 8
using std::string;
using std::cout;
using std::vector;
using std::tuple;
using std::endl;
using std::list;
using std::get;

REGISTER_ALGORITHM (_308394642_a)

int _308394642_a::setWeightBalanceCalculator(WeightBalanceCalculator& _calculator){
    this->calculator = _calculator;
    return 0; // TODO: manage errors
}

int findCurrPortIndex(const ShipRoute& shipRoute, const string& portSymbol, int visitNum){
    int counter = 0;
    int currPortIndex = BEFORE_FIRST_PORT;
    for (const Port& port : shipRoute.getPortsList()){
        currPortIndex++;
        if (portSymbol == port.Port::getPortId()){
            counter++;
            if (counter == visitNum)
                break;
        }
    }
    return currPortIndex;
}

int _308394642_a::getInstructionsForCargo(const std::string& input_full_path_and_file_name, const std::string& output_full_path_and_file_name){
    int slashInd = input_full_path_and_file_name.find(std::filesystem::path::preferred_separator);
    string portSymbol = input_full_path_and_file_name.substr(slashInd + 1, PORT_SYMBOL_LENGTH);

    int dotInd = input_full_path_and_file_name.find('.');
    int visitNumLength = dotInd - (slashInd + 8);
    string visitNumString = input_full_path_and_file_name.substr(slashInd + PORT_VISIT_NUM_START_IND_AFTER_SLASH, visitNumLength);
    int visitNum = stoi(visitNumString);

    size_t currPortIndex = findCurrPortIndex(this->shipRoute, portSymbol, visitNum);

    vector<INSTRUCTION> instructions;
    getUnloadingInstructions(instructions, currPortIndex);

    // TODO: readContainersAwaitingAtPort even if final port -> and if it isn't empty mark as error
    bool isFinalPort = currPortIndex == shipRoute.getPortsList().size() -1;
    if (!isFinalPort) {
    vector<Container*> containersAwaitingAtPort;
        errors |= readContainersAwaitingAtPort(input_full_path_and_file_name, containersAwaitingAtPort, isFinalPort, shipPlan, shipRoute, currPortIndex);    getLoadingInstructions(instructions, containersAwaitingAtPort, currPortIndex);
    }

    ofstream instructionsForCargoFile (output_full_path_and_file_name);
    writeInstructionsToFile(instructions, instructionsForCargoFile);
    return 0; // TODO: manage errors
}

void _308394642_a::getUnloadingInstructions(vector<INSTRUCTION>& instructions, int currPortIndex){
    for (Container* container : shipRoute.getPortsList()[currPortIndex].getContainersToUnload()){
        unloadToPort(container, instructions);
    }
}

void _308394642_a::unloadToPort(Container* container, vector<INSTRUCTION>& instructions){
    size_t floorOfContainer, x, y;
    std::tie(x, y, floorOfContainer) = container->getLocation();

    size_t currFloor = this->shipPlan.getContainers()[x][y].size() - 1; //start from highest floor of x,y
    vector<INSTRUCTION> containersToLoadBack;
    Container* currContainer;

    // unloading all containers above the desired container
    while(currFloor != floorOfContainer){
        currContainer = this->shipPlan.getContainers()[x][y][currFloor];
        if (currContainer == nullptr){
            currFloor--;
            continue;
        }
        if(this->calculator.tryOperation('U', currContainer->getWeight(), x, y) == WeightBalanceCalculator::APPROVED){
            instructions.emplace_back('U',currContainer->getId(), currFloor, x, y);
            containersToLoadBack.emplace_back('L', currContainer->getId(), currFloor - 1, x, y);
            currFloor--;
        }
    }

    currContainer = this->shipPlan.getContainers()[x][y][currFloor];
    instructions.emplace_back('U', currContainer->getId(), currFloor, x, y);
    this->shipPlan.removeContainer(x, y, currFloor);

    int i = containersToLoadBack.size() - 1;
    currFloor++;
    while (currFloor != this->shipPlan.getContainers()[x][y].size() && i >= 0) {
        currContainer = this->shipPlan.getContainers()[x][y][currFloor];
        currContainer->setLocation(x,y,currFloor - 1);
        this->shipPlan.setContainers(x,y,currFloor - 1, currContainer);
        instructions.push_back(containersToLoadBack[i]);
        i--;
        currFloor++;
    }
    this->shipPlan.setContainers(x,y,currFloor - 1, nullptr);
}

void _308394642_a::getLoadingInstructions(vector<INSTRUCTION>& instructions, vector<Container*> containersAwaitingAtPort
        , int currPortIndex){
    for (Container* container : containersAwaitingAtPort){
        string portDest = container->getDestination();
        if (findPortIndex(this->shipRoute, portDest, currPortIndex) == NOT_IN_ROUTE) {
            instructions.emplace_back('R', container->getId(), NOT_IN_ROUTE, NOT_IN_ROUTE, NOT_IN_ROUTE);
        }
    }
    vector<Container*> sortedContainersAwaitingAtPort = orderContainersByDest(containersAwaitingAtPort, shipRoute, currPortIndex);
    for (Container* container : sortedContainersAwaitingAtPort){
        if (container->isRejected()){
            instructions.emplace_back('R', container->getId(), NOT_IN_ROUTE, NOT_IN_ROUTE, NOT_IN_ROUTE);
            continue;
        }
        loadToShip(container, instructions, currPortIndex);
    }
}

void _308394642_a::loadToShip(Container* container, vector<INSTRUCTION>& instructions, int currPortIndex){
    string portDest = container->getDestination();
    if (findPortIndex(this->shipRoute, portDest, currPortIndex) == NOT_IN_ROUTE) {
        instructions.emplace_back('R', container->getId(), NOT_IN_ROUTE, NOT_IN_ROUTE, NOT_IN_ROUTE);
        return;
    }
    for (int x = 0; x < this->shipPlan.getPivotXDimension(); x++){
        for (int y = 0; y < this->shipPlan.getPivotYDimension(); y++){
            for (int floor = 0; floor < this->shipPlan.getFloorsNum(); floor++){
                if (this->shipPlan.getContainers()[x][y][floor] != nullptr){
                    continue;
                }
                else{
                    this->shipPlan.setContainers(x, y, floor, container); //for now put in the first free spot
                    container->setLocation(x, y, floor);
                    if(this->calculator.tryOperation('U', container->getWeight(), x, y) == WeightBalanceCalculator::APPROVED){
                        instructions.emplace_back('L', container->getId(), floor, x, y);
                        (const_cast<Port&>(shipRoute.getPortsList()[findPortIndex(this->shipRoute, portDest, currPortIndex)])).addContainerToUnloadToPort(container);
                        return;
                    }
                }
            }
        }
    }
}




