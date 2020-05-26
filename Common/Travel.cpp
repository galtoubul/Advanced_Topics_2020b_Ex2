#include "Travel.h"

int Travel::getIndex(){
    return index;
}

fs::path& Travel::getShipPlanPath(){
    return shipPlanPath;
}

fs::path& Travel::getShipRoutePath(){
    return shipRoutePath;
}

fs::path& Travel::getDir(){
    return dir;
}
