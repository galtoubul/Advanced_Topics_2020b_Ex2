#include <filesystem>
#include <string>
#include "../Interfaces/WeightBalanceCalculator.h"
#include "_308394642_a.h"
#define PORT_SYMBOL_LENGTH 6
#define PORT_VISIT_NUM_START_IND_AFTER_SLASH 8
#define SEPARATOR std::filesystem::path::preferred_separator
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

int calcVisitNum (const string& input_full_path_and_file_name){
    string visitNumString;
    if(input_full_path_and_file_name.substr(input_full_path_and_file_name.size() - 3) == "txt")
        visitNumString = input_full_path_and_file_name.substr(input_full_path_and_file_name.size() -
                                                              string("x.cargo_data.txt").size(), 1);
    else // ends with .cargo_data
        visitNumString = input_full_path_and_file_name.substr(input_full_path_and_file_name.size() -
                                                              string("x.cargo_data").size(), 1);
    cout << visitNumString << endl;
    return stoi(visitNumString);
}

int _308394642_a::getInstructionsForCargo(const std::string& input_full_path_and_file_name, const std::string& output_full_path_and_file_name){
    string portSymbol = input_full_path_and_file_name.substr(input_full_path_and_file_name.find_last_of(SEPARATOR) + 1, PORT_SYMBOL_LENGTH);
    int visitNum = calcVisitNum (input_full_path_and_file_name);
    size_t currPortIndex = findCurrPortIndex(this->shipRoute, portSymbol, visitNum);
    vector<INSTRUCTION> instructions;
    getUnloadingInstructions(instructions, currPortIndex);
    bool isFinalPort = currPortIndex == shipRoute.getPortsList().size() -1;
    vector<Container*> containersAwaitingAtPort;
    errors |= readContainersAwaitingAtPort(input_full_path_and_file_name, containersAwaitingAtPort, isFinalPort, shipPlan, shipRoute, currPortIndex);
    getLoadingInstructions(instructions, containersAwaitingAtPort, currPortIndex);

    ofstream instructionsForCargoFile (output_full_path_and_file_name);
    writeInstructionsToFile(instructions, instructionsForCargoFile);
    return errors;
}

void _308394642_a::getUnloadingInstructions(vector<INSTRUCTION>& instructions, int currPortIndex){
    for (Container* container : shipRoute.getPortsList()[currPortIndex].getContainersToUnload())
        unloadToPort(container, instructions);
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
//          instructions.emplace_back('U',currContainer->getId(), currFloor, x, y);
//          containersToLoadBack.emplace_back('L', currContainer->getId(), currFloor - 1, x, y);
            instructions.emplace_back('U',currContainer->getId(), currFloor, x, y, -1, -1, -1);
            containersToLoadBack.emplace_back('L', currContainer->getId(), currFloor - 1, x, y, -1, -1, -1);
            currFloor--;
        }
    }

    currContainer = this->shipPlan.getContainers()[x][y][currFloor];
//  instructions.emplace_back('U', currContainer->getId(), currFloor, x, y);
    instructions.emplace_back('U', currContainer->getId(), currFloor, x, y, -1, -1, -1);
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
//          instructions.emplace_back('R', container->getId(), NOT_IN_ROUTE, NOT_IN_ROUTE, NOT_IN_ROUTE);
            instructions.emplace_back('R', container->getId(), NOT_IN_ROUTE, NOT_IN_ROUTE, NOT_IN_ROUTE, -1, -1, -1);
        }
    }
    vector<Container*> sortedContainersAwaitingAtPort = orderContainersByDest(containersAwaitingAtPort, shipRoute, currPortIndex);
    for (Container* container : sortedContainersAwaitingAtPort){
        if (container->isRejected()){
//          instructions.emplace_back('R', container->getId(), NOT_IN_ROUTE, NOT_IN_ROUTE, NOT_IN_ROUTE);
            instructions.emplace_back('R', container->getId(), NOT_IN_ROUTE, NOT_IN_ROUTE, NOT_IN_ROUTE, -1, -1, -1);
            continue;
        }
        loadToShip(container, instructions, currPortIndex);
    }
}

void _308394642_a::loadToShip(Container* container, vector<INSTRUCTION>& instructions, int currPortIndex){
    string portDest = container->getDestination();
    if (findPortIndex(this->shipRoute, portDest, currPortIndex) == NOT_IN_ROUTE) {
        instructions.emplace_back('R', container->getId(), NOT_IN_ROUTE, NOT_IN_ROUTE, NOT_IN_ROUTE, -1, -1, -1);
//      instructions.emplace_back('R', container->getId(), NOT_IN_ROUTE, NOT_IN_ROUTE, NOT_IN_ROUTE);
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
                    if(this->calculator.tryOperation('L', container->getWeight(), x, y) == WeightBalanceCalculator::APPROVED){
//                      instructions.emplace_back('L', container->getId(), floor, x, y);
                        instructions.emplace_back('L', container->getId(), floor, x, y, -1, -1, -1);

                        (const_cast<Port&>(shipRoute.getPortsList()[findPortIndex(this->shipRoute, portDest, currPortIndex)])).addContainerToUnloadToPort(container);
                        return;
                    }
                }
            }
        }
    }
}
