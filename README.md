# gLAD
Simple autocompletion application written in C++11 based on a succinct representation of a ternary search tree (tst). Succinct representation is achieved using the [Succinct Data Structure Library](https://github.com/simongog/sdsl-lite) and the application is inspired someway by [this tutorial](https://github.com/simongog/sigir16-topkcomplete) presented at SIGIR 2016.

## Installation guide
* `git clone https://github.com/Murray1991/gLAD.git`
* `cd gLAD`
* `./install.sh`
* `make build && make main.bin`

## Test
In `data` folder are provided two simple dictionary files to test the application.
Each file line MUST be composed by a string and a weight separated by a TAB.

To run the program:
* `./build/main.bin ./data/italian-cities.txt`

## Results in a nutshell
I've tested the application generating a succinct tst index built over titles and click counts of Wikipedia pages. The file of ~900 MB (cleaned from duplicates is about ~630 MB) has about ~30M of unique strings and the index generated is about ~310 MB. Top-k queries are completed with a time varying typically from 0.8 to 1.6 milliseconds.

## Enviroment used
The tests have been done using commodity hardware mounting a Intel(R) Core(TM)i5-3317U CPU clocked at 1.70GHz and a 4GB DDR3 memory. 

## Known Issues
* Very slow in building the index for a big file and very memory consuming in this phase. 

