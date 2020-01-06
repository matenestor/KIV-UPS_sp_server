# C++ compiler
CXX = g++
# C++ flags
CXXFLAGS = -std=c++17 -Wall -ggdb3

# source directory
DIR_SRC = src/
# output directory
DIR_BIN = bin/

# server sources
_SRC_SERVER = main.cpp Server.cpp Logger.cpp ServerHandler.cpp
SRC_SERVER = $(patsubst %,$(DIR_SRC)%,$(_SRC_SERVER))
# client sources
_SRC_CLIENT = main.cpp Client.cpp Logger.cpp
SRC_CLIENT = $(patsubst %,$(DIR_SRC)%,$(_SRC_CLIENT))

REMOVE = rm -f

TARGETS = clean server client


.PHONY: clean

all: $(TARGETS)

server: $(SRC_SERVER)
	$(CXX) $(CXXFLAGS) -o $(DIR_BIN)$@ $^

client:	$(SRC_CLIENT)
	$(CXX) $(CXXFLAGS) -o $(DIR_BIN)$@ $^

clean: 
	$(REMOVE) $(DIR_BIN)server
	$(REMOVE) $(DIR_BIN)client
