COMP = g++-9.3.0

COMMON = ../Common
INTERFACES = ../Interfaces
ALGORITHM = ../Algorithm
OBJS = AlgorithmRegistration.o main.o Registrar.o Simulation.o $(COMMON)/Container.o $(COMMON)/Parser.o \
        $(COMMON)/Port.o $(COMMON)/ShipPlan.o $(COMMON)/ShipRoute.o $(COMMON)/WeightBalanceCalculator.o

EXEC = simulator
CPP_COMP_FLAG = -std=c++2a -Wall -Wextra -Werror -pedantic-errors -DNDEBUG
CPP_LINK_FLAG = -lstdc++fs -ldl -export-dynamic

$(EXEC): $(OBJS)
	$(COMP) $(OBJS) $(CPP_LINK_FLAG) -o $@

AlgorithmRegistration.o: AlgorithmRegistration.cpp $(INTERFACES)/AlgorithmRegistration.h Registrar.h $(INTERFACES)/AbstractAlgorithm.h
    $(COMP) $(CPP_COMP_FLAG) -c $*.cpp
main.o: main.cpp Simulation.h $(INTERFACES)/ErrorsInterface.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
Registrar.o: Registrar.cpp Registrar.h
    $(COMP) $(CPP_COMP_FLAG) -c $*.cpp
Simulation.o: Simulation.cpp Simulation.h $(COMMON)/Parser.h $(INTERFACES)/AbstractAlgorithm.h Registrar.h $(INTERFACES)/WeightBalanceCalculator.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
$(COMMON)/Container.o: $(COMMON)/Container.cpp $(COMMON)/Container.h
    $(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp
$(COMMON)/Parser.o: $(COMMON)/Parser.cpp $(COMMON)/Parser.h $(COMMON)/ShipPlan.h $(COMMON)/ShipRoute.h $(INTERFACES)/ErrorsInterface.h
	$(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp
$(COMMON)/Port.o: $(COMMON)/Port.cpp $(COMMON)/Port.h
    $(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp
$(COMMON)/ShipPlan.o: $(COMMON)/ShipPlan.cpp $(COMMON)/ShipPlan.h $(COMMON)/Container.h
	$(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp
$(COMMON)/ShipRoute.o: $(COMMON)/ShipRoute.cpp $(COMMON)/ShipRoute.h $(COMMON)/Port.h
	$(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp
$(COMMON)/WeightBalanceCalculator.o: $(COMMON)/WeightBalanceCalculator.cpp $(INTERFACES)/WeightBalanceCalculator.h
	$(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp

clean:
	rm -f $(OBJS) $(EXEC)
