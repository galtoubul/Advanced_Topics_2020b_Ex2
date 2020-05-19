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
    void registerAlgorithmFactory(const function<unique_ptr<AbstractAlgorithm>()>& algorithmFactory) {
        factoryVec.push_back(algorithmFactory);
    }

    static Registrar& getRegistrar(){
        return registrar;
    }

    vector<function<unique_ptr<AbstractAlgorithm>()>>& getAlgorithmFactoryVector(){
        return factoryVec;
    }

    vector<unique_ptr<AbstractAlgorithm>> getAlgorithmVector();
};