
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

using namespace std;
using namespace chrono;

std::ifstream::pos_type filesize(const string& file) {
    std::ifstream in(file, std::ios::binary | std::ios::ate);
    return in.tellg(); 
}

void query_from_input(glad::tst<>& t) {
    cout << "Please enter queries line by line." << endl;
    cout << "Pressing Crtl-D will quit the program." << endl;
    string prefix;
    while ( getline(cin, prefix) ) {
        auto query_start = chrono::high_resolution_clock::now(); 
        auto result_list = t.top_k(prefix, 5);
        auto query_time  = chrono::high_resolution_clock::now() - query_start;
        auto query_us    = chrono::duration_cast<chrono::microseconds>(query_time).count();
        cout << "-- top results:" << endl;
        for (size_t i=0; i<result_list.size(); ++i) {
            cout << result_list[i].first << "  " << result_list[i].second << endl;
        }
        cout << "-- (" << std::setprecision(3) << query_us / 1000.0;
        cout << " ms)" << endl;
    }
}

int main(int argc, char *argv[]) {
    
    if ( argc < 2 ) {
        cerr << "Error: Insert filename\n";
        return 1;
    }
    const string file = std::string(argv[1]);
    const string index_file = file+"."+type+".sdsl";
    
    cout << "build index for " << file << endl;
    cout << "\nsize (MB): " << ((double) filesize(file))/1000000 << endl;
    cout << "--------------------\n";
            
    auto t1 = std::chrono::high_resolution_clock::now();
    //glad::tst<> t(file);
    glad::tst<> t;
    if ( ! load_from_file (t, index_file ) ) {
        cout << "building tst index for '" << file << "'..." << endl;
        glad::tst<> t1(file);
        cout << "store tst index in '" << index_file << "'..." << endl;
        store_to_file(t1, index_file);
        load_from_file (t, index_file ); //it's horrible.
    }
    
    auto t2 = std::chrono::high_resolution_clock::now();
    
    cout << "init: " << (duration_cast<duration<double>>(t2-t1)).count() << "sec\n" << \
            "nodes: " << t.get_nodes() << "\n" << \
            "space (MB): " << t.get_size()/1000000 << endl;
    
    query_from_input(t);
    //queries_at_random(t,file);
    return 0;
}
