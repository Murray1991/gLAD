
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

std::vector<string> get_prefixes(const string& file) {
    ifstream in(file);
    size_t size = 15000;
    std::vector<size_t> ints;
    std::vector<string> entries;
    std::vector<string> prefixes;
    
    prefixes.reserve(size);
    ints.reserve(size);
    for ( size_t i = 0; i < size; i++ )
        ints.push_back(rand()%size);
    
    for ( string entry ; getline(in, entry, '\t') ; ) {
        entries.push_back(entry);
        string s_weight;
        getline(in, s_weight);
    }
    
    for ( auto it = ints.begin(); it != ints.end(); it++ ) {
        auto prefix = entries.at(*it);
        if (prefix.length() > 4)
            prefix = prefix.substr(0,prefix.length()-3);
        prefixes.push_back(prefix);
    }
    
    return prefixes;
}

void queries_at_random(glad::tst<> t, const string& file) {
    auto prefixes = get_prefixes(file);
    
    auto t3 = std::chrono::high_resolution_clock::now();
    
    size_t found = 0;
    while ( ! prefixes.empty() ) {
        auto prefix = prefixes[0];
        prefixes.erase(prefixes.begin());
        auto query_start = chrono::high_resolution_clock::now(); 
        auto result_list = t.top_k(prefix, 5);
        auto query_time  = chrono::high_resolution_clock::now() - query_start;
        auto query_us    = chrono::duration_cast<chrono::microseconds>(query_time).count();
        found += result_list.size();
    }
    
    auto t4 = std::chrono::high_resolution_clock::now();
    cout << "found: " << found << endl;
    cout <<   "search: " << (duration_cast<duration<double>>(t4-t3)).count() << "sec\n";
}

void query_from_input(glad::tst<> t) {
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
        cerr << "Pirla, serve il nome del file\n";
        return 1;
    }
    srand(time(0));
    const string file = std::string(argv[1]);
    const string index_file = file+".sdsl";
    
    cout << "build index for " << file << \
            "\nsize (MB): " << ((double) filesize(file))/1000000 << endl;
            
    auto t1 = std::chrono::high_resolution_clock::now();
    glad::tst<> t;
    if ( ! load_from_file (t, index_file ) ) {
        cout << "not existing tst-sdsl file..." << endl;
        glad::tst<> tst2(file);
        store_to_file(tst2, index_file);
        cout << "stored!\n";
        load_from_file(t, index_file);
    }
    cout << "loaded!\n";
    auto t2 = std::chrono::high_resolution_clock::now();
    
    cout << "init: " << (duration_cast<duration<double>>(t2-t1)).count() << "sec\n" << \
            "nodes: " << t.get_nodes() << "\n" << \
            "space (MB): " << t.get_size()/1000000 << endl;
    
    query_from_input(t);
            
    return 0;
}
