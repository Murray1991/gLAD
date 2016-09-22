# gLAD
Simple autocompletion application based on a succinct representation of a ternary search tree (tst).
This application is inspired by this [tutorial](https://github.com/simongog/sigir16-topkcomplete) presented at SIGIR 2016.

## Installation guide
* `https://github.com/Murray1991/gLAD.git`
* `cd gLAD`
* `./install.sh`
* `make build && make main.bin`

## Test
In `data` folder are provided two simple dictionary files to test the application.
Each file line MUST be composed by a string and a weight separated by a TAB.

To run the program:
* `./build/main.bin ./data/italian-cities.txt` 
