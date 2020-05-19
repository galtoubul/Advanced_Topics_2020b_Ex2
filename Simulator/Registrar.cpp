#include "Registrar.h"

Registrar Registrar::registrar;

vector<unique_ptr<AbstractAlgorithm>> Registrar::getAlgorithmVector(){
    vector <unique_ptr<AbstractAlgorithm>> algorithmVec;
    for(auto& algorithmFactory : factoryVec)
        algorithmVec.push_back(algorithmFactory());

    return algorithmVec;
}


