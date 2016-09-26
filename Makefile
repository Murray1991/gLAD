CXX 			= g++
CXX_FLAGS 		= -std=c++11 -O3
CXX_FLAGS_NO_OPT 	= -std=c++11 -O0

INCLUDES		= -I sdsl/include/ -I include/tst/ -L sdsl/lib -lsdsl -ldivsufsort -ldivsufsort64 
DEBUG			= -g -DDEBUG
build			= build

.PHONY: clean

build:
	mkdir -p $(build)

main:	build main.tst1.bin main.tst2.bin

test:	top_k_queries.tst1.bin top_k_queries.tst2.bin

top_k_queries.tst1.bin: test/top_k_queries.cpp
	$(CXX) $^ $(CXX_FLAGS) -DTST1="tst1" -o $(build)/$@ $(DEBUG) $(INCLUDES)

top_k_queries.tst2.bin: test/top_k_queries.cpp
	$(CXX) $^ $(CXX_FLAGS) -DTST2="tst2" -o $(build)/$@ $(DEBUG) $(INCLUDES)

main.tst1.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS) -DDEBUG -DTST1="tst1" -o $(build)/$@ $(INCLUDES)

main.tst2.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS) -DDEBUG -DTST2="tst2" -o $(build)/$@ $(INCLUDES) 

maind.tst1.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS_NO_OPT) -DDEBUG -DTST1="tst1" -o $(build)/$@ $(INCLUDES) $(DEBUG)

maind.tst2.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS_NO_OPT) -DDEBUG -DTST2="tst2" -o $(build)/$@ $(INCLUDES) $(DEBUG)

clean: 
	-rm build/*

