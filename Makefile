# C++ compiler
CXX = g++
# C++ flags
CXXFLAGS = -std=c++17 -Wall -ggdb3 -pthread

# source directory
DIR_SRC = src/
# output directory
DIR_BIN = bin/

# server sources
_SRC = main.cpp Server.cpp Logger.cpp ServerHandler.cpp
SRC = $(patsubst %,$(DIR_SRC)%,$(_SRC))

REMOVE = rm -f

TARGETS = clean server


.PHONY: clean

all: $(TARGETS)

server: $(SRC_SERVER)
	$(CXX) $(CXXFLAGS) -o $(DIR_BIN)$@ $^

clean: 
	$(REMOVE) $(DIR_BIN)server
