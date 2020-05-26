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

    vector<fs::path> algorithmsPaths;
    if (algorithmPath.empty()){
        std::error_code errorCode;
        for (const auto & entry : fs::directory_iterator(getCurrentDir(), errorCode)){
            string fileName = entry.path().string();
            if(string(".so") == (fileName.string::substr(fileName.size() - 3))){
                algorithmsPaths.push_back(fileName);
            } // TODO: handle an error of not getting so files
        }
    }

//    if (algorithmPath.empty()){
//        fs::create_directory(output + SEPERATOR + "Algorithms");
//        algorithmPath = getCurrentDir() + SEPERATOR + "Algorithms";
//        std::error_code errorCode;
//        for (const auto & entry : fs::directory_iterator(getCurrentDir(), errorCode)){
//            string fileName = entry.path().string();
//            if(string(".so") == (fileName.string::substr(fileName.size() - 3))){
//                string algorithmName = fileName.string::substr(fileName.find_last_of(SEPERATOR) + 1);
//                fs::copy(fileName, algorithmPath + SEPERATOR + algorithmName);
//            } // TODO: handle an error of not getting so files
//        }
//    }
    return algorithmsPaths;
}

int Simulator::algorithmActionsCounter;
size_t Simulator::currPortIndex;
std::map<int, std::string> ErrorsInterface::errorsMap;

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

