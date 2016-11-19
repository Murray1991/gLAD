#include <iostream>
#include <fstream>
#include <cstdlib>
#include <chrono>

#ifdef TST1
    std::string type = "tst1.txt";
    #include "tst1.hpp"
#endif

#ifdef TST2
    std::string type = "tst2.txt";
    #include "tst2.hpp"
#endif

#ifdef TST3
    std::string type = "tst3.txt";
    #include "tst3.hpp"
#endif
    
#ifdef TST4
    std::string type = "tst4.txt";
    #include "tst4.hpp"
#endif

#include "utils.hpp"

using namespace std;
using namespace glad;

void build_strings(std::string file, std::vector<std::string> &strings) {
    std::ifstream in(file);
    for (std::string entry; getline(in, entry); ) {
        strings.push_back(std::move(entry));
    }
}

int main(int argc, char *argv[]) {
    if ( argc != 4 ) {
        std::cerr << argc << "...";
        std::cerr << "fatal error, need two args" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    int k = atoi(argv[1]);
    std::string out_file("outfile."+type);
    std::string test_file(argv[2]);
    std::string index_file(argv[3]);
    std::vector<std::string> prefixes;
    build_strings(test_file, prefixes);
    
    glad::tst<> index;
    if ( !load_from_file(index, index_file) ) {
        std::cerr << "index does not exists..." << std::endl;
        std::cerr << "please, build it before calling this test" << std::endl;
        exit(EXIT_FAILURE);
    }
    double total_us = 0;
    size_t found = 0;
    
    glad::trunc_file(out_file);
    auto start = chrono::high_resolution_clock::now();
    for ( auto& prefix : prefixes ) {
        auto query_start = chrono::high_resolution_clock::now(); 
        auto result_list = index.top_k(prefix, k);
        auto query_time  = chrono::high_resolution_clock::now() - query_start;
        auto query_us    = chrono::duration_cast<chrono::microseconds>(query_time).count();
        total_us        += query_us;
        
        write_in_file(out_file, prefix, result_list);
        found           += result_list.size();
    } 
    auto end = chrono::high_resolution_clock::now();
    auto average = total_us/prefixes.size();
    
    std::ifstream t(out_file);
    std::stringstream buffer;
    buffer << t.rdbuf();
    //std::remove(out_file.c_str());
     
    std::hash<std::string> hash_fn;
    size_t buffer_hash = hash_fn(buffer.str());
    std::cout << k << "\t" << average << "\t" << found << "\t" << prefixes.size() << "\t" << buffer_hash << endl;

    return 0;
}
