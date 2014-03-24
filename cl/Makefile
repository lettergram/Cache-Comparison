EXE = cl_speed

COMPILER = g++
COMPILER_OPTS = -std=c++11 -stdlib=libc++ -Wall -framework OpenCL

all : $(EXE)

$(EXE) : opencl.cpp
	$(COMPILER) $(COMPILER_OPTS) opencl.cpp -o $(EXE)

clean :
	-rm -f *.o $(EXE)
