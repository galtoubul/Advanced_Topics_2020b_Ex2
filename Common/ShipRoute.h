#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "Port.h"
using std::vector;

class ShipRoute{
    vector<Port> portsList;

public:
    ShipRoute () : portsList() {}

    const vector<Port>& getPortsList() const;

    void addPort(const string& portId);

    friend std::ostream& operator<<(std::ostream& out, const ShipRoute& shipRoute);
};
