#include "Registrar.h"

void Registrar::registerAlgorithmFactory(const function<unique_ptr<AbstractAlgorithm>()>& algorithmFactory) {
    factoryVec.push_back(algorithmFactory);
}

vector<function<unique_ptr<AbstractAlgorithm>()>>& Registrar::getFactoryVec(){
    return factoryVec;
}

Registrar& Registrar::getRegistrar() {
    return registrar;
}

