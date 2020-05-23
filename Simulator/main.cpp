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
#include <string>
#include "Simulation.h"
#include "Registrar.h"
using std::cout;
using std::cin;
using std::endl;
using std::unique_ptr;
using std::pair;
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

void getPaths (int argc, char** argv, string& travelPath, string& algorithmPath, string& output){
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

    if (algorithmPath.empty()){
        fs::create_directory(output + SEPERATOR + "Algorithms");
        algorithmPath = getCurrentDir() + SEPERATOR + "Algorithms";
        std::error_code errorCode;
        for (const auto & entry : fs::directory_iterator(getCurrentDir(), errorCode)){
            string fileName = entry.path().string();
            if(string(".so") == (fileName.string::substr(fileName.size() - 3))){
                string algorithmName = fileName.string::substr(fileName.find_last_of(SEPERATOR) + 1);
                fs::copy(fileName, algorithmPath + SEPERATOR + algorithmName);
            } // TODO: handle an error of not getting so files
        }
    }
}

int Simulator::algorithmActionsCounter;
size_t Simulator::currPortIndex;
std::map<int, std::string> ErrorsInterface::errorsMap;

inline void clearData(ShipPlan& shipPlan, ShipRoute& shipRoute){
    const_cast<VVVC&>(shipPlan.getContainers()).clear();
    const_cast<vector<Port>&>(shipRoute.getPortsList()).clear();
}

int main(int argc, char** argv) {
    string travelPath, algorithmPath, output;
    getPaths(argc, argv, travelPath, algorithmPath, output);
    if(travelPath.empty()){
        cout << "Fatal Error: missing -travel_path argument. Exiting..." << endl;
        return EXIT_FAILURE;
    }

    auto& registrar = Registrar::getRegistrar();

    std::error_code errorCode;
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

    Simulator simulator;
    ErrorsInterface::populateErrorsMap();


//    for (auto algorithm = registrar.getAlgorithmMap().begin(); algorithm != registrar.getAlgorithmMap().end(); ++algorithm) {
    for(auto& algorithm : registrar.getAlgorithmMap()){
        cout << "registrar.getAlgorithmMap().size() = " << registrar.getAlgorithmMap().size() << endl;

        cout << "before" << endl;
        unique_ptr<AbstractAlgorithm> alg = algorithm.second();
        cout << "after" << endl;

        for (int j = 1; j <= 2; ++j) {
            cout << "travel's num = " << j << endl;
            string travelName = "Travel" + std::to_string(j);

            simulator.errorsFileName = output + SEPERATOR + "output" + SEPERATOR + "errors" + SEPERATOR + travelName + "_" + algorithm.first + ".errors.txt";
            cout << simulator.errorsFileName << endl;

            string shipPlanPath = travelName +  std::string(1, std::filesystem::path::preferred_separator) + "Ship Plan.txt";
            string shipRoutePath = travelName + std::string(1, std::filesystem::path::preferred_separator) + "Route.txt";
            simulator.getInput(shipPlanPath, shipRoutePath);

            cout << "1" << endl;
            int travelErrors = simulator.getInput(shipPlanPath, shipRoutePath);

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
                continue;
            }
            cout << "3" << endl;
            int errorsOfAlgorithm = 0;
            errorsOfAlgorithm |= alg->readShipPlan(shipPlanPath);
            errorsOfAlgorithm |= alg->readShipRoute(shipRoutePath);
            cout << "4" << endl;
            WeightBalanceCalculator _calculator;
            alg->setWeightBalanceCalculator(_calculator);
            simulator.setWeightBalanceCalculator(_calculator);
            cout << "5" << endl;
            string algorithmErrorString;

            errorsOfAlgorithm |= simulator.startTravel(alg.get(), travelName, algorithmErrorString);

            if (errorsOfAlgorithm != 0) {
                cout << "11" << endl;
                clearData(simulator.shipPlan, simulator.shipRoute);

                fs::create_directory(output + SEPERATOR + "output" + SEPERATOR + "errors");
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
                    continue;
                }

                errorsFile.close();
            }
            cout << travelName << " was ended successfully for algorithm " << algorithm.first
                 << " .The number of algorithm operations: " << Simulator::algorithmActionsCounter << endl;

            clearData(simulator.shipPlan, simulator.shipRoute);
            continue;
        }
    }



    if (algorithmPath == getCurrentDir() + SEPERATOR + "Algorithms"){
        cout << algorithmPath << endl;
        for (const auto & entry : fs::directory_iterator(algorithmPath, errorCode)){
            string fileName = entry.path().string();
            cout << fileName << endl;
            fs::remove(fileName);
        }
        rmdir(algorithmPath.c_str());
    }

    return EXIT_SUCCESS;
}
