#include <fstream>
#include <vector>
#include <list>
#include <tuple>
#include <string>
#include "ShipRoute.h"
#include "Travel.h"
#include "../Interfaces/WeightBalanceCalculator.h"
#include "Errors.h"
#include "ISO6346.h"
using std::ifstream;
using std::ofstream;
using std::tuple;
#define INSTRUCTION tuple<char,string,int,int,int,int,int,int>

namespace Parser{
    int readShipPlan (ShipPlan& shipPlan, const string& shipPlanFileName);

    int readShipRoute(ShipRoute& shipRoute, const string& shipPlanFileName);
}

int readContainersAwaitingAtPort (const string& inputFileName, bool isFinalPort,
                                  vector<Container>& containersAwaitingAtPort, ShipPlan& shipPlan, ShipRoute& shipRoute, int currPortIndex);

void writeInstructionsToFile(vector<tuple<char, string, int, int, int, int, int, int>> &instructions, const string& output_full_path_and_file_name);

void getPortFilesName(string& inputFileName, string& outputFileName, const string& portId, const int portVisitNum, Travel& travel, const string& dir);

int findPortIndex(ShipRoute& shipRoute, const string& portSymbol, int currPortIndex);

void orderContainersByDest(vector<Container>& containersAwaitingAtPort, vector<Container>& sortedContainersAwaitingAtPort,
                           ShipRoute& shipRoute, int currPortIndex);

void getInstructionsForPort(const string& outputFileName, vector<INSTRUCTION>& instructions);

int findCurrPortIndex(ShipRoute& shipRoute, const string& portSymbol, int visitNum);
