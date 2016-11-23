# gLAD
Simple autocompletion application written in C++11 based on a succinct representation of a ternary search tree (tst). Succinct representation is achieved using the [Succinct Data Structure Library](https://github.com/simongog/sdsl-lite) and the application is inspired someway by [this tutorial](https://github.com/simongog/sigir16-topkcomplete) presented at SIGIR 2016.

## Installation guide
* `git clone https://github.com/Murray1991/gLAD.git`
* `cd gLAD`
* `./install.sh`
* `make build && make main`

After the installation, in `build` directory there will be the executables for each different version. Differences between versions are explained in **report.pdf** file.

## Run the program
In order to build the index of the file `italian-cities.txt` and run the program for the first version with K=5
* `./build/main.tst1.bin data/italian-cities.txt 5`

You can build the indexes for each version using the script `build_indexes.sh`
* `./build_indexes.sh data/italian-cities.txt`

## Dictionary
In `data` folder are provided two simple dictionary files to test the application. Each file line MUST be composed by a string and a weight separated by a TAB. 

You can download a bigger dictionary file (the one used for the tests that list titles and click counts of Wikipedia pages)
* `cd data`
* `./download.sh`

## Content of this folder
* `data/`: data directory with the dictionary files and the indexes built by the application

* `include/tst/`: directory with the implementations of the different versions of the index (tst*.hpp files)

* `src/`: directory with the program main source file

* `test/`: directory with the test main source file `top_k_queries.cpp` and a subdirectory `test_cases` with some query files.

* `Makefile`: for the compilation of the applications and the tests

* `build_indexes.sh`: simple script that builds the indexes for a file

* `collect_results.sh`, `testK.sh`, `tests.sh`: scripts used for testing.

* `README.md`: this readme

* `report.pdf`: the report of the project

## Test
Compile the test programs and build the indexes (long process for big dictionaries)
* `make test`
* `./build_indexes.sh data/italian-cities.txt`

Script `tests.sh` tests each version with the queries contained in a test case file (e.g. `range_queries.txt`) and give the following output format:
* `[IndexName] K  Time(us)  ResultsFound  SizeOfQueryFile  HashValue`

Example over the test case file `range_queries.txt`, the dictionary `italian-cities.txt` and K=5:
* `./tests.sh 5 test/test_cases/range_queries.txt data/italian-cities.txt`

Script `testK.sh` tests one version with the queries contained in a test case file and different values of K. Example for the **tst4** version over the test case file `range_queries.txt` and dictionary `italian-cities.txt`
* `./testK.sh 4 test/test_cases/range_queries.txt data/italian-cities.txt`

## Results in a nutshell
Each version has been tested using a dictionary from a file over titles and click counts of Wikipedia pages. The file of ~900 MB has about ~30M of unique strings and the indexes generated are about ~320 MB. Time for the construction of an index is ~300 seconds. Top-5 queries are completed in few hundreds microseconds, further informations in `report.pdf`.

## Enviroment used
The tests have been done on a workstation providing an Intel XEON CPU E5-260 (8 core clocked at 2GHz, two private levels of cache per core and one level shared). Compiler used: gcc (GCC) 4.9.4

## Known Issues
* Slow in building the index for big files and very memory consuming in this phase.

## TODO list
* Use the approach for the version 4 in version 1, should be faster
* Investigate if the resulting indexes can be reduced in space

## License
This code is licensed under [Creative Commons Attribution-NonCommercial 4.0 International License](https://creativecommons.org/licenses/by-nc/4.0/)
