cmake_minimum_required(VERSION 3.15)
project(Advanced_Topics_2020b_Ex2)

set(CMAKE_CXX_STANDARD 17)
set(GCC_COVERAGE_COMPILE_FLAGS "-std=c++2a -Wall -Wextra -pedantic-errors -DNDEBUG -fPIC")
set(GCC_COVERAGE_LINK_FLAGS    "-lstdc++fs -shared -ldl -export-dynamic")

add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

add_executable(Advanced_Topics_2020b_Ex2
        Algorithm/_308394642_a.h        Algorithm/_308394642_a.cpp
        Algorithm/_308394642_b.cpp      Algorithm/_308394642_b.h
        Common/Container.h              Common/Container.cpp
        Common/ISO6346.h                Common/ISO6346.cpp
        Common/Parser.h                 Common/Parser.cpp
        Common/Port.h                   Common/Port.cpp
        Common/ShipPlan.h               Common/ShipPlan.cpp
        Common/ShipRoute.h              Common/ShipRoute.cpp
        Common/Travel.h                 Common/Travel.cpp
        Common/WeightBalanceCalculator.cpp
        Interfaces/AlgorithmRegistration.h
        Interfaces/AbstractAlgorithm.h
        Interfaces/ErrorsInterface.h
        Interfaces/WeightBalanceCalculator.h
        Simulator/AlgorithmRegistration.cpp
        Simulator/main.cpp
        Simulator/Registrar.h           Simulator/Registrar.cpp
        Simulator/Simulation.h          Simulator/Simulation.cpp)
