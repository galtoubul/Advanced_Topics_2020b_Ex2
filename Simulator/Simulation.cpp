#include <string>
#include "../Interfaces/WeightBalanceCalculator.h"
#include <iostream>
#include <filesystem>
#include "Simulation.h"
#include <cstddef>
using std::function;
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
#define SEPERATOR string(1, fs::path::preferred_separator)

inline void clearData(ShipPlan& shipPlan, ShipRoute& shipRoute){
    const_cast<VVVC&>(shipPlan.getContainers()).clear();
    const_cast<vector<Port>&>(shipRoute.getPortsList()).clear();
}

fs::path getPath(fs::directory_entry entry, const string& lookFor, const string& lookFor2){
    std::error_code ec;
    for (const auto& file : fs::directory_iterator(entry, ec)) {
        string fileName = file.path().string();
        if(fileName.substr(fileName.size() - lookFor.size()) == lookFor ||
           fileName.substr(fileName.size() - lookFor2.size()) == lookFor2)
            return file;
    }
    return "";
}

vector<Travel>& Simulator::initTravelsVec(const string& travelsPath){
    std::error_code ec;
    int index = 1;
    for (const auto& entry : fs::directory_iterator(travelsPath, ec)){
        if(entry.is_directory()){
            fs::path shipPlanPath = getPath(entry, ".ship_plan.txt", ".ship_plan");
            fs::path shipRoutePath = getPath(entry, ".route.txt", ".route");
            if(shipPlanPath.string().empty() || shipRoutePath.string().empty())
                continue;
            travelsVec.emplace_back(index, shipPlanPath, shipRoutePath, entry);
            index++;
        }
    }
    return travelsVec;
}

void Simulator::setWeightBalanceCalculator(WeightBalanceCalculator& _calculator){
    this->calculator = _calculator;
}

int Simulator::getInput(const string& shipPlanFileName, const string& shipRouteFileName){
    int errors = 0;
    errors |= Parser::readShipPlan(this->shipPlan, shipPlanFileName);
    errors |= Parser::readShipRoute(this->shipRoute, shipRouteFileName);
    return errors;
}

int Simulator::checkLoadInstruction(int x, int y, int floor, Container& container, string& algorithmErrorString){
    if(shipPlan.getContainers()[x][y][floor] != nullptr && shipPlan.getContainers()[x][y][floor]->isFutile()){
        algorithmErrorString = Errors::buildNotLegalOperationError("Loading", container.getId(), floor, x, y, "this location is blocked");
        return ERROR;
    } else if (shipPlan.getContainers()[x][y][floor] != nullptr && !shipPlan.getContainers()[x][y][floor]->isFutile()){
        algorithmErrorString = Errors::buildNotLegalOperationError("Loading", container.getId(), floor, x, y, "this location is occupied by another container");
        return ERROR;
    }else if (shipPlan.getContainers()[x][y][floor - 1] == nullptr){
        algorithmErrorString = Errors::buildNotLegalOperationError("Loading", container.getId(), floor, x, y, "there isn't any container under the loaded container");
        return ERROR;
    } else if (calculator.tryOperation('L', container.getWeight(), x, y) != WeightBalanceCalculator::APPROVED){
        algorithmErrorString = Errors::buildNotLegalOperationError("Loading", container.getId(), floor, x, y, "the operation isn't approved by the weight balance calculator");
        return ERROR;
    } else{
        shipPlan.setContainers(x, y, floor, container);
        container.setLocation(x, y, floor);
        return VALID;
    }
}

