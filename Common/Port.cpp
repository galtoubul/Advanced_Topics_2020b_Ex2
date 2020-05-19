#include "Port.h"
using std::vector;

const string& Port::getPortId() const{
    return this->id;
}

void Port::addContainerToUnloadToPort(Container* container) {
    containersToUnload.push_back(container);
}

const vector<Container*>& Port::getContainersToUnload() const{
    return containersToUnload;
}

std::ostream&operator<<(std::ostream& out, const Port& port){
    out << "id: " << port.id;
    return out;
}
