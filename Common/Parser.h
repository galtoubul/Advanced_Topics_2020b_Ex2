#include <fstream>
#include <vector>
#include <list>
#include <tuple>
#include <string>
#include "ShipRoute.h"
#include "ShipPlan.h"
#include "../Interfaces/WeightBalanceCalculator.h"
#include "../Interfaces/ErrorsInterface.h"
#include "ISO6346.h"
using std::ifstream;
using std::ofstream;
using std::tuple;
//#define INSTRUCTION tuple<char,string,int,int,int>
#define INSTRUCTION tuple<char,string,int,int,int,int,int,int>
//#define REJECT 'R'

namespace Parser{
    int readShipPlan (ShipPlan& shipPlan, const string& shipPlanFileName);

    int readShipRoute(ShipRoute& shipRoute, const string& shipPlanFileName);
}

int readContainersAwaitingAtPort (const string& inputFileName, vector<Container*>& containersAwaitingAtPort, bool isFinalPort, const ShipPlan& shipPlan, const ShipRoute& shipRoute, int currPortIndex);

void writeInstructionsToFile(vector<INSTRUCTION>& instructions, ofstream& instructionsForCargoFile);

void getPortFilesName(string& inputFileName, string& outputFileName, const string& portId, const int portVisitNum, const string& travelName);

int findPortIndex(const ShipRoute& shipRoute, const string& portSymbol, int currPortIndex);

vector<Container*> orderContainersByDest(vector<Container*>& containersAwaitingAtPort, ShipRoute& shipRoute, int currPortIndex);

void getInstructionsForPort(const string& outputFileName, vector<INSTRUCTION>& instructions);