EXE = matrix
OBJS = opt.o

COMPILER = g++-4.2
COMPILER_OPTS = -fopenmp -c -g -Wall -Werror
LINKER = g++-4.2
LINKER_OPTS = -fopenmp

all : $(EXE)

$(EXE) : $(OBJS)
	$(LINKER) $(OBJS) $(LINKER_OPTS) -o $(EXE)

opt.o : opt.cpp
	$(COMPILER) $(COMPILER_OPTS) opt.cpp

clean :
	-rm -f *.o $(EXE)
