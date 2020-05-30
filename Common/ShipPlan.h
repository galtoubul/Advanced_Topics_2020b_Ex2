#include <iostream>
#include <vector>
#include <memory>
#include "Container.h"
using std::vector;
typedef vector<vector<vector<std::unique_ptr<Container>>>> VVVC;
#define NOT_IN_ROUTE -2

class ShipPlan{
    int dimX;
    int dimY;
    int floorsNum;
    VVVC containers;

public:
    explicit ShipPlan() : dimX(NOT_ON_SHIP), dimY(NOT_ON_SHIP), floorsNum(NOT_ON_SHIP) {}

    void init(int _dimX, int _dimY, int _floorsNum);

    ShipPlan(const ShipPlan& other) = delete;

    int getFloorsNum() const;

    int getPivotXDimension() const;

    int getPivotYDimension() const;

    const VVVC& getContainers() const;

    void setContainers(int x, int y, int floor, Container& container);

    void addFutileContainer(int x, int y, int floor);

    void removeContainer (int x, int y, int floorNum);

    void printShipPlan () const;
};