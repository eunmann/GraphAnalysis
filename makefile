# Compiler
CXX = g++

# Warnings
WARN = -Wall

# Link
LINK = -lpmem -fopenmp

# Include Directory
IDIR = -Iinclude

# Optimizing Flags
OPT = -O3 -march=native -flto
debug: OPT = -O0 -march=native
assembly: OPT = -O3 -march=native

# Flags
CXXFLAGS = $(WARN) -std=c++14
debug: CXXFLAGS += -g

# Source
srcCPP = $(wildcard ./src/*.cpp) $(wildcard ./src/*/*.cpp)
_objCPP = $(srcCPP:.cpp=.o)
_asmCPP = $(srcCPP:.cpp=.s)
objCPP = $(subst src,obj,$(_objCPP))
asmCPP = $(subst src,asm,$(_asmCPP))

obj = $(objCPP)
asm = $(asmCPP)

out_name=pmem_benchmark

# Compile
main: $(obj)
	$(CXX) $(CXXFLAGS) $(OPT) $(IDIR) -o $(out_name) $^ $(LINK)

debug: $(obj) 
	$(CXX) $(CXXFLAGS) $(OPT) $(IDIR) -o $(out_name) $^ $(LINK)

assembly: $(asm)

obj/%.o: src/%.cpp | dir_make
	$(CXX) $(CXXFLAGS) $(OPT) $(IDIR) -c -o $@ $^ $(LINK)

asm/%.s: src/%.cpp | dir_make
	$(CXX) $(CXXFLAGS) $(OPT) $(IDIR) -S -o $@ $^ $(LINK)

dir_make:
	@mkdir -p obj asm obj/PMEM asm/PMEM obj/GNUPlot asm/GNUPlot tmp output

.PHONY: clean

# Clean
clean:
	rm -rf obj/*.o asm/*.s obj/PMEM/*.o asm/PMEM/*.s obj/GNUPlot/*.o asm/GNUPlot/*.s $(out_name) tmp/* output/debug/*
