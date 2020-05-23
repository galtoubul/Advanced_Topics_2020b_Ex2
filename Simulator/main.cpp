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

    for (int j = 1; j <= 2; ++j) {
        for (auto algorithm = registrar.getAlgorithmMap().begin(); algorithm != registrar.getAlgorithmMap().end(); ++algorithm) {
            cout << "travel's num = " << j << endl;
            simulator.initSimulation(algorithm->second, algorithm->first, j);
        }
    }

    if (algorithmPath == getCurrentDir() + SEPERATOR + "Algorithms")
        fs::remove(algorithmPath);

    return EXIT_SUCCESS;
}
