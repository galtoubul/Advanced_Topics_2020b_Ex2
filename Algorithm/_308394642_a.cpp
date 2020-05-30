#include <filesystem>
#include <string>
#include "../Interfaces/WeightBalanceCalculator.h"
#include "_308394642_a.h"
#define PORT_SYMBOL_LENGTH 6
#define SEPARATOR std::filesystem::path::preferred_separator
using std::string;
using std::cout;
using std::vector;
using std::tuple;
using std::endl;
using std::list;
using std::get;

REGISTER_ALGORITHM (_308394642_a)

void _308394642_a::getUnloadingInstructions(vector<INSTRUCTION>& instructions, int currPortIndex){
    for (auto& container : shipRoute.getPortsList()[currPortIndex].getContainersToUnload())
        unloadToPort(container, instructions, shipRoute.getPortsList()[currPortIndex]);
}

void _308394642_a::unloadToPort(Container& container, vector<INSTRUCTION>& instructions, Port& port){
    int floorOfContainer, x, y;
    std::tie(x, y, floorOfContainer) = findLoc(container.getId());
    int currFloor = (int) shipPlan.getContainers()[x][y].size() - 1; //start from highest floor of x,y
    vector<INSTRUCTION> containersToLoadBack;
    std::unique_ptr<Container> currContainer;

    // unloading all containers above the desired container
    while (shipPlan.getContainers()[x][y][currFloor] == nullptr && !port.getPortId().empty())
        currFloor--;
    while(currFloor != floorOfContainer){
        if(calculator.tryOperation('U', shipPlan.getContainers()[x][y][currFloor]->getWeight(), x, y) == WeightBalanceCalculator::APPROVED){
            instructions.emplace_back('U',shipPlan.getContainers()[x][y][currFloor]->getId(), currFloor, x, y, -1, -1, -1);
            containersToLoadBack.emplace_back('L', shipPlan.getContainers()[x][y][currFloor]->getId(), currFloor - 1, x, y, -1, -1, -1);
            currFloor--;
        }
    }

    // unloading the desired container
    instructions.emplace_back('U', shipPlan.getContainers()[x][y][currFloor]->getId(), currFloor, x, y, -1, -1, -1);
    shipPlan.removeContainer(x, y, currFloor);

    // loading back the containers that were above it
    int i = (int) containersToLoadBack.size() - 1;
    currFloor++;
    while (currFloor != (int) shipPlan.getContainers()[x][y].size() && i >= 0) {
        shipPlan.setContainers(x, y, currFloor - 1, *shipPlan.getContainers()[x][y][currFloor]);
        instructions.push_back(containersToLoadBack[i]);
        i--;
        currFloor++;
    }
    shipPlan.removeContainer(x, y, currFloor - 1);
}
