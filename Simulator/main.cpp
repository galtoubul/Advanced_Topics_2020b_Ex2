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
#include <fstream>
#include <stdio.h>
#include <dlfcn.h>
#include <map>
#include <memory>
#include <filesystem>
#include <cstdint>
#include <string>
#include "Simulation.h"
#include "Registrar.h"
using std::cout;
using std::cin;
using std::endl;
using std::unique_ptr;
using std::pair;
using std::get;
#define NOT_LEGAL -1
#define LEGAL 1
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
        if(string(".so") == (fileName.string::substr(fileName.size() - 3))){
            algorithmsPaths.emplace_back(fileName);
        } // TODO: handle an error of not getting so files
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

bool compareAlgoTuples(tuple<string,vector<int>,int,int> x, tuple<string,vector<int>,int,int> y){
    if (get<3>(x) == get<3>(y)){
        return (get<2>(x) < get<2>(y));
    }
    else{
        return (get<3>(x) < get<3>(y));
    }
}

ofstream initSimulationResults(const string& output, int travelNum){
    cout << "main:  output dir = " << output << endl;
    ofstream simulationResults(output + SEPERATOR + "simulation.results.csv");
    simulationResults << "RESULTS,";
    for (int j = 1; j <= travelNum; ++j)
        simulationResults << "travel "+ to_string(j) +",";
    simulationResults << "SUM,";
    simulationResults << "NumErrors\n";
    return simulationResults;
}

int Simulator::algorithmActionsCounter;
size_t Simulator::currPortIndex;
std::map<int, std::string> ErrorsInterface::errorsMap;

