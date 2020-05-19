#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "../Interfaces/AbstractAlgorithm.h"
using std::function;
using std::unique_ptr;
using std::vector;

class Registrar{
    static Registrar registrar;

    vector<function<unique_ptr<AbstractAlgorithm>()>> factoryVec;

public:
    void registerAlgorithmFactory(const function<unique_ptr<AbstractAlgorithm>()>& algorithmFactory);

    vector<function<unique_ptr<AbstractAlgorithm>()>>& getFactoryVec();

    static Registrar& getRegistrar();
};