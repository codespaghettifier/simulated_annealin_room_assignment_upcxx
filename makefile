# Compilator
CXX = upcxx

# Flags
CXXFLAGS = -std=c++17 -g0

# Set UPCXX_CODEMODE
export UPCXX_CODEMODE = opt

# Exec
EXECUTABLE = out

# Sources
SOURCES = main_upcxx.cpp src/CostMatrix.cpp src/RoomsAssignment.cpp src/AnnealingWorker.cpp src/AnnealingTask.cpp

# Headers
HEADERS = include/CostMatrix.hpp include/RoomsAssignment.hpp include/AnnealingWorker.hpp include/AnnealingTask.hpp include/Constants.hpp Buffer.hpp

# UPCXX_CODEMODE
export UPCXX_CODEMODE=debug

# Object
OBJECTS = $(SOURCES:.cpp=.o)
UPCXX_NODES = $(upcxx-nodes nodes)

.PHONY: all clean run

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXECUTABLE)
	chmod 777 $(OBJECTS)
	chmod 777 $(EXECUTABLE)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

run: $(EXECUTABLE)
	./run.sh