# gLAD
Simple autocompletion application written in C++11 based on a succinct representation of a ternary search tree (tst). Succinct representation is achieved using the [Succinct Data Structure Library](https://github.com/simongog/sdsl-lite) and the application is inspired someway by [this tutorial](https://github.com/simongog/sigir16-topkcomplete) presented at SIGIR 2016.

## Installation guide
* `git clone https://github.com/Murray1991/gLAD.git`
* `cd gLAD`
* `./install.sh`
* `make build && make main`

## Run the program
After the installation, in `build` there will be the executables for each different version (see report.pdf for additional informations).
In order to run, for example, the program of the first version using the `italian-cities.txt` file and K equal to 5:
* `./build/main.tst1.bin ./data/italian-cities.txt 5`

## Content of this folder
* `data/`: data directory with two sample dictionary files to test the application and a bash script to download a bigger dictionary file
* `include/tst/`: directory with the implementations of the different versions of the index (tst*.hpp)
* `src/`: directory with the program main
* `test/`: directory containing the `top_k_queries.cpp` test program and the subfolder `test_cases/` which contains files in which each line represent a query.
* `Makefile`: for the compilation of the applications and the tests
* `build_indexes.sh`: simple script that builds the indexes for a file
* `collect_results.sh`, `testK.sh`, `tests.sh`: scripts used for testing.
* `README.md`: this readme
* `report.pdf`: the report of the project

## Test
In `data` folder are provided two simple dictionary files to test the application. 
Each file line MUST be composed by a string and a weight separated by a TAB. 

You can download a bigger dictionary file (the one used for the tests that list titles and click counts of Wikipedia pages)
* `cd data`
* `./download.sh`

Compile the test program for each version
* `make test`

Build the indexes (this can take some hundreds of seconds)
* `./build_indexes.sh ./data/italian-cities.txt`

Test each version using K=5, test case `range_queries.txt` and dictionary file `italian-cities.txt`
Output format: [name] K  Time(us)  ResultsFound  SizeOfQueryFile  HashValue
* `./tests.sh 5 ./test/test_cases/range_queries.txt ./data/italian-cities.txt`

Test *tst1* version varying with K with the same test case file and dictionary file
* `./testK.sh 1 ./test/test_cases/range_queries.txt ./data/italian-cities.txt`

## Results in a nutshell
Each version has been tested using a dictionary from a file over titles and click counts of Wikipedia pages. The file of ~900 MB (cleaned from duplicates is about ~630 MB) has about ~30M of unique strings and the indexes generated are about ~320 MB. Time for the construction of an index is ~200 seconds. Top-5 queries are completed under 1 millisecond, further informations in `report.pdf`.

## Enviroment used
The tests have been done on a workstation providing an Intel XEON CPU E5-260 (8 core clocked at 2GHz, two private levels of cache per core and one level shared). Compiler used: gcc (GCC) 4.9.4

## Known Issues
* Slow in building the index for big files and very memory consuming in this phase.

## License
This code is licensed under [Creative Commons Attribution-NonCommercial 4.0 International License](https://creativecommons.org/licenses/by-nc/4.0/)
