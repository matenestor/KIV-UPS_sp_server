# C++ compiler
CXX = g++
# C++ flags
CXXFLAGS = -std=c++17 -Wall -O -pthread

# name of executable
BIN = hnefserver

# source directory
DIR_SRC = src/
# object directory
DIR_OBJ = obj/
# output directory
DIR_BIN = bin/

# sub-directories
DIR_SUBD = system/ network/ game/

# include location of dependent header files
IDEPS = $(foreach SUBD,$(DIR_SUBD),-I$(DIR_SRC)$(SUBD))

# each .hpp file in whole hiearchy
HDR = $(foreach SUBD,$(DIR_SUBD),$(wildcard $(DIR_SRC)$(SUBD)*.hpp))
# each .cpp file in whole hiearchy
SRC = $(foreach SUBD,$(DIR_SUBD),$(wildcard $(DIR_SRC)$(SUBD)*.cpp))
# all object files
OBJ = $(patsubst $(DIR_SRC)%,$(DIR_OBJ)%.o,$(SRC))

RM = rm -rf


all: mkdirobj $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(DIR_BIN)$@ $^

$(DIR_OBJ)%.o: $(DIR_SRC)% $(HDR)
	$(CXX) $(CXXFLAGS) $(IDEPS) -c $< -o $@

.PHONY: all


mkdirobj:
	mkdir -p $(DIR_OBJ)
	mkdir -p $(patsubst %,$(DIR_OBJ)%,$(DIR_SUBD))
	
.PHONY: mkdirobj


clean:
	$(RM) $(DIR_OBJ)

.PHONY: clean
