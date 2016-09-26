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
        
        bool is_end() {
            return label.at(label.size()-1) == 3;
        }
        
        bool is_leaf() { 
            return !lonode && !eqnode && !hinode;
        }
        
        string label;
        tnode *lonode;
        tnode *eqnode;
        tnode *hinode;
    };
}
