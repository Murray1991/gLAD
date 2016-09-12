#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <ctime>
#include <cassert>
#include "tst.hpp"

using namespace std;

void esare(string& str) {
    
    int i = rand() % str.size() + 1;
    while ( --i && str.size() > 1 ) str.pop_back();
}

int main(int argc, char **argv) {
    
    srand (time(NULL));
    
    printf("glad version %d.%d\n",tst_VERSION_MAJOR, tst_VERSION_MINOR);
    string filename = "data/Dictionary_Wikipedia.txt";
    
    cout << "open dictionary '" << filename << "'" << endl;
    ifstream dictionary(filename);
    if (!dictionary) {
        cerr << "file not opened..." << endl;
        exit(1);
    }
    
    cout << "building tst for '" << filename << "'" << endl;
    tst t(filename);
    
    cout << "lookup of: '" << "endo" << "': " << t.lookup("endo") << endl;
    
    vector<string> v;
    v.push_back("helloworld");
    v.push_back("cia");
    v.push_back("ciao");
    v.push_back("ciaon");
    v.push_back("cyclothymiac");
    v.push_back("cyclothymia");
    v.push_back("oh");
    v.push_back("ohh");
    v.push_back("ok");
    v.push_back("itt");
    v.push_back("ittttt");
    v.push_back("byk");
    v.push_back("o");
    
    for ( int i=0; i < v.size(); i++ )
        cout << "lookup of: '" << v[i] << "': " << t.lookup(v[i]) << endl;
    
    /*
    int dim = 1;
    for ( string line ; getline(dictionary, line) ; dim++ ) {
        if (line.length() == 0) continue;
        assert(t.lookup(line));
        esare(line);
        assert(t.lookup(line));
    } */
    
    dictionary.close();
    
    cout << "Something about the tst: " << endl;
    cout << "\tnodes: " << t.nodes() << endl;
    cout << "\tsize in bytes: " << t.size_in_bytes() << endl;
    return 0;
}
