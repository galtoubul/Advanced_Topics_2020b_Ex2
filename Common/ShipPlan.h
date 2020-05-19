#include <iostream>
#include <vector>
#include "Container.h"
using std::vector;
typedef vector<Container*> VC;
typedef vector<vector<Container*>> VVC;
typedef vector<vector<vector<Container*>>> VVVC;
#define NOT_IN_ROUTE -2

class ShipPlan{
    int dimX;
    int dimY;
    int floorsNum;
    VVVC containers;

public:
    explicit ShipPlan() : dimX(NOT_ON_SHIP), dimY(NOT_ON_SHIP), floorsNum(NOT_ON_SHIP) {}

    ShipPlan(int _dimX, int _dimY, int _floorsNum) :
            dimX(_dimX), dimY(_dimY), floorsNum(_floorsNum),
            containers(dimX, VVC(dimY, VC (_floorsNum, nullptr))) {}

    ShipPlan(const ShipPlan& other) = delete;

    int getFloorsNum() const;

    int getPivotXDimension() const;

    int getPivotYDimension() const;

    const VVVC& getContainers() const;

    void setContainers(int x, int y, int floor, Container* container);

    void removeContainer (int x, int y, int floorNum);

    void printShipPlan () const;
};