CXX 			= g++
CXX_FLAGS 		= -std=c++11 -O3 -march=native
CXX_FLAGS_NO_OPT 	= -std=c++11 -O0

INCLUDES		= -I sdsl/include/ -I include/tst/ -L sdsl/lib/
DEBUG			= -gdwarf-2 -g
build			= build

.PHONY: clean

build:
	mkdir -p $(build)

main:	build main.tst1.bin main.tst2.bin

test:	top_k_queries.tst1.bin top_k_queries.tst2.bin

top_k_queries.tst1.bin: test/top_k_queries.cpp
	$(CXX) $^ $(CXX_FLAGS) $(INCLUDES) $(DEBUG) -DTST1="tst1" -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

top_k_queries.tst2.bin: test/top_k_queries.cpp
	$(CXX) $^ $(CXX_FLAGS) $(INCLUDES) $(DEBUG) -DTST2="tst2" -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

main.tst1.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS) $(INCLUDES) -DDEBUG -DTST1="tst1" -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

main.tst2.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS) $(INCLUDES) -DDEBUG -DTST2="tst2" -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

maind.tst1.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS_NO_OPT) $(DEBUG) -DDEBUG -DTST1="tst1" $(INCLUDES) -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

maind.tst2.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS_NO_OPT) $(DEBUG) -DDEBUG -DTST2="tst2" $(INCLUDES) -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

clean: 
	-rm build/*

