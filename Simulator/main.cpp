#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include <cstdlib>
#include <tuple>
#include <iostream>
#include <stdio.h>
#include <dlfcn.h>
#include <map>
#include <memory>
#include <filesystem>
#include <string>
#include "Simulation.h"
#include "Registrar.h"
using std::cout;
using std::cin;
using std::endl;
using std::unique_ptr;
using std::pair;
using std::get;
namespace fs = std::filesystem;
#define SEPERATOR string(1, fs::path::preferred_separator)
#define CANNOTRUNTRAVEL ((1 << 3) | (1 <<4) | (1 << 7) | (1 << 8))

string getCurrentDir() {
    char buff[FILENAME_MAX]; //create string buffer to hold path
    GetCurrentDir(buff, FILENAME_MAX);
    string currentWorkingDir(buff);
    return currentWorkingDir;
}

vector<fs::path> getAlgorithmsPaths(const string& algorithmPath){
    vector<fs::path> algorithmsPaths;
    std::error_code ec;
    for (const auto & entry : fs::directory_iterator(algorithmPath, ec)){
        string fileName = entry.path().string();
        if(string(".so") == (fileName.string::substr(fileName.size() - 3)))
            algorithmsPaths.emplace_back(fileName);
    }
    return algorithmsPaths;
}

vector<fs::path> getPaths (int argc, char** argv, string& travelPath, string& algorithmPath, string& output){
    vector<string> args(argv, argv + argc);
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "-algorithm_path")
            algorithmPath = args[i + 1];
        if (args[i] == "-output")
            output = args[i + 1];
        if (args[i] == "-travel_path")
            travelPath = args[i + 1];
    }
    if (output.empty())
        output = getCurrentDir();

    if (algorithmPath.empty())
        algorithmPath = getCurrentDir();

    return getAlgorithmsPaths(algorithmPath);
}

inline void clearData(ShipPlan& shipPlan, ShipRoute& shipRoute){
    const_cast<VVVC&>(shipPlan.getContainers()).clear();
    const_cast<vector<Port>&>(shipRoute.getPortsList()).clear();
}

ofstream initSimulationResults(const string& output, int travelNum){
    ofstream simulationResults(output + SEPERATOR + "simulation.results");
    simulationResults << "RESULTS,";
    for (int j = 1; j <= travelNum; ++j)
        simulationResults << "travel "+ to_string(j) +",";
    simulationResults << "SUM,";
    simulationResults << "NumErrors\n";
    return simulationResults;
}

int Simulator::algorithmActionsCounter;
size_t Simulator::currPortIndex;
std::map<int, std::string> Errors::errorsMap;

int main(int argc, char** argv) {
    string travelPath, algorithmPath, output;
    vector<fs::path> algorithmPaths = getPaths(argc, argv, travelPath, algorithmPath, output);
    if(travelPath.empty()){
        cout << "Fatal Error: missing -travel_path argument. Exiting..." << endl;
        return EXIT_FAILURE;
    }

//  Dynamically load algorithms
    auto& registrar = Registrar::getRegistrar();
    for (auto& algoPath : algorithmPaths){
        string path = algoPath.string();
        string algorithmName = path.substr(path.find_last_of(SEPERATOR) + 1, path.size() - path.find_last_of(SEPERATOR) - 4);
        string error;
        if (!registrar.loadAlgorithmFromFile(path.c_str(), error, algorithmName)) {
            std::cerr << error << '\n';
            return EXIT_FAILURE;
        }
    }

    Simulator simulator;
    auto travelsVec = simulator.initTravelsVec(travelPath);
    Errors::populateErrorsMap();
    ofstream simulationResults = initSimulationResults(output, travelsVec.size());

    vector<tuple<string,vector<int>,int,int>> outputVector;
    for(auto& algorithm : registrar.getAlgorithmMap()){
        tuple<string,vector<int>,int,int> algoTuple;
        get<0>(algoTuple) = algorithm.first;
        int sum = 0;
        int numErrors = 0;

        for(Travel& travel : travelsVec){
            unique_ptr<AbstractAlgorithm> alg = algorithm.second();
            simulator.setErrorsFileName(output + SEPERATOR + "errors" + SEPERATOR +
                                        algorithm.first + "_" + to_string(travel.getIndex()) + ".errors");
            int travelErrors = simulator.getInput(travel.getShipPlanPath().string(), travel.getShipRoutePath().string());
            if ((CANNOTRUNTRAVEL & travelErrors) != 0) {
                simulator.makeTravelError(travelErrors, output, algoTuple, numErrors);
                continue;
            }
            WeightBalanceCalculator _calculator;
            alg->setWeightBalanceCalculator(_calculator);
            simulator.setWeightBalanceCalculator(_calculator);

            int errorsOfAlgorithm = 0;
            errorsOfAlgorithm |= alg->readShipPlan(travel.getShipPlanPath().string());
            errorsOfAlgorithm |= alg->readShipRoute(travel.getShipRoutePath().string());
            if ((CANNOTRUNTRAVEL & errorsOfAlgorithm) != 0) {
                simulator.makeTravelError(errorsOfAlgorithm, output, algoTuple, numErrors);
                continue;
            }

            string algorithmErrorString;
            errorsOfAlgorithm |= simulator.startTravel(alg.get(), algorithm.first, travel, algorithmErrorString, output);

            if (errorsOfAlgorithm != 0) {
                std::error_code ec;
                fs::create_directory(output + SEPERATOR + "errors", ec);
                ofstream errorsFile(simulator.getErrorsFileName());
                for (int i = 1; i <= (1 << 18); i *= 2) {
                    if ((i & errorsOfAlgorithm) > 0)
                        errorsFile << Errors::errorsMap[i] << "\n";
                }
                if ((errorsOfAlgorithm & (1 << 19)) > 0) {
                    errorsFile << algorithmErrorString;
                    errorsFile.close();

                    get<1>(algoTuple).push_back(-1);
                    numErrors += 1;
                    clearData(simulator.getShipPlan(), simulator.getShipRoute());
                    continue;
                }
                errorsFile.close();
            }
            clearData(simulator.getShipPlan(), simulator.getShipRoute());
            sum += Simulator::algorithmActionsCounter;
            get<1>(algoTuple).push_back(Simulator::algorithmActionsCounter);
        }
        get<2>(algoTuple) = sum;
        get<3>(algoTuple) = numErrors;
        outputVector.push_back(algoTuple);
    }
    simulator.createSimulationResults(simulationResults, outputVector);
    return EXIT_SUCCESS;
}