int Simulator::checkMoveInstruction(int x1, int y1, int floor1, int x2, int y2, int floor2, Container& container, string& algorithmErrorString){
    if (shipPlan.getContainers()[x1][y1][floor1]->isFutile()){
        algorithmErrorString = Errors::buildNotLegalOperationError("Moving", container.getId(), floor1, x1, y1, "this location is blocked");
        return ERROR;
    } else if (floor1 != (int)shipPlan.getContainers()[x1][y1].size() - 1 && shipPlan.getContainers()[x1][y1][floor1 + 1] != nullptr){
        algorithmErrorString = Errors::buildNotLegalOperationError("Moving", container.getId(), floor1, x1, y1, "there are containers above the unloaded container");
        return ERROR;
    } else if (calculator.tryOperation('U', container.getWeight(), x1, y1) != WeightBalanceCalculator::APPROVED){
        algorithmErrorString = Errors::buildNotLegalOperationError("Unload", container.getId(), floor1, x1, y1, "the operation isn't approved by the weight balance calculator");
        return ERROR;
    } else if(shipPlan.getContainers()[x2][y2][floor2] != nullptr && shipPlan.getContainers()[x2][y2][floor2]->isFutile()){
        algorithmErrorString = Errors::buildNotLegalOperationError("Moving", container.getId(), floor2, x2, y2, "this location is blocked");
        return ERROR;
    } else if (shipPlan.getContainers()[x2][y2][floor2] != nullptr && !shipPlan.getContainers()[x2][y2][floor2]->isFutile()){
        algorithmErrorString = Errors::buildNotLegalOperationError("Moving", container.getId(), floor2, x2, y2, "this location is occupied by another container");
        return ERROR;
    }else if (shipPlan.getContainers()[x2][y2][floor2 - 1] == nullptr){
        algorithmErrorString = Errors::buildNotLegalOperationError("Moving", container.getId(), floor2, x2, y2, "there isn't any container under the loaded container");
        return ERROR;
    } else if (calculator.tryOperation('L', container.getWeight(), x2, y2) != WeightBalanceCalculator::APPROVED){
        algorithmErrorString = Errors::buildNotLegalOperationError("Loading", container.getId(), floor2, x2, y2, "the operation isn't approved by the weight balance calculator");
        return ERROR;
    } else{
        int newX, newY, newZ;std::tie(newX, newY, newZ) = shipPlan.getContainers()[x1][y1][floor1]->getLocation();
        shipPlan.setContainers(x2, y2, floor2, *shipPlan.getContainers()[x1][y1][floor1]);
        shipPlan.removeContainer(x1, y1, floor1);
        std::tie(newX, newY, newZ) = shipPlan.getContainers()[x2][y2][floor2]->getLocation();
        return VALID;
    }
}

int Simulator::checkUnloadInstruction(int x, int y, int floor, Container& container, string& algorithmErrorString){
    if (shipPlan.getContainers()[x][y][floor]->isFutile()){
        algorithmErrorString = Errors::buildNotLegalOperationError("Unloading", container.getId(), floor, x, y, "this location is blocked");
        return ERROR;
    } else if (floor != (int)shipPlan.getContainers()[x][y].size() - 1 && shipPlan.getContainers()[x][y][floor + 1] != nullptr){
        algorithmErrorString = Errors::buildNotLegalOperationError("Unloading", container.getId(), floor, x, y, "there are containers above the unloaded container");
        return ERROR;
    } else if (calculator.tryOperation('U', container.getWeight(), x, y) != WeightBalanceCalculator::APPROVED){
        algorithmErrorString = Errors::buildNotLegalOperationError("Unloading", container.getId(), floor, x, y, "the operation isn't approved by the weight balance calculator");
        return ERROR;
    } else return VALID;
}

inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v"){
    s.erase(0, s.find_first_not_of(t));
    return s;
}

int Simulator::checkAndCountAlgorithmActions(vector<Container>& containersAwaitingAtPort, const string& outputFileName,
                                             const string& currPortSymbol, string& algorithmErrorString){
    vector<INSTRUCTION> instructions;
    getInstructionsForPort(outputFileName, instructions);

    for (INSTRUCTION& instruction : instructions) {
        char instructionType;
        string containerIdBeforeTrim;
        int x1, y1, floor1, x2, y2, floor2;
        std::tie(instructionType, containerIdBeforeTrim, floor1, x1, y1, floor2, x2, y2) = instruction;
        string containerId = ltrim(containerIdBeforeTrim);

        Container* container = nullptr;
        if (instructionType == 'R')
            continue;
        else if (instructionType == 'L'){
            algorithmActionsCounter++;

            // look for the loaded container at containersAwaitingAtPort
            for (auto& _container : containersAwaitingAtPort) {
                if (_container.getId() == ltrim(containerId)) {
                    container = &_container;
                    break;
                }
            }

            if (container == nullptr) {
                algorithmErrorString = Errors::buildNotLegalOperationError("Loading", containerId, floor1, x1, y1, "this container isn't exist at " + currPortSymbol);
                return ERROR;
            }

            if (checkLoadInstruction(x1, y1, floor1, *container, algorithmErrorString) == ERROR)
                return ERROR;

            continue;
        }
        else if (instructionType == 'U'){
            algorithmActionsCounter++;
            if (shipPlan.getContainers()[x1][y1][floor1] == nullptr) {
                algorithmErrorString = Errors::buildNotLegalOperationError("Unloading", containerId, floor1, x1, y1, "this container isn't exist at Ship");
                return ERROR;
            }
            if (checkUnloadInstruction(x1, y1, floor1, *shipPlan.getContainers()[x1][y1][floor1], algorithmErrorString) == ERROR)
                return ERROR;
            else{
                containersAwaitingAtPort.emplace_back(*shipPlan.getContainers()[x1][y1][floor1]);
                shipPlan.removeContainer(x1, y1, floor1);
            }
        }
        else if(instructionType == 'M'){
            algorithmActionsCounter++;
            if (shipPlan.getContainers()[x1][y1][floor1] == nullptr) {
                algorithmErrorString = Errors::buildNotLegalOperationError("Moving", containerId, floor1, x1, y1, "this container isn't exist at Ship");
                return ERROR;
            }
            if (checkMoveInstruction(x1, y1, floor1, x2, y2, floor2, *shipPlan.getContainers()[x1][y1][floor1], algorithmErrorString) == ERROR)
                return ERROR;
        }
    }
    // look for containers that were unloaded at the current port and shouldn't
    for (auto& _container : containersAwaitingAtPort) {
        if (findPortIndex(shipRoute, currPortSymbol, currPortIndex) != NOT_IN_ROUTE &&
            _container.getDestination() != currPortSymbol && freeSlotsInShip() > 0){
            algorithmErrorString = Errors::buildContainerForgottenError(currPortSymbol);
            return ERROR;
        }
    }

    // look for containers that weren't unloaded at the current port and should
    for (int x = 0; x < shipPlan.getPivotXDimension(); x++){
        for (int y = 0; y < shipPlan.getPivotYDimension(); y++){
            for (int floor = 0; floor < shipPlan.getFloorsNum(); floor++){
                if (shipPlan.getContainers()[x][y][floor] != nullptr &&
                    shipPlan.getContainers()[x][y][floor]->getDestination() == currPortSymbol){
                    algorithmErrorString = Errors::buildContainerWasntDroppedError(currPortSymbol);
                    return ERROR;
                }
            }
        }
    }
    return VALID;
}

