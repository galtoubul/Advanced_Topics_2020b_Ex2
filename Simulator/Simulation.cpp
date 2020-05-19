#include <string>
#include "../Interfaces/WeightBalanceCalculator.h"
#include <iostream>
#include <filesystem>
#include "Simulation.h"

using std::vector;
using std::tuple;
using std::get;
using std::ofstream;
using std::string;
using std::cout;
using std::endl;
#define ERROR -1
#define VALID 1
#define CANNOTRUNTRAVEL ((1 << 3) | (1 <<4) | (1 << 7) | (1 << 8))

int Simulator::algorithmActionsCounter;
size_t Simulator::currPortIndex;

inline void clearData(ShipPlan& shipPlan, ShipRoute& shipRoute){
    const_cast<VVVC&>(shipPlan.getContainers()).clear();
    const_cast<vector<Port>&>(shipRoute.getPortsList()).clear();
}

void Simulator::initSimulation (unique_ptr<AbstractAlgorithm> algorithm, int travelNum){
    string travelName = "Travel" + std::to_string(travelNum);

    errorsFileName = "output" + string(1, std::filesystem::path::preferred_separator) +
                     "errors" + string(1, std::filesystem::path::preferred_separator);
                     //travelName + "_" + std::to_string(algorithmNum) + ".errors.txt"; // TODO

    string shipPlanPath = travelName +  std::string(1, std::filesystem::path::preferred_separator) + "Ship Plan.txt";
    string shipRoutePath = travelName + std::string(1, std::filesystem::path::preferred_separator) + "Route.txt";
    this->getInput(shipPlanPath, shipRoutePath);

    int travelErrors = this->getInput(shipPlanPath, shipRoutePath);

    if ((CANNOTRUNTRAVEL & travelErrors) != 0) {
        ofstream errorsFile(errorsFileName);
        for (int i = 1; i <= (1 << 18); i *= 2) {
            if ((i & travelErrors) > 0) {
                errorsFile << ErrorsInterface::errorsMap[i] << "\n";
            }
        }
        errorsFile << "Travel errors occurred. Skipping travel.";
        errorsFile.close();
        clearData(this->shipPlan, this->shipRoute);
        return;
    }

    int errorsOfAlgorithm = 0;
    errorsOfAlgorithm |= algorithm->readShipPlan(shipPlanPath);
    errorsOfAlgorithm |= algorithm->readShipRoute(shipRoutePath);

    WeightBalanceCalculator _calculator;
    algorithm->setWeightBalanceCalculator(_calculator);
    setWeightBalanceCalculator(_calculator);

    string algorithmErrorString;

    errorsOfAlgorithm |= startTravel(algorithm.get(), travelName, algorithmErrorString);

    //TODO: errors of Bad algorithm behavior
    if (errorsOfAlgorithm != 0) {
        ofstream errorsFile(errorsFileName);
        for (int i = 1; i <= (1 << 18); i *= 2) {
            if ((i & errorsOfAlgorithm) > 0) {
                errorsFile << ErrorsInterface::errorsMap[i] << "\n";
            }
        }
        if ((errorsOfAlgorithm & (1 << 9)) > 0) {
            errorsFile << algorithmErrorString;
        }

        errorsFile.close();
    }
    //TODO: bad algo behavior = -1 in the travel-algorithm pair
    /*
        if (status == ERROR)
            cout << travelName << " was ended with an error for algorithm" << algorithmNum
                 << " .The number of algorithm operations: " << algorithmActionsCounter << endl;
        else
            cout << travelName << " was ended successfully for algorithm" << algorithmNum
                 << " .The number of algorithm operations: " << algorithmActionsCounter << endl;*/

    clearData(this->shipPlan, this->shipRoute);
}

int Simulator::getInput(const string& shipPlanFileName, const string& shipRouteFileName){
    int errors = 0;
    errors |= Parser::readShipPlan(this->shipPlan, shipPlanFileName);
    errors |= Parser::readShipRoute(this->shipRoute, shipRouteFileName);
    return errors;
}

