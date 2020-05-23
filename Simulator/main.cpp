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
#include "../Common/Parser.h"
using std::cout;
using std::cin;
using std::endl;
using std::unique_ptr;
using std::pair;
#define NOT_LEGAL -1
#define LEGAL 1
namespace fs = std::filesystem;
#define SEPERATOR string(1, fs::path::preferred_separator)

// this is a deleter that will be operated with the "dying" of the unique_ptr
// and by that dlclose will occur
// if the handle will be null -> no deleter will occur
struct DLCloser{
    void operator()(void* dlhandle) const noexcept {
        cout << "Closing dl\n";
        dlclose(dlhandle);
    }
};

string getCurrentDir() {
    char buff[FILENAME_MAX]; //create string buffer to hold path
    GetCurrentDir(buff, FILENAME_MAX);
    string currentWorkingDir(buff);
    return currentWorkingDir;
}

void getPaths (int argc, char** argv, string& travelPath, string& algorithmPath, string& output){
    std::vector<std::string> args(argv, argv + argc);
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
        algorithmPath = output + SEPERATOR + "Algorithms";
        for (const auto & entry : fs::directory_iterator(output)){
            string fileName = entry.path().string();
            if(string(".so") == (fileName.string::substr(fileName.size() - 3))){
                string algorithmName = fileName.string::substr(fileName.find_last_of(SEPERATOR) + 1);
                algorithmPath += SEPERATOR += algorithmName;
                fs::copy(fileName, algorithmPath);
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

    unique_ptr<void, DLCloser> handle(dlopen("../Algorithm/_308394642_a.so",RTLD_LAZY)); // TODO: enter path from command line args
    if(!handle){
        std::cerr << "dlopen failed: " << dlerror() <<'\n';
    } else
        cout << "_308394642_a.so opened\n";

    ErrorsInterface::populateErrorsMap();

    Simulator simulator;

    string algName = "alg1";
    vector<function<unique_ptr<AbstractAlgorithm>()>> algFactoryVec = Registrar::getRegistrar().getAlgorithmFactoryVector();
    //vector<pair<string, unique_ptr<AbstractAlgorithm>>> algVecWithNames;
    //algVecWithNames.emplace_back(std::make_pair(algName, algVec[0]));
    cout << "after getting algorithmVec. Its size is: " << algFactoryVec.size() << endl;
    for (int j = 1; j <= 2; ++j) {
        for(auto& algFactory : algFactoryVec) {
            cout << "travel's num = " << j << endl;
            simulator.initSimulation(algFactory, j);
        }
    }

    return EXIT_SUCCESS;
}
