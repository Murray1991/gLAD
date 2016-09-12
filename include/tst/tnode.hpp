#pragma once
#include <string>

using namespace std;

namespace glad {

    struct tnode {
        tnode(string label) : label(label), lonode(nullptr), eqnode(nullptr), hinode(nullptr) {};
        tnode(char ch = 0) : lonode(nullptr), eqnode(nullptr), hinode(nullptr) { label += ch; };
        virtual ~tnode() { 
            delete lonode; 
            delete eqnode; 
            delete hinode; 
        }
        
        bool is_leaf() { return !lonode && !eqnode && !hinode; }
        virtual bool is_string() { return end_string; }
        
        string label;
        tnode *lonode;
        tnode *eqnode;
        tnode *hinode;
        int id = -1;
        bool end_string = false;
    };

    struct tstring_node : tnode {
        tstring_node(string label, uint64_t weight, uint64_t id) : tnode(label), weight(weight), id(id) {}
        tstring_node(char ch, uint64_t weight, uint64_t id) : tnode(ch), weight(weight), id(id) {}
        virtual bool is_string() { return true; }
        uint64_t weight;
        uint64_t id;
    };

}