int Simulator::checkLoadInstruction(int x, int y, int floor, Container* container, string& algorithmErrorString){
    if(shipPlan.getContainers()[x][y][floor] != nullptr && shipPlan.getContainers()[x][y][floor]->isFutile()){
        algorithmErrorString = ErrorsInterface::buildNotLegalOperationError("Loading", container->getId(), floor, x, y, "this location is blocked");
        return ERROR;
    } else if (shipPlan.getContainers()[x][y][floor] != nullptr && !shipPlan.getContainers()[x][y][floor]->isFutile()){
        algorithmErrorString = ErrorsInterface::buildNotLegalOperationError("Loading", container->getId(), floor, x, y, "this location is occupied by another container");
        return ERROR;
    }else if (shipPlan.getContainers()[x][y][floor - 1] == nullptr){
        algorithmErrorString = ErrorsInterface::buildNotLegalOperationError("Loading", container->getId(), floor, x, y, "there isn't any container under the loaded container");
        return ERROR;
    } else if (calculator.tryOperation('L', container->getWeight(), x, y) != WeightBalanceCalculator::APPROVED){
        algorithmErrorString = ErrorsInterface::buildNotLegalOperationError("Loading", container->getId(), floor, x, y, "the operation isn't approved by the weight balance calculator");
        return ERROR;
    }
    else{
        this->shipPlan.setContainers(x, y, floor, new Container(container));
        delete(container);
        return VALID;
    }
}

int Simulator::checkUnloadInstruction(int x, int y, int floor, Container* container, vector<Container*>& containersAwaitingAtPort, string& algorithmErrorString){
    if (shipPlan.getContainers()[x][y][floor]->isFutile()){
        algorithmErrorString = ErrorsInterface::buildNotLegalOperationError("Unloading", container->getId(), floor, x, y, "this location is blocked");
        return ERROR;
    } else if (floor != (int)shipPlan.getContainers()[x][y].size() - 1 && shipPlan.getContainers()[x][y][floor + 1] != nullptr){
        algorithmErrorString = ErrorsInterface::buildNotLegalOperationError("Unloading", container->getId(), floor, x, y, "there are containers above the unloaded container");
        return ERROR;
    } else if (calculator.tryOperation('L', container->getWeight(), x, y) != WeightBalanceCalculator::APPROVED){
        algorithmErrorString = ErrorsInterface::buildNotLegalOperationError("Unloading", container->getId(), floor, x, y, "the operation isn't approved by the weight balance calculator");
        return ERROR;
    }
    else {
        this->shipPlan.setContainers(x, y, floor, nullptr);
        containersAwaitingAtPort.push_back(new Container(container));
        return VALID;
    }
}

inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

