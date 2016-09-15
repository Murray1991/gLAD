
#include <iostream>
#include <algorithm>
#include <chrono>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include "tst.hpp"

using namespace std;
using namespace chrono;

typedef std::__cxx11::basic_string<uint8_t> ustring;

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

bool test_lookups(const string& file, glad::tst<>& t) {
    ifstream in(file);
    size_t k = 0, j = 0;
    
    
    cout << "start...\n";
    auto t3 = std::chrono::high_resolution_clock::now();
    
    for ( string entry ; getline(in, entry, '\t'); j++ ) {
        string s_w;
        if ( t.lookup(entry) ) k++;
        else cout << "WTF IS WRONG???\n";
        getline(in, s_w);
    }
    cout << j << " lookups..." << endl;
    auto t4 = std::chrono::high_resolution_clock::now();
    cout <<   "search: " << (duration_cast<duration<double>>(t4-t3)).count() << "sec\n";
    return k==j;
}

std::vector<string> get_prefixes(const string& file) {
    srand(time(0));
    ifstream in(file);
    size_t size = 50;
    std::vector<size_t> ints;
    std::vector<string> entries;
    std::vector<string> prefixes;
    
    prefixes.reserve(size);
    ints.reserve(size);
    
    for ( string entry ; getline(in, entry, '\t') ; ) {
        entries.push_back(entry);
        string s_weight;
        getline(in, s_weight);
    }
    
    //cout << "numbers of entries: " << entries.size();
    for ( size_t i = 0; i < size; i++ )
        ints.push_back(rand() % entries.size());
    
    for ( auto it = ints.begin(); it != ints.end(); it++ ) {
        auto prefix = entries.at(*it);
        if (prefix.length() > 4)
            prefix = prefix.substr(0,prefix.length()-3);
        prefixes.push_back(prefix);
    }
    
    return prefixes;
}

void queries_at_random(glad::tst<>& t, const string& file) {
    auto prefixes = get_prefixes(file);
    
    cout << "start...\n";
    auto t3 = std::chrono::high_resolution_clock::now();
    
    size_t found = 0;
    while ( ! prefixes.empty() ) {
        auto prefix = prefixes[0];
        prefixes.erase(prefixes.begin());
        //auto query_start = chrono::high_resolution_clock::now(); 
        //cout << "search " << prefix << endl;
        auto result_list = t.top_k(prefix, 5);
        //auto query_time  = chrono::high_resolution_clock::now() - query_start;
        //auto query_us    = chrono::duration_cast<chrono::microseconds>(query_time).count();
        found += result_list.size();
    }
    
    auto t4 = std::chrono::high_resolution_clock::now();
    cout << "end, found:: " << found << endl;
    cout << "search: " << (duration_cast<duration<double>>(t4-t3)).count() << "sec\n";
    
    //query_from_input(t);
}


int main(int argc, char *argv[]) {
    
    if ( argc < 2 ) {
        cerr << "Error: Insert filename\n";
        return 1;
    }
    const string file = std::string(argv[1]);
    const string index_file = file+".sdsl";
    
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
    bool b = test_lookups(file, t);
    if ( b ) cout << "ALL CORRECT\n" << endl;
    else cout << "WTF????\n";
    
}
