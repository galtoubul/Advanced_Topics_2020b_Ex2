#include <iostream>
#include <algorithm>
#include <regex>
#include <filesystem>
#include "Parser.h"
using std::string;
using std::cout;
using std::vector;
using std::tuple;
using std::endl;
using std::list;
using std::ofstream;
using std::get;
#define NOT_A_COMMENT_LINE 0
#define COMMENT_LINE 1

void split(vector<string>& elems, const string &s, char delim) {
    auto result = std::back_inserter(elems);
    std::istringstream iss(s);
    string item;
    while (std::getline(iss, item, delim)) {
        if (!item.empty())
            *result++ = item;
    }
}


bool isShipPlanLineValid (const string& line){ //TODO: check limits
    const std::regex regex("\\s*[0-9]\\s*[,]\\s*[0-9]\\s*[,]\\s*[0-9]\\s*");
    if (!(std::regex_match(line, regex))){
        //   INVALID_INPUT("ship plan")
        return false;
        //exit(EXIT_FAILURE);
    }
    return true;
}

int isCommentOrWS (const string& line){
    const std::regex regex("\\s*[#].*");
    const std::regex regexWS(R"(\s*\t*\r*\n*)"); //TODO maybe isspace is a better choice
    if (std::regex_match(line, regex) || std::regex_match(line, regexWS))
        return COMMENT_LINE;
    return NOT_A_COMMENT_LINE;
}

int Parser::readShipPlan (ShipPlan& shipPlan, const string& shipPlanFileName){
    int errors = 0;
    ifstream shipPlanInputFile(shipPlanFileName);
    vector<tuple<int, int, int>> vecForShipPlan;
    string line;
    int floorsNum, dimX, dimY;

    if (shipPlanInputFile.is_open()) {
        while (getline(shipPlanInputFile, line)) { //validating first line, if the format is ok we will continue
            if (isCommentOrWS(line))
                continue;
            if(!isShipPlanLineValid(line)){
                errors |= (1 << 3);
                return errors;
            } else{
                vector<string> temp;
                split(temp, line, ',');
                floorsNum = stoi(temp[0]);
                dimX = stoi(temp[1]);
                dimY = stoi(temp[2]);
                break;
            }
        }
        while (getline(shipPlanInputFile, line)) {
            if(isCommentOrWS(line))
                continue;

            if(!isShipPlanLineValid(line)){
                errors |= (1 << 2);
                continue;
            }

            vector<string> temp;
            split(temp, line, ',');

            bool isDuplicateAppearanceWithSameData  = false;
            for(tuple<int, int, int> data : vecForShipPlan){
                if (stoi(temp[0]) == get<0>(data) && stoi(temp[1]) == get<1>(data)){
                    if (stoi(temp[2]) != get<2>(data)){
                        errors |= (1 << 4);
                        return errors;                    }
                    else {
                        errors |= (1 << 2);
                        isDuplicateAppearanceWithSameData = true;
                        break;
                    }
                }
            }
            if (isDuplicateAppearanceWithSameData)
                continue;

            vecForShipPlan.emplace_back(stoi(temp[0]), stoi(temp[1]), stoi(temp[2]));
        }
        shipPlanInputFile.close();
    } else{
        //       UNABLE_TO_OPEN_FILE(shipPlanFileName)
        errors |= (1 << 3);
        return errors;
        //exit(EXIT_FAILURE);
    }

    shipPlan = ShipPlan(dimX, dimY, floorsNum);

    for (size_t i = 1; i < vecForShipPlan.size(); ++i) {
        int blockedFloors = floorsNum - get<2>(vecForShipPlan[i]);
        if (blockedFloors <= 0){
            errors |= (1 << 0);
            continue;
        }
        if (get<0>(vecForShipPlan[i]) >= dimX || get<1>(vecForShipPlan[i]) >= dimY){
            errors |= (1 << 1);
            continue;
        }
        for (int j = 0; j < blockedFloors; j++){
            Container* futileContainer = new Container();
            shipPlan.setContainers(get<0>(vecForShipPlan[i]), get<1>(vecForShipPlan[i]), j, futileContainer);
        }
    }
    return errors;
}

int checkIfValidPortId(string port){
    //have to be in model of: XX XXX - size 6
    const std::regex regex("\\s*[a-zA-z]{2}[ ][a-zA-z]{3}\\s*");
    if (!(std::regex_match(port, regex))){
        //     NON_LEGAL_SEA_PORT_CODE(port)
        return (1 << 13); // error code for "containers at port"
        //exit(EXIT_FAILURE);
    }
    return 0;
}