int Simulator::checkAndCountAlgorithmActions(vector<Container*>& containersAwaitingAtPort, const string& outputFileName,
                                             const string& currPortSymbol, string& algorithmErrorString){
    vector<INSTRUCTION> instructions;
    getInstructionsForPort(outputFileName, instructions);
    for (INSTRUCTION instruction : instructions) {
        char instructionType;
        string containerId;
        int x, y, floor;
        std::tie(instructionType, containerId, floor, x, y) = instruction;

        Container *container = nullptr;
        if (instructionType == 'R')
            continue;
        else if (instructionType == 'L'){
            algorithmActionsCounter++;
            vector<Container*> currContainersAwaitingAtPort = containersAwaitingAtPort;
            int locInVec = -1;
            for (Container *_container : currContainersAwaitingAtPort) {
                locInVec++;
                if (_container->getId() == ltrim(containerId)) {
                    container = _container;
                    break;
                }
            }
            containersAwaitingAtPort.erase(containersAwaitingAtPort.begin()+locInVec);
            if (container == nullptr) {
                algorithmErrorString = ErrorsInterface::buildNotLegalOperationError("Loading", containerId, floor, x, y, "this container isn't exist at "+currPortSymbol);
                return ERROR;
            }
            if (checkLoadInstruction(x, y, floor, container, algorithmErrorString) == ERROR)
                return ERROR;
            continue;
        }
        else if (instructionType == 'U'){
            algorithmActionsCounter++;
            container = shipPlan.getContainers()[x][y][floor];
            if (container == nullptr) {
                algorithmErrorString = ErrorsInterface::buildNotLegalOperationError("Unloading", containerId, floor, x, y,"this container isn't exist at Ship");
                return ERROR;
            }
            if (checkUnloadInstruction(x, y, floor, container, containersAwaitingAtPort, algorithmErrorString) == ERROR)
                return ERROR;
        }
    }

    for (Container* _container : containersAwaitingAtPort) {
        if (findPortIndex(shipRoute, currPortSymbol, currPortIndex) == NOT_IN_ROUTE)
            continue;
        if (_container->getDestination() != currPortSymbol){
            if(freeSlotsInShip() > 0){
                algorithmErrorString = ErrorsInterface::buildContainerForgottenError(currPortSymbol);
                return ERROR;
            }
        }
    }

    for (int x = 0; x < this->shipPlan.getPivotXDimension(); x++){
        for (int y = 0; y < this->shipPlan.getPivotYDimension(); y++){
            for (int floor = 0; floor < this->shipPlan.getFloorsNum(); floor++){
                if (this->shipPlan.getContainers()[x][y][floor] != nullptr &&
                    this->shipPlan.getContainers()[x][y][floor]->getDestination() == currPortSymbol){
                    algorithmErrorString = ErrorsInterface::buildContainerWasntDroppedError(currPortSymbol);
                    return ERROR;
                }
            }
        }
    }
    return VALID;
}


void Simulator::writeNotLegalOperation(const string&){
    //TODO: write func
}

int Simulator::startTravel (AbstractAlgorithm* algorithm, const string& travelName, string& algorithmErrorString) {
    Simulator::currPortIndex = 0;
    Simulator::algorithmActionsCounter = 0;
    int errors = 0;
    for (const Port &port : this->shipRoute.getPortsList()) {
        Simulator::currPortIndex++;
        string inputFileName, outputFileName;
        bool isFinalPort = currPortIndex == this->shipRoute.getPortsList().size();

        //finding portVisitNum
        int portVisitNum = 0;
        for (size_t i = 0; i <= Simulator::currPortIndex; ++i) {
            if (this->shipRoute.getPortsList()[i].getPortId() == port.getPortId())
                portVisitNum++;
        }

        getPortFilesName(inputFileName, outputFileName, port.getPortId(), portVisitNum, travelName);

        vector<Container *> containersAwaitingAtPort;
        readContainersAwaitingAtPort(inputFileName, containersAwaitingAtPort, isFinalPort, shipPlan, shipRoute,
                                     currPortIndex); //Errors here will be written in the same func of the next step

        errors |= algorithm->getInstructionsForCargo(inputFileName, outputFileName);
        int status = checkAndCountAlgorithmActions(containersAwaitingAtPort, outputFileName, port.getPortId(), algorithmErrorString);
        if (status == VALID)
            continue;
        else{
            errors |= (1 << 19); //It means there is a bad algorithm behavior, error string is algorithmErrorString
            return errors;
        }
    }
    return errors;
}

const ShipPlan& Simulator::getShipPlan () const{
    return this->shipPlan;
}

const ShipRoute& Simulator::getShipRoute() const{
    return this->shipRoute;
}

std::ostream& operator<<(std::ostream& out, const Simulator& simulator){
    out << "Ship Plan: " << '\n';
    simulator.getShipPlan().printShipPlan();
    out << simulator.getShipRoute() << '\n';
    return out;
}

int Simulator::freeSlotsInShip() {
    int counter = 0;
    for (int x = 0; x < this->shipPlan.getPivotXDimension(); x++)
        for (int y = 0; y < this->shipPlan.getPivotYDimension(); y++)
            for (int floor = 0; floor < this->shipPlan.getFloorsNum(); floor++)
                if (this->shipPlan.getContainers()[x][y][floor] == nullptr)
                    counter++;
    return counter;
}