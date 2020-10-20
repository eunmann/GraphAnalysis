# Compiler
CXX = g++

# Link
LINK = -L"C:\workdir\TEMP\vcpkg\installed\x64-windows\lib\" -llibpmemobj

# Include Directory
IDIR = -Iinclude -I"C:\workdir\TEMP\vcpkg\installed\x64-windows\include"

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

# Compile
main: $(obj)
	$(CXX) $(CXXFLAGS) $(OPT) $(IDIR) -o $@ $^ $(LINK)

debug: $(obj)
	$(CXX) $(CXXFLAGS) $(OPT) $(IDIR) -o $@ $^ $(LINK)

assembly: $(asm)

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(OPT) $(IDIR) -c -o $@ $^ $(LINK)

asm/%.s: src/%.cpp
	$(CXX) $(CXXFLAGS) $(OPT) $(IDIR) -S -o $@ $^ $(LINK)

.PHONY: clean

# Clean
clean:
	del /f /q $(subst /,\,$(obj)) $(subst /,\,$(asm)) main.exe debug.exe
