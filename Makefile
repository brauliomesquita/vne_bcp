SYSTEM     = x86-64_sles10_4.1
LIBFORMAT  = static_pic

#------------------------------------------------------------
#
# When you adapt this makefile to compile your CPLEX programs
# please copy this makefile and set CPLEXDIR and CONCERTDIR to
# the directories where CPLEX and CONCERT are installed.
#
#------------------------------------------------------------

# Lapo
# make
#CPLEXDIR      = /home/lapo/cplex/cplex126/cplex
#CONCERTDIR    = /home/lapo/cplex/cplex126/concert

# Notebook
# make LOCAL=h
#ifeq ($(LOCAL),h)
CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio125/cplex
CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Studio125/concert
#endif

# ---------------------------------------------------------------------
# Compiler selection
# ---------------------------------------------------------------------

CCC = g++

# ---------------------------------------------------------------------
# Compiler options
# ---------------------------------------------------------------------

CCOPT = -m64 -O3 -fPIC -fno-strict-aliasing -fexceptions -g -DIL_STD -std=c++11 -w

# ---------------------------------------------------------------------
# Link options and libraries
# ---------------------------------------------------------------------

CPLEXBINDIR   = $(CPLEXDIR)/bin/$(BINDIST)
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

CCLNDIRS  = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR)
CCLNFLAGS = -lconcert -lilocplex -lcplex -lm -pthread

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include


CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR)

# ------------------------------------------------------------
# Final binary
BIN = ../Solver

# Put all auto generated stuff to this build dir.
BUILD_DIR = ./build
SRC_DIR = ./src

# List of all .cpp source files.
CPPS = $(wildcard $(SRC_DIR)/*.cpp)

# All .o files go to build dir.
#OBJ = $(CPPS:%.cpp=$(BUILD_DIR)/%.o)
OBJ = $(addprefix $(BUILD_DIR)/, $(notdir $(CPPS:.cpp=.o)))


# Gcc/Clang will create these .d files containing dependencies.
DEP=$(OBJ:%.o=%.d)

# Default target named after the binary.
$(BIN) : $(BUILD_DIR)/$(BIN)

# Actual target of the binary - depends on all .o files.
$(BUILD_DIR)/$(BIN) : $(OBJ)
        # Create build directories - same structure as sources.
	mkdir -p $(@D)
        # Just link all the object files.
	$(CCC) $(CCFLAGS) $(CCLNDIRS) $^ -o $@ $(CCLNFLAGS)

# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/%.o : $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
        # The -MMD flags additionaly creates a .d file with
        # the same name as the .o file.
	$(CCC) $(CCFLAGS) $(CCLNDIRS) -MMD -c $< -o $@ $(CCLNFLAGS)

.PHONY : clean
clean :
        # This should remove all generated files.
	-rm $(BUILD_DIR)/$(BIN) $(OBJ) $(DEP)
# ------------------------------------------------------------
