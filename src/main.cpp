
#include <iostream>
#include <algorithm>
#include <chrono>
#include <unistd.h>
#include <ctime>
#include <cstdlib>

#ifdef TST1
    #include "tst1.hpp"
    const char * type = "1";
#endif

#ifdef TST2
    #include "tst2.hpp"
    const char * type = "2";
#endif

#ifdef TST3
    #include "tst3.hpp"
    const char * type = "3";
#endif
    
#ifdef TST4
    #include "tst4.hpp"
    const char * type = "4";
#endif
    
using namespace std;
using namespace chrono;

std::ifstream::pos_type filesize(const string& file) {
    std::ifstream in(file, std::ios::binary | std::ios::ate);
    return in.tellg(); 
}

void query_from_input(glad::tst<>& t, size_t k) {
    std::cout << "Please enter queries line by line." << std::endl;
    std::cout << "Pressing Crtl-D will quit the program." << std::endl;
    std::string prefix;
    while ( std::getline(std::cin, prefix) ) {
        auto query_start = chrono::high_resolution_clock::now(); 
        auto result_list = t.top_k(prefix, k);
        auto query_time  = chrono::high_resolution_clock::now() - query_start;
        auto query_us    = chrono::duration_cast<chrono::microseconds>(query_time).count();
        cout << "-- top results:" << endl;
        for (size_t i=0; i<result_list.size(); ++i) {
            std::cout << result_list[i].first << "  " << result_list[i].second << std::endl;
        }
        cout << "-- (" << std::setprecision(3) << query_us / 1000.0;
        cout << " ms)" << endl;
    }
}

int main(int argc, char *argv[]) {
    
    if ( argc < 2 ) {
        std::cerr << "Error: Insert filename [and K]\n";
        exit(EXIT_FAILURE);
    }
    
    size_t k = 5;
    if ( argc == 3 ) {
        k = atoi(argv[2]);
    }
    
    const string file = std::string(argv[1]);
    const string index_file = file+"."+type+".sdsl";
    std::cout << "Get index for " << file ;
    std::cout << " of size : " << ((double) filesize(file))/1000000 << " MB" << std::endl;
            
    auto t1 = std::chrono::high_resolution_clock::now();
    glad::tst<> t;
    if ( ! load_from_file (t, index_file ) ) {
        std::cout << "Index not found. Start building... " << std::endl;
        glad::tst<> t1(file);
        store_to_file(t1, index_file);
        load_from_file (t, index_file);
        std::cout << "Index saved in " << index_file << std::endl;
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Init time: " << (duration_cast<duration<double>>(t2-t1)).count() << "sec\n" << \
            "Number of nodes: " << t.get_nodes() << std::endl << \
            "Space (MB): " << t.get_size()/1000000 << std::endl << std::endl;
    
    query_from_input(t, k);
    return 0;
}
