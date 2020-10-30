# Compiler
CXX = g++

# Link
ifeq ($(OS),Windows_NT)
	LINK = -L"C:\workdir\TEMP\vcpkg\installed\x64-windows\lib\" -llibpmemobj
else
	LINK = -lpmemobj -lpmem
endif

# Include Directory
IDIR = -Iinclude

ifeq ($(OS),Windows_NT)
	IDIR += -I"C:\workdir\TEMP\vcpkg\installed\x64-windows\include"
endif

# Optimizing Flags
OPT = -O3 -march=native
debug: OPT = -O0

# Flags
CXXFLAGS = -Wall -std=c++17
debug: CXXFLAGS += -g

# Source
srcCPP = $(wildcard ./src/*.cpp)
_objCPP = $(srcCPP:.cpp=.o)
_asmCPP = $(srcCPP:.cpp=.s)
objCPP = $(subst src,obj,$(_objCPP))
asmCPP = $(subst src,asm,$(_asmCPP))

obj = $(objCPP)
asm = $(asmCPP)

# Clean
pm_files = test_memory_simple_struct test_memory_simple_struct_2

ifeq ($(OS),Windows_NT)
	CLN = del /f /q $(subst /,\,$(obj)) $(subst /,\,$(asm)) main.exe debug.exe $(pm_files)
	CLN_PM = del /f /q $(pm_files)
else
	CLN = rm -rf $(obj) $(asm) main debug $(pm_files)
	CLN_PM = rm -rf $(pm_files)
endif

# Compile
main: $(obj)
	$(CXX) $(CXXFLAGS) $(OPT) $(IDIR) -o $@ $^ $(LINK)

debug: $(obj) 
	$(CXX) $(CXXFLAGS) $(OPT) $(IDIR) -o $@ $^ $(LINK)

assembly: $(asm)

obj/%.o: src/%.cpp | dir_make
	$(CXX) $(CXXFLAGS) $(OPT) $(IDIR) -c -o $@ $^ $(LINK)

asm/%.s: src/%.cpp | dir_make
	$(CXX) $(CXXFLAGS) $(OPT) $(IDIR) -S -o $@ $^ $(LINK)

dir_make:
	@mkdir -p asm obj tmp

.PHONY: clean clean_pm

# Clean
clean:
	$(CLN)

clean_pm:
	$(CLN_PM)
