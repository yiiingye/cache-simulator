CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

SERVER_SRC = simulator.cpp api.cpp
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)

TEST_SRC = simulator.cpp main.cpp
TEST_OBJ = $(TEST_SRC:.cpp=.o)

SERVER_TARGET = server
TEST_TARGET = test_simulator

all: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJ)

test: $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJ)

clean:
	rm -f $(SERVER_OBJ) $(TEST_OBJ) $(SERVER_TARGET) $(TEST_TARGET)

.PHONY: all clean test
