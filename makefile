CC = g++
CFLAGS = -Wall -std=c++17 -O3
OFOLDER = Objects
SFOLDER = Sources

OBJS = shortestWidestPathMetric.o widestShortestPathMetric.o \
	   shortestPathMetric.o eigrpMetric.o \
	   bgpAttribute.o algebra.o \
	   nodeOneDest.o \
	   event.o eventQueueNode.o \
	   eigrpSynchronous.o bgpSynchronous.o \
	   eigrpOneDest.o bgpOneDest.o bgpQuasiSync.o \
	   vectoringProtocol.o \
	   eigrpOneDestFC.o \
	   stats.o utilities.o \
	   syncSimulation.o \
	   asyncSimulation.o

main: main.cpp $(OBJS:%.o=$(OFOLDER)/%.o)
	@echo "making executable $<... [$@]"
	$(CC) $(CFLAGS) $(OBJS:%.o=$(OFOLDER)/%.o) main.cpp -o $@

main2: main.cpp $(OBJS:%.o=$(OFOLDER)/%.o)
	@echo "making executable $<... [$@]"
	$(CC) $(CFLAGS) $(OBJS:%.o=$(OFOLDER)/%.o) main.cpp -o $@

# Generic rule for objects
# $< := input.cpp; $@ := output.cpp
$(OFOLDER)/%.o: $(SFOLDER)/%.cpp
	@echo "compiling $<... [$@]"
	$(CC) $(CFLAGS) -c $< -o $@

%.o: $(SFOLDER)/%.cpp
	@echo "compiling $<... [$@]"
	$(CC) $(CFLAGS) -c $< -o $(OFOLDER)/$@

clean:
	rm -fv $(OFOLDER)/*.o *.o core a.out pt2 *~