// trim white spaces from left
inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim white spaces from right
inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim white spaces from left & right
inline std::string& trim(std::string& s, const char* t = " \t\n\r\f\v")
{
    return ltrim(rtrim(s, t), t);
}

int Parser::readShipRoute(ShipRoute& shipRoute, const string& shipPlanFileName){
    int errors = 0;
    ifstream shipRouteInputFile (shipPlanFileName);
    string line;
    int currPortInd = 0;
    if (shipRouteInputFile.is_open()){
        while (getline(shipRouteInputFile,line)){
            if(isCommentOrWS(line))
                continue;

            line = trim(line);
            if (checkIfValidPortId(line) != 0){ //have to be in model of: XX XXX - size 6
                errors |= (1 << 6);
                continue;
            }

            std::transform(line.begin(), line.end(), line.begin(), [](unsigned char c){ return std::toupper(c);});

            //the same port can't appear in two consecutive lines
            if(!shipRoute.getPortsList().empty() && line == shipRoute.getPortsList()[currPortInd - 1].getPortId()){
                //           SAME_PORT_AS_PREV
                errors |= (1 << 5);
                //exit(EXIT_FAILURE);
                continue;
            }

            shipRoute.addPort(line);
        }
        shipRouteInputFile.close();
    }
    else{
        //   UNABLE_TO_OPEN_FILE(shipPlanFileName)
        errors |= (1 << 7);
        //exit(EXIT_FAILURE);
    }

    if (shipRoute.getPortsList().empty())
        errors |= (1 << 7);

    if (shipRoute.getPortsList().size() == 1)
        errors |= (1 << 8);

    return errors;
}

inline bool fileExists (const std::string& fileName) {
    ifstream f(fileName);
    return f.good();
}

void getPortFilesName(string& inputFileName, string& outputFileName, const string& portId, const int portVisitNum, const string& travelName){
    string str;
    inputFileName = travelName + string(1, std::filesystem::path::preferred_separator) +
                    portId + "_" + std::to_string(portVisitNum) + ".cargo_data.txt";

    // In this stage we just need the names, not validation therefore we delete this:

    /* if (!fileExists(inputFileName) && !isFinalPort) {
         PORT_FILE_NAME_ISNT_MATCHING(inputFileName)
         exit (EXIT_FAILURE);
     }*/
    //if (fileExists(inputFileName) && isFinalPort)
    //   LAST_PORT_WARNING

    outputFileName = travelName +  std::string(1, std::filesystem::path::preferred_separator) +
                     portId + '_' + std::to_string(portVisitNum) + ".instructions_for_cargo.txt";
}

//int validateContainerId (const string& line){
//    const std::regex regex("\\s*[A-Z]{3}[UJZ][0-9]{7}\\s*");
//    if (!(std::regex_match(line, regex))){
//        //    CONTAINER_ERROR("id")
//        return (1 << 14);
//        //exit(EXIT_FAILURE);
//    }
//    return 0;
//}

int validateWeight (const string& line){
    const std::regex regex("\\s*[0-9]*\\s*");
    if (!(std::regex_match(line, regex))){
        //    CONTAINER_ERROR("weight")
        return (1 << 12);
        //exit(EXIT_FAILURE);
    }
    return 0;
}

