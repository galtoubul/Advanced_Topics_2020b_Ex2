COMP = g++-9.3.0

COMMON = ../Common
INTERFACES = ../Interfaces
ALGORITHM = ../Algorithm

SHARED_OBJS = _308394642_a.so
CPP_COMP_FLAG = -std=c++2a -Wall -Wextra -Werror -pedantic-errors -DNDEBUG -fPIC
CPP_LINK_FLAG = -lstdc++fs -shared

all: $(SHARED_OBJS)

_308394642_a.so: _308394642_a.o
	$(COMP) $(CPP_LINK_FLAG) -o $@ $^

_308394642_a.o: _308394642_a.h _308394642_a.cpp $(INTERFACES)/AbstractAlgorithm.h $(INTERFACES)/AlgorithmRegistration.h $(COMMON)/Parser.h $(COMMON)/ShipPlan.h $(COMMON)/Container.h $(COMMON)/ShipRoute.h $(COMMON)/Port.h $(INTERFACES)/WeightBalanceCalculator.h
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

clean:
	rm -f $(OBJS) $(SHARED_OBJS)