int main(int argc, char** argv) {
    string travelPath, algorithmPath, output;
    vector<fs::path> algorithmPaths = getPaths(argc, argv, travelPath, algorithmPath, output);
    if(travelPath.empty()){
        cout << "Fatal Error: missing -travel_path argument. Exiting..." << endl;
        return EXIT_FAILURE;
    }

    auto& registrar = Registrar::getRegistrar();

    cout << output << endl;
    ofstream simulationResults(output + SEPERATOR + "output" + SEPERATOR + "simulation.results.csv"); //TODO: make sure the path is right
    simulationResults << "RESULTS,";
    for (int j =1; j<=2; ++j ){
        simulationResults << "travel "+ to_string(j) +",";
    }
    simulationResults << "SUM,";
    simulationResults << "NumErrors\n";

    vector<tuple<string,vector<int>,int,int>> outputVector;

    std::error_code errorCode;
    if(algorithmPaths.empty()){
        cout << "algorithmPaths.empty()" << endl;
        for (const auto& entry : fs::directory_iterator(algorithmPath, errorCode)){
            string fileName = entry.path().string();
            if(string(".so") == (fileName.string::substr(fileName.size() - 3))){
                string algorithmName = fileName.string::substr(fileName.find_last_of(SEPERATOR) + 1, fileName.size() - fileName.find_last_of(SEPERATOR) - 4);
                string path = algorithmPath + SEPERATOR + algorithmName + ".so";
                std::string error;
                if (!registrar.loadAlgorithmFromFile(path.c_str(), error, algorithmName)) {
                    std::cerr << error << '\n';
                    return EXIT_FAILURE;
                }
            }
        }
    } else{
        cout << "!algorithmPaths.empty()" << endl;
        for (auto& entry : algorithmPaths){
            string path = entry.string();
            string algorithmName = path.substr(path.find_last_of(SEPERATOR) + 1, path.size() - path.find_last_of(SEPERATOR) - 4);
            std::string error;
            if (!registrar.loadAlgorithmFromFile(path.c_str(), error, algorithmName)) {
                std::cerr << error << '\n';
                return EXIT_FAILURE;
            }
        }
    }

    Simulator simulator;
    simulator.initTravelsVec(travelPath);
    ErrorsInterface::populateErrorsMap();

    for(auto& algorithm : registrar.getAlgorithmMap()){
        cout << "registrar.getAlgorithmMap().size() = " << registrar.getAlgorithmMap().size() << endl;


        tuple<string,vector<int>,int,int> algoTuple;
        get<0>(algoTuple) =  algorithm.first;
        int sum = 0;
        int numErrors = 0;

        cout << "before" << endl;
        unique_ptr<AbstractAlgorithm> alg = algorithm.second();
        cout << "after" << endl;

        for(Travel& travel : simulator.getTravelsVec()){
            cout << "travel's num = " << travel.getIndex() << endl;

            simulator.errorsFileName = output + SEPERATOR + "output" + SEPERATOR + "errors" + SEPERATOR + travel.getDir().string() + "_" + algorithm.first + ".errors.txt";
            cout << simulator.errorsFileName << endl;

            int travelErrors = simulator.getInput(travel.getShipPlanPath().string(), travel.getShipRoutePath().string());
            cout << "1" << endl;

            cout << "2" << endl;
            if ((CANNOTRUNTRAVEL & travelErrors) != 0) {
                cout << "if ((CANNOTRUNTRAVEL & travelErrors) != 0) {" << endl;
                fs::create_directory(output + SEPERATOR + "output" + SEPERATOR + "errors");
                cout << output + SEPERATOR + "output" + SEPERATOR + "errors" << endl;
                ofstream errorsFile(simulator.errorsFileName);
                for (int i = 1; i <= (1 << 18); i *= 2) {
                    if ((i & travelErrors) > 0) {
                        errorsFile << ErrorsInterface::errorsMap[i] << "\n";
                    }
                }
                errorsFile << "Travel errors occurred. Skipping travel.";
                errorsFile.close();
                clearData(simulator.shipPlan, simulator.shipRoute);
                get<1>(algoTuple).push_back(-1);
                numErrors += 1;
                continue;
            }
            cout << "3" << endl;
            int errorsOfAlgorithm = 0;
            errorsOfAlgorithm |= alg->readShipPlan(travel.getShipPlanPath().string());
            errorsOfAlgorithm |= alg->readShipRoute(travel.getShipRoutePath().string());
            cout << "4" << endl;
            WeightBalanceCalculator _calculator;
            alg->setWeightBalanceCalculator(_calculator);
            simulator.setWeightBalanceCalculator(_calculator);
            cout << "5" << endl;
            string algorithmErrorString;

            errorsOfAlgorithm |= simulator.startTravel(alg.get(), travel, algorithmErrorString);

            if (errorsOfAlgorithm != 0) {
                cout << "11" << endl;
                clearData(simulator.shipPlan, simulator.shipRoute);

		std::error_code ec;
                fs::create_directory(output + SEPERATOR + "output" + SEPERATOR + "errors", ec);
                cout << output + SEPERATOR + "output" + SEPERATOR + "errors" << endl;
                ofstream errorsFile(simulator.errorsFileName);
                for (int i = 1; i <= (1 << 18); i *= 2) {
                    if ((i & errorsOfAlgorithm) > 0) {
                        errorsFile << ErrorsInterface::errorsMap[i] << "\n";
                    }
                }
                cout << "12" << endl;

                if ((errorsOfAlgorithm & (1 << 19)) > 0) {
                    cout << "13" << endl;

                    errorsFile << algorithmErrorString;
                    errorsFile.close();
                    //return -1;

                    get<1>(algoTuple).push_back(-1);
                    cout << "in tuple" + to_string(get<1>(algoTuple)[0]) << endl;
                    numErrors += 1;
                    continue;
                }

                errorsFile.close();
            }
            cout << travel.getIndex()<< " was ended successfully for algorithm " << algorithm.first
                 << " .The number of algorithm operations: " << Simulator::algorithmActionsCounter << endl;

            clearData(simulator.shipPlan, simulator.shipRoute);
            sum += simulator.algorithmActionsCounter;
            get<1>(algoTuple).push_back(simulator.algorithmActionsCounter);

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

//    if (algorithmPath == getCurrentDir() + SEPERATOR + "Algorithms"){
//        cout << algorithmPath << endl;
//        for (const auto & entry : fs::directory_iterator(algorithmPath, errorCode)){
//            string fileName = entry.path().string();
//            cout << fileName << endl;
//            fs::remove(fileName);
//        }
//        fs::is_empty(algorithmPath) ? cout << algorithmPath << " is empty" << endl : cout << algorithmPath << " isn't empty" << endl;
//        rmdir(algorithmPath.c_str());
//    }

//    if (algorithmPath == getCurrentDir() + SEPERATOR + "Algorithms"){
//        cout << algorithmPath << endl;
////        std::uintmax_t n = fs::remove_all(algorithmPath + SEPERATOR);
////        std::cout << "Deleted " << n << " files or directories\n";
//        for (const auto & entry : fs::directory_iterator(algorithmPath, errorCode)){
//            string fileName = entry.path().string();
//            cout << fileName << endl;
//            fs::remove(fileName);
//        }
//        fs::is_empty(algorithmPath) ? cout << algorithmPath << " is empty" << endl : cout << algorithmPath << " isn't empty" << endl;

//        if(!fs::is_empty(algorithmPath)){
//            std::uintmax_t n = fs::remove_all(algorithmPath);
//            std::cout << "Deleted " << n << " files or directories\n";
//        }
//        fs::remove(algorithmPath + SEPERATOR);
    //rmdir(algorithmPath.c_str());
//        system("rmdir  --ignore-fail-on-non-empty /specific/a/home/cc/students/cs/galtoubul/Ex2/Simulator/Algorithms");
//    }

    return EXIT_SUCCESS;
}
