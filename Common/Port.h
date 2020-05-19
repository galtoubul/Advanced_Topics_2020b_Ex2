/**
 * Port Summary:
 *
 * Contains the class of Port and its related functions:
 *
 * Container ctor               -  custom ctor which acts as empty ctor as well
 * getPortId                    -  returns the port's id (symbol)
 * getContainersToUnload        -  returns the container's weight
 * addContainerToUnloadToPort              -  returns the container's weight
 * getLocation        -  returns the container's weight
 * isFutile           -  returns true if the container is a futile container. Otherwise, returns false.
 * setLocation        -  sets the container's location with the given location.
 */

#include <iostream>
#include <string>
#include <vector>
using std::string;
using std::vector;
class Container;

class Port {
    string id;
    vector<Container*> containersToUnload;

public:

    Port(const string& _portId = "UNINITIALIZED") : id(_portId) {}

    const string& getPortId() const;

    void addContainerToUnloadToPort(Container* container);

    const vector<Container*>& getContainersToUnload() const;

    friend std::ostream& operator<<(std::ostream& out, const Port& port);

};