int Simulator::startTravel (AbstractAlgorithm* algorithm, Travel& travel, string& algorithmErrorString, const string& output) {
    string travelAlgorithmDir = output + SEPERATOR + "_308394642_b_travel" + to_string(travel.getIndex()) + "_crane_instructions";
    fs::create_directory(travelAlgorithmDir);
    int errors = 0;
    Simulator::currPortIndex = -1;
    Simulator::algorithmActionsCounter = 0;
    for (const Port& port : shipRoute.getPortsList()) {
        Simulator::currPortIndex++;

        //finding portVisitNum
        int portVisitNum = 0;
        for (size_t i = 0; i <= Simulator::currPortIndex; ++i)
            if (this->shipRoute.getPortsList()[i].getPortId() == port.getPortId())
                portVisitNum++;

        string inputFileName, outputFileName;
        getPortFilesName(inputFileName, outputFileName, port.getPortId(), portVisitNum, travel, travelAlgorithmDir);

        // simulator is reading which containers are waiting on port
        vector<Container> containersAwaitingAtPort;
        bool isFinalPort = currPortIndex == this->shipRoute.getPortsList().size();
        readContainersAwaitingAtPort(inputFileName, isFinalPort, containersAwaitingAtPort, shipPlan, shipRoute, currPortIndex); //TODO: what about the status?

        // algorithm is reading the input and making actions on his ship plan
        //Errors here will be written in the same func of the next step
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

ShipPlan& Simulator::getShipPlan(){
    return shipPlan;
}

ShipRoute& Simulator::getShipRoute(){
    return shipRoute;
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

void Simulator::makeTravelError(int travelErrors, const string& output, tuple<string,vector<int>,int,int> algoTuple, int& numErrors){
    std::error_code ec;
    fs::create_directory(output + SEPERATOR + "errors", ec);
    ofstream errorsFile(errorsFileName);
    for (int i = 1; i <= (1 << 18); i *= 2){
        if ((i & travelErrors) > 0)
            errorsFile << Errors::errorsMap[i] << "\n";
    }
    errorsFile << "Travel errors occurred. Skipping travel.";
    errorsFile.close();
    clearData(shipPlan, shipRoute);
    get<1>(algoTuple).push_back(-1);
    numErrors += 1;
}

bool compareAlgoTuples(tuple<string,vector<int>,int,int> x, tuple<string,vector<int>,int,int> y){
    if (get<3>(x) == get<3>(y)){
        return (get<2>(x) < get<2>(y));
    }
    else{
        return (get<3>(x) < get<3>(y));
    }
}

void Simulator::createSimulationResults(ofstream& simulationResults, vector<tuple<string,vector<int>,int,int>> outputVector){
    sort(outputVector.begin(), outputVector.end(), compareAlgoTuples);
    for (tuple<string,vector<int>,int,int> algoTuple : outputVector){
        if (simulationResults.is_open()){
            simulationResults << get<0>(algoTuple) + ",";
            simulationResults.flush();
            for (int travelOpNum : get<1>(algoTuple)){
                simulationResults << to_string(travelOpNum) + ",";
                simulationResults.flush();
            }
            if (simulationResults.is_open())
                simulationResults << to_string(get<2>(algoTuple)) + "," << to_string(get<3>(algoTuple)) + "\n";
        }
    }
    simulationResults.close();
}
