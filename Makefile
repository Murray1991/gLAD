CXX 			= g++
CXX_FLAGS 		= -std=c++11 -O3 -march=native
CXX_FLAGS_NO_OPT 	= -std=c++11 -O0

INCLUDES		= -I sdsl/include/ -I include/tst/ -L sdsl/lib/
DEBUG			= -g -pg
build			= build

.PHONY: clean

build:
	mkdir -p $(build)

main:	build main.bin maind.bin

main.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS) $(INCLUDES) -DDEBUG -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

maind.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS_NO_OPT) $(DEBUG) -DDEBUG $(INCLUDES) -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

clean: 
	-rm build/*