int main(int argc, char** argv) {
    string travelPath, algorithmPath, output;
    vector<fs::path> algorithmPaths = getPaths(argc, argv, travelPath, algorithmPath, output);
    if(travelPath.empty()){
        cout << "Fatal Error: missing -travel_path argument. Exiting..." << endl;
        return EXIT_FAILURE;
    }

    // Dynamically load algorithms
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

//    std::error_code errorCode;
//    if(algorithmPaths.empty()){
//        cout << "algorithmPaths.empty()" << endl;
//        for (const auto& entry : fs::directory_iterator(algorithmPath, errorCode)){
//            string fileName = entry.path().string();
//            if(string(".so") == (fileName.string::substr(fileName.size() - 3))){
//                string algorithmName = fileName.string::substr(fileName.find_last_of(SEPERATOR) + 1, fileName.size() - fileName.find_last_of(SEPERATOR) - 4);
//                string path = algorithmPath + SEPERATOR + algorithmName + ".so";
//                std::string error;
//                if (!registrar.loadAlgorithmFromFile(path.c_str(), error, algorithmName)) {
//                    std::cerr << error << '\n';
//                    return EXIT_FAILURE;
//                }
//            }
//        }
//    } else{
//        cout << "!algorithmPaths.empty()" << endl;
//        for (auto& entry : algorithmPaths){
//            string path = entry.string();
//            string algorithmName = path.substr(path.find_last_of(SEPERATOR) + 1, path.size() - path.find_last_of(SEPERATOR) - 4);
//            std::string error;
//            if (!registrar.loadAlgorithmFromFile(path.c_str(), error, algorithmName)) {
//                std::cerr << error << '\n';
//                return EXIT_FAILURE;
//            }
//        }
//    }

    Simulator simulator;
    simulator.initTravelsVec(travelPath);
    auto travelsVec = simulator.getTravelsVec();
    ErrorsInterface::populateErrorsMap();
    ofstream simulationResults = initSimulationResults(output, travelsVec.size());

    vector<tuple<string,vector<int>,int,int>> outputVector;

    for(auto& algorithm : registrar.getAlgorithmMap()){
        cout << "main:  1st row of outer for(algorithm):   registrar.getAlgorithmMap().size() = " << registrar.getAlgorithmMap().size() << endl;
        tuple<string,vector<int>,int,int> algoTuple;
        get<0>(algoTuple) = algorithm.first;
        int sum = 0;
        int numErrors = 0;

        unique_ptr<AbstractAlgorithm> alg = algorithm.second();
        for(Travel& travel : travelsVec){
            cout << "main:  1st row of inner for(travel):   travel's num = " << travel.getIndex() << endl;
            simulator.errorsFileName = output + SEPERATOR + "errors" + SEPERATOR +
                                       travel.getDir().string() + "_" + algorithm.first + ".errors.txt"; // TODO: change to simulations.errors without a folder?
            cout << "main:  inner for(travel):  simulator.errorsFileName = " << simulator.errorsFileName << endl;
            int travelErrors = simulator.getInput(travel.getShipPlanPath().string(), travel.getShipRoutePath().string());
            if ((CANNOTRUNTRAVEL & travelErrors) != 0) {
                cout << "main:  inner for(travel):  if ((CANNOTRUNTRAVEL & travelErrors) != 0)" << endl;
                fs::create_directory(output + SEPERATOR + "errors");
                cout << "   created dir: " << output + SEPERATOR + "errors" << endl;
                ofstream errorsFile(simulator.errorsFileName);
                for (int i = 1; i <= (1 << 18); i *= 2)
                    if ((i & travelErrors) > 0)
                        errorsFile << ErrorsInterface::errorsMap[i] << "\n";
                errorsFile << "Travel errors occurred. Skipping travel.";
                errorsFile.close();
                clearData(simulator.shipPlan, simulator.shipRoute);
                get<1>(algoTuple).push_back(-1);
                numErrors += 1;
                continue;
            }
            cout << "1" << endl;
            int errorsOfAlgorithm = 0;
            errorsOfAlgorithm |= alg->readShipPlan(travel.getShipPlanPath().string());
            errorsOfAlgorithm |= alg->readShipRoute(travel.getShipRoutePath().string());
            cout << "2" << endl;
            WeightBalanceCalculator _calculator;
            alg->setWeightBalanceCalculator(_calculator);
            simulator.setWeightBalanceCalculator(_calculator);
            cout << "3" << endl;
            string algorithmErrorString;
            errorsOfAlgorithm |= simulator.startTravel(alg.get(), travel, algorithmErrorString);
            if (errorsOfAlgorithm != 0) {
                clearData(simulator.shipPlan, simulator.shipRoute); // TODO: should it be here and not before continue?
                std::error_code ec;
                fs::create_directory(output + SEPERATOR + "output" + SEPERATOR + "errors", ec);
                cout << "main:  inner for(travel):  if (errorsOfAlgorithm != 0)     errors directory = " <<
                output + SEPERATOR + "output" + SEPERATOR + "errors" << endl;
                ofstream errorsFile(simulator.errorsFileName);
                for (int i = 1; i <= (1 << 18); i *= 2)
                    if ((i & errorsOfAlgorithm) > 0)
                        errorsFile << ErrorsInterface::errorsMap[i] << "\n";
                if ((errorsOfAlgorithm & (1 << 19)) > 0) {
                    cout << "main:  inner for(travel):  if (errorsOfAlgorithm != 0)     if ((errorsOfAlgorithm & (1 << 19)) > 0)" << endl;
                    errorsFile << algorithmErrorString;
                    errorsFile.close();

                    get<1>(algoTuple).push_back(-1);
                    cout << "in tuple" + to_string(get<1>(algoTuple)[0]) << endl; // TODO: meant to print the last element?
                    numErrors += 1;
                    continue;
                }
                errorsFile.close();
            }
            cout << "4" << endl;
            cout << travel.getIndex()<< " was ended successfully for algorithm " << algorithm.first
                 << " .The number of algorithm operations: " << Simulator::algorithmActionsCounter << endl; //TODO: delete before submitting
            clearData(simulator.shipPlan, simulator.shipRoute);
            sum += Simulator::algorithmActionsCounter;
            get<1>(algoTuple).push_back(Simulator::algorithmActionsCounter);
        }
        get<2>(algoTuple) = sum;
        get<3>(algoTuple) = numErrors;
        outputVector.push_back(algoTuple);
    }

    sort(outputVector.begin(), outputVector.end(), compareAlgoTuples);
    for (tuple<string,vector<int>,int,int> algoTuple : outputVector){
        if (simulationResults.is_open()){
            cout << "simulationResults.is_open()" << endl;
            cout << get<0>(algoTuple)+"," + to_string(get<1>(algoTuple)[1])+"," + to_string(get<2>(algoTuple))+","+ to_string(get<3>(algoTuple)) <<endl;
            simulationResults << get<0>(algoTuple)+",";
            simulationResults.flush();
            for (int travelOpNum : get<1>(algoTuple)){
                simulationResults << to_string(travelOpNum)+",";
                simulationResults.flush();
            }
            if (simulationResults.is_open()) {
                cout << to_string(get<2>(algoTuple)) + "," << to_string(get<3>(algoTuple)) + "\n";
                simulationResults << to_string(get<2>(algoTuple)) + "," << to_string(get<3>(algoTuple)) + "\n";
            }else cout << "not open" << endl;
        }
    }
    simulationResults.close();

    return EXIT_SUCCESS;
}