int validateContainersAwaitingAtPortLine (vector<string>& line) {
    if (line.size() != 3) {
        INVALID_INPUT("Containers awaiting at port")
        //TODO: check relevant code
        //exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 3; ++i)
        trim(line[i]);
    return (ISO6346::isValidId(line[0]) | validateWeight(line[1]) | checkIfValidPortId(line[2]));
    //   return portInRoute(shipRoute, line[2]);
}

int readContainersAwaitingAtPort (const string& inputFileName, vector<Container*>& containersAwaitingAtPort, bool isFinalPort,
                                  const ShipPlan& shipPlan, const ShipRoute& shipRoute, int currPortIndex){
    int errors = 0;
    ifstream inputFile (inputFileName);
    string line;
    bool rejected = false;
    if (inputFile.is_open()){
        if (isFinalPort) {
            errors |= (1 << 17);
            return errors;
        }
        while (getline(inputFile, line)){
            vector<string> temp;
            split(temp, line, ',');
            if(isCommentOrWS(line))
                continue;

            //port id to uppercase
            std::transform(temp[2].begin(), temp[2].end(), temp[2].begin(),
                           [](unsigned char c){ return std::toupper(c);});

            //check duplicate ID on port
            for (Container* container : containersAwaitingAtPort){
                if (container->getId().compare(temp[0]) == 0){
                    errors |= (1 << 10);
                    rejected = true;
                }
            }

            int validation = validateContainersAwaitingAtPortLine(temp);
            if(validation != 0){
                errors |= validation;
                rejected = true;
            }

            if (findPortIndex(shipRoute, temp[2], currPortIndex) == NOT_IN_ROUTE) {
                errors |= (1 << 13);
                rejected = true;
            }

            for (int x = 0; x < shipPlan.getPivotXDimension(); x++) {
                for (int y = 0; y < shipPlan.getPivotYDimension(); y++) {
                    for (int floor = 0; floor < shipPlan.getFloorsNum(); floor++) {
                        if (shipPlan.getContainers()[x][y][floor] != nullptr && !shipPlan.getContainers()[x][y][floor]->isFutile()
                            && shipPlan.getContainers()[x][y][floor]->getId().compare(temp[0]) == 0) {
                            errors|= (1 << 11);
                            rejected = true;
                        }
                    }
                }
            }

            containersAwaitingAtPort.push_back(new Container(stoi(temp[1]), temp[2], temp[0], false, rejected));
        }
        inputFile.close();
    }
    else if (!isFinalPort){
        //   UNABLE_TO_OPEN_FILE(inputFileName)
        errors |= (1 << 16); // "assuming no cargo to be loaded at this port" - we will get an empty vector and that's ok
        //exit(EXIT_FAILURE);
    }
    return errors;
}

void writeInstructionsToFile(vector<INSTRUCTION>& instructions, ofstream& instructionsForCargoFile)
{
    for (INSTRUCTION instruction : instructions){
        instructionsForCargoFile << get<0>(instruction) <<", " << get<1>(instruction);
        if (get<0>(instruction) == 'R'){
            instructionsForCargoFile << "\n";
            continue;
        }
        instructionsForCargoFile << ", " << get<2>(instruction) <<", "
                                 << get<3>(instruction) << ", "<< get<4>(instruction) << "\n";
    }
    instructionsForCargoFile << std::endl;
    instructionsForCargoFile.close();
}

int findPortIndex(const ShipRoute& shipRoute, const string& portSymbol, int currPortIndex){
    for (int i = currPortIndex +1; (size_t)i < shipRoute.getPortsList().size(); i++) {
        if (shipRoute.getPortsList()[i].getPortId() == portSymbol)
            return i;
    }
    return NOT_IN_ROUTE;
}

vector<Container*> orderContainersByDest(vector<Container*>& containersAwaitingAtPort, ShipRoute& shipRoute, int currPortIndex){
    vector<Container*> newContainersAwaitingAtPort;
    for (size_t i = currPortIndex + 1; i < shipRoute.getPortsList().size(); i++){
        for (Container* container : containersAwaitingAtPort){
            string destPort = container->getDestination();
            if (findPortIndex(shipRoute, destPort, (int)currPortIndex) == (int)i)
                newContainersAwaitingAtPort.push_back(container);
        }
    }
    return newContainersAwaitingAtPort;
}

void getInstructionsForPort(const string& outputFileName, vector<INSTRUCTION>& instructions) {
    ifstream outputFile(outputFileName);
    string line;
    if (outputFile.is_open()) {
        while (getline(outputFile, line)) {
            vector<string> temp;
            split(temp, line, ',');
            if(temp.size() < 5)
                continue;
            if(temp.size() == 5)
//                instructions.emplace_back(temp[0]. at(0), temp[1], stoi(temp[2]), stoi(temp[3]), stoi(temp[4]));
            instructions.emplace_back(temp[0]. at(0), temp[1], stoi(temp[2]), stoi(temp[3]), stoi(temp[4]), -1, -1, -1);

            if(temp.size() == 8)
//                instructions.emplace_back(temp[0]. at(0), temp[1], stoi(temp[2]), stoi(temp[3]), stoi(temp[4]));
            instructions.emplace_back(temp[0]. at(0), temp[1], stoi(temp[2]), stoi(temp[3]),
                                      stoi(temp[4]), stoi(temp[5]), stoi(temp[6]), stoi(temp[7]));

        }
        outputFile.close();
    } else {
        //TODO: check what to do in this situation
        UNABLE_TO_OPEN_FILE(outputFileName)
        exit(EXIT_FAILURE);
    }
}