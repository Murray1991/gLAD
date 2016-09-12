
#include <iostream>
#include <algorithm>
#include <chrono>
#include <unistd.h>
#include <ctime>
#include "tst.hpp"

using namespace std;
using namespace chrono;

typedef std::__cxx11::basic_string<uint8_t> ustring;

std::ifstream::pos_type filesize(const string& file) {
    std::ifstream in(file, std::ios::binary | std::ios::ate);
    return in.tellg(); 
}

int main(int argc, char *argv[]) {
    
    if ( argc < 2 ) {
        cerr << "Pirla, serve il nome del file\n";
        return 1;
    }
    srand(time(0));
    const string file = std::string(argv[1]);
    const string index_file = file+".sdsl";
    
    cout << "build index for " << file << \
            "\nsize (MB): " << ((double) filesize(file))/1000000 << endl;
            
    auto t1 = std::chrono::high_resolution_clock::now();
    glad::tst<> t(file);
    
    /*if ( ! load_from_file (t, index_file ) ) {
        cout << "not existing tst-sdsl file..." << endl;
        glad::tst<> tst2(file);
        cout << "stored!\n";
        load_from_file(t, index_file);
    }*/
    cout << "loaded!\n";
    auto t2 = std::chrono::high_resolution_clock::now();
    
    cout << "init: " << (duration_cast<duration<double>>(t2-t1)).count() << "sec\n" << \
            "nodes: " << t.get_nodes() << "\n" << \
            "space (MB): " << t.get_size()/1000000 << endl;
    
    //query_from_input(t);
            
    return 0;
}
