COMP = g++-9.3.0

COMMON = ../Common
INTERFACES = ../Interfaces
ALGORITHM = ../Algorithm

SHARED_OBJS = _308394642_a.so
CPP_COMP_FLAG = -std=c++2a -Wall -Wextra -Werror -pedantic-errors -DNDEBUG -fPIC -fvisibility=hidden
CPP_LINK_FLAG = -lstdc++fs -shared

all: $(SHARED_OBJS)

_308394642_a.so: _308394642_a.o
	$(COMP) $(CPP_LINK_FLAG) -o $@ $^

_308394642_a.o: 308394642_a.cpp _308394642_a.h  $(INTERFACES)/AbstractAlgorithm.h $(INTERFACES)/AlgorithmRegistration.h $(COMMON)/Parser.h $(COMMON)/ShipPlan.h $(COMMON)/Container.h $(COMMON)/ShipRoute.h $(COMMON)/Port.h $(INTERFACES)/WeightBalanceCalculator.h
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
	rm -f $(OBJS) $(SHARED_OBJS)
