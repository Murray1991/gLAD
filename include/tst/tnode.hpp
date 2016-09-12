#pragma once
#include <string>

using namespace std;

namespace glad {

    struct tnode {
        tnode(string label) : label(label), lonode(nullptr), eqnode(nullptr), hinode(nullptr) {};
        tnode(char ch = 0) : lonode(nullptr), eqnode(nullptr), hinode(nullptr) { label += ch; };
        ~tnode() { 
            delete lonode; 
            delete eqnode; 
            delete hinode; 
        }
        
        bool is_string() { return end_string; }
        bool is_leaf() { return !lonode && !eqnode && !hinode; }
        
        string label;
        tnode *lonode;
        tnode *eqnode;
        tnode *hinode;
        bool end_string = false;
    };
}
