#include <filesystem>
#include <string>
#include "_308394642_b.h"
#define SEPARATOR std::filesystem::path::preferred_separator
#define PORT_SYMBOL_LENGTH 6
using std::string;
using std::cout;
using std::vector;
using std::tuple;
using std::endl;
using std::list;
using std::get;

REGISTER_ALGORITHM (_308394642_b)

void _308394642_b::getUnloadingInstructions(vector<INSTRUCTION>& instructions, int currPortIndex) {
    for (auto &container : shipRoute.getPortsList()[currPortIndex].getContainersToUnload()) {
        if (shipRoute.getPortsList()[currPortIndex].isStillOnPort(container.getId()))
            unloadToPort(container, instructions, shipRoute.getPortsList()[currPortIndex]);
    }
}

std::tuple<int,int,int> _308394642_b::findEmptySpot(int x, int y){
    for (int i = 0; i < shipPlan.getPivotXDimension(); i++) {
        for (int j = 0; j < shipPlan.getPivotYDimension(); j++) {
            for (int k = 0; k < shipPlan.getFloorsNum(); k++) {
                if (i != x && j != y && shipPlan.getContainers()[i][j][k] == nullptr)
                    return {i, j, k};
            }
        }
    }
    return {-1, -1, -1};
}

void _308394642_b::unloadToPort(Container& container, vector<INSTRUCTION>& instructions, Port& port){
    int floorOfContainer, x, y;
    std::tie(x, y, floorOfContainer) = findLoc(container.getId());
    int currFloor = (int) shipPlan.getContainers()[x][y].size() - 1; //start from highest floor of x,y
    vector<INSTRUCTION> containersToLoadBack;
    std::unique_ptr<Container> currContainer;
    string currPort = shipPlan.getContainers()[x][y][floorOfContainer]->getDestination();

    // unloading all containers above the desired container
    while (shipPlan.getContainers()[x][y][currFloor] == nullptr)
        currFloor--;
    while(currFloor != floorOfContainer){
        if(calculator.tryOperation('U', shipPlan.getContainers()[x][y][currFloor]->getWeight(), x, y) == WeightBalanceCalculator::APPROVED) {
            if (shipPlan.getContainers()[x][y][currFloor]->getDestination() == currPort) { // unload the container
                instructions.emplace_back('U', shipPlan.getContainers()[x][y][currFloor]->getId(), currFloor, x, y, -1, -1, -1);
                port.removeContainer(shipPlan.getContainers()[x][y][currFloor]->getId());
                shipPlan.removeContainer(x, y, currFloor);
                currFloor--;
            } else { // move the container
                int row, col, floor;
                std::tie(row, col, floor) = findEmptySpot(x, y);
                while (calculator.tryOperation('L', shipPlan.getContainers()[x][y][currFloor]->getWeight(), row, col) != WeightBalanceCalculator::APPROVED)
                    std::tie(row, col, floor) = findEmptySpot(x, y);
                instructions.emplace_back('M', shipPlan.getContainers()[x][y][currFloor]->getId(), currFloor, x, y, floor, row, col);
                shipPlan.setContainers(row, col, floor, *shipPlan.getContainers()[x][y][currFloor]);
                shipPlan.removeContainer(x, y, currFloor);
                currFloor--;
            }
        }
    }
    // unloading the desired container
    instructions.emplace_back('U', shipPlan.getContainers()[x][y][currFloor]->getId(), currFloor, x, y, -1, -1, -1);
    shipPlan.removeContainer(x, y, currFloor);
}
