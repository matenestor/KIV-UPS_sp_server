# C++ compiler
CXX = g++
# C++ flags
CXXFLAGS = -std=c++17 -Wall -O -pthread

# name of executable
BIN = hnefsrv

# logging directory
DIR_LOG = log/

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


all: mkdirs $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(DIR_BIN)$@ $^

$(DIR_OBJ)%.o: $(DIR_SRC)% $(HDR)
	$(CXX) $(CXXFLAGS) $(IDEPS) -c $< -o $@

.PHONY: all


mkdirs:
	mkdir -p $(DIR_LOG)
	mkdir -p $(DIR_BIN)
	mkdir -p $(DIR_OBJ)
	mkdir -p $(patsubst %,$(DIR_OBJ)%,$(DIR_SUBD))
	
.PHONY: mkdirs


clean:
	$(RM) $(DIR_OBJ)

.PHONY: clean
