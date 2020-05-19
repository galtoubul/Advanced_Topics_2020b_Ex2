#include <cstdlib>
#include <tuple>
#include <iostream>
#include <dlfcn.h>
#include <map>
#include <memory>
#include <string>
#include "Simulation.h"
#include "Registrar.h"
using std::cout;
using std::cin;
using std::endl;
using std::unique_ptr;
#define NOT_LEGAL -1
#define LEGAL 1

int check(int num, const std::string& mode){
    if (num < 0){
        TRAVELS_OR_ALGORITHMS_NUMBER_ERROR(mode)
        cout << "Please reenter the number you would like to test" << endl;
        return NOT_LEGAL;
    }
    return LEGAL;
}

// this is a deleter that will be operated with the "dying" of the unique_ptr
// and by that dlclose will occur
// if the handle will be null -> no deleter will occur
struct DLCloser{
    void operator()(void* dlhandle) const noexcept {
        cout << "Closing dl\n";
        dlclose(dlhandle);
    }
};

Registrar Registrar::registrar;

int main() {
    cout << "Welcome to Stowage Algorithm Simulator" << endl;

    unique_ptr<void, DLCloser> handle(dlopen("../Algorithm/_308394642_a.so",RTLD_LAZY)); // TODO: enter path from command line args
    if(!handle){
        std::cerr << "dlopen failed: " << dlerror() <<'\n';
    } else
        cout << "_308394642_a.so opened\n";

    ErrorsInterface::populateErrorsMap();

    Simulator simulator;

    auto& registrar = Registrar::getRegistrar();
    vector<std::function<std::unique_ptr<AbstractAlgorithm>()>>& factoryVec = registrar.getFactoryVec();

    for(auto& algorithmFactory : factoryVec) {
        for (int j = 1; j <= 2; ++j) {
            simulator.initSimulation(algorithmFactory(), j);
        }
    }

    return EXIT_SUCCESS;
}
