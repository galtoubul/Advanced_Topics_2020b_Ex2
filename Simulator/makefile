COMP = g++-9.3.0

COMMON = ../Common
INTERFACES = ../Interfaces
ALGORITHM = ../Algorithm
OBJS = main.o Simulation.o $(COMMON)/Parser.o $(COMMON)/ShipPlan.o $(COMMON)/Container.o \
$(COMMON)/ShipRoute.o $(COMMON)/Port.o $(COMMON)/WeightBalanceCalculator.o AlgorithmRegistration.o 

EXEC = simulator
CPP_COMP_FLAG = -std=c++2a -Wall -Wextra -Werror -pedantic-errors -DNDEBUG
CPP_LINK_FLAG = -lstdc++fs -ldl -export-dynamic

$(EXEC): $(OBJS)
	$(COMP) $(OBJS) $(CPP_LINK_FLAG) -o $@

main.o: main.cpp Simulation.h $(INTERFACES)/ErrorsInterface.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
Registrar.o: Registrar.cpp Registrar.h
    $(COMP) $(CPP_COMP_FLAG) -c $*.cpp
Simulation.o: Simulation.h Simulation.cpp $(COMMON)/Parser.h $(INTERFACES)/AbstractAlgorithm.h Registrar.h $(INTERFACES)/WeightBalanceCalculator.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
$(COMMON)/Parser.o: $(COMMON)/Parser.h $(COMMON)/Parser.cpp $(COMMON)/ShipPlan.h $(COMMON)/ShipRoute.h $(INTERFACES)/ErrorsInterface.h
	$(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp
$(COMMON)/ShipPlan.o: $(COMMON)/ShipPlan.h $(COMMON)/ShipPlan.cpp $(COMMON)/Container.h
	$(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp
$(COMMON)/Container.o: $(COMMON)/Container.h $(COMMON)/Container.cpp
	$(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp
$(COMMON)/ISO6346.o: $(COMMON)/ISO6346.cpp $(COMMON)/ISO6346.h
    $(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp
$(COMMON)/ShipRoute.o: $(COMMON)/ShipRoute.h $(COMMON)/ShipRoute.cpp $(COMMON)/Port.h
	$(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp
$(COMMON)/Port.o: $(COMMON)/Port.h $(COMMON)/Port.cpp
	$(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp
$(COMMON)/WeightBalanceCalculator.o: $(INTERFACES)/WeightBalanceCalculator.h $(COMMON)/WeightBalanceCalculator.cpp
	$(COMP) $(CPP_COMP_FLAG) -o $@ -c $*.cpp
AlgorithmRegistration.o: $(INTERFACES)/AlgorithmRegistration.h AlgorithmRegistration.cpp Registrar.h $(INTERFACES)/AbstractAlgorithm.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp

clean:
	rm -f $(OBJS) $(EXEC)
