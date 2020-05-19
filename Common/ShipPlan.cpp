#include "ShipPlan.h"

int ShipPlan::getFloorsNum() const{
    return floorsNum;
}

int ShipPlan::getPivotXDimension() const {
    return dimX;
}

int ShipPlan::getPivotYDimension() const{
    return dimY;
}

const VVVC& ShipPlan::getContainers() const{
    return this->containers;
}

void ShipPlan::setContainers(int x, int y, int floor, Container* container){
    this->containers[x][y][floor] = container;
    if (container == nullptr)
        return;
    containers[x][y][floor]->setLocation(x, y, floor);
}

void ShipPlan::removeContainer (int x, int y, int floor){
    free(this->containers[x][y][floor]);
    this->containers[x][y][floor] = nullptr;
}

void ShipPlan::printShipPlan () const {
    for (size_t i = 0; i < containers.size(); ++i) {
        for (size_t j = 0; j < containers[0].size(); ++j) {
            for (size_t k = 0; k <containers[0][0].size() ; ++k) {
                if (containers[i][j][k] != nullptr)
                    std::cout << "containers[" << i << "][" << j << "][" << k << "] = " << *containers[i][j][k]<< std::endl;
            }
        }
    }
}