
CC			= gcc

CXX 			= g++
CXX_FLAGS 		= -std=c++11 -O3 -march=native
CXX_FLAGS_NO_OPT 	= -std=c++11 -O0

INCLUDES		= -I sdsl/include/ -L sdsl/lib/
DEBUG			= -g -pg
build			= build

.PHONY: clean index index1-main index4ci-main old

build:
	mkdir -p $(build)

oldc:	build oldc.bin

oldc.bin: oldc/tstdemo.c
	$(CC) $^ $(CC_FLAGS) -o $(build)/$@

old:	build old.bin

old.bin: old/main.cpp old/tst.cpp
	$(CXX) $^ $(CXX_FLAGS) -Wnonnull-compare $(DEBUG) $(INCLUDES) -L old/ -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

main:	build main.bin maind.bin

main.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS) $(DEBUG) $(INCLUDES) -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

maind.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS_NO_OPT) $(DEBUG) -DDEBUG $(INCLUDES) -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

maino.bin: src/main.cpp
	$(CXX) $^ $(CXX_FLAGS_NO_OPT) $(DEBUG) $(INCLUDES) -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

index:	build index1-main

index1-main: src/index.cpp
	$(CXX) $^ $(CXX_FLAGS) $(INCLUDES) -DINDEX_TYPE=index1  -DINDEX_NAME='"index1"' -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

index3-main: src/index.cpp
	$(CXX) $^ $(CXX_FLAGS) $(INCLUDES) -DINDEX_TYPE=index3  -DINDEX_NAME='"index3"' -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

index4ci-main: src/index.cpp
	$(CXX) $^ $(CXX_FLAGS) $(INCLUDES) -DINDEX_TYPE=index4ci  -DINDEX_NAME='"index4ci"' -o $(build)/$@ -lsdsl -ldivsufsort -ldivsufsort64

clean: 
	-rm build/*
