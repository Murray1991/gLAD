#pragma once
#include <iostream>
#include <algorithm>
#include <tuple>
#include <cassert>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/bp_support.hpp>
#include <sdsl/rmq_support.hpp>
#include "tnode.hpp"

using namespace std;

namespace glad {
 
    #define EOS ((char) 3)
    
    typedef std::tuple<uint64_t, uint64_t, char>        tTUUC;
    typedef std::pair<std::string, uint64_t>            tPSU;
    typedef std::vector<tPSU>                           tVPSU;
    
    template<typename t_bv = sdsl::sd_vector<>,
         typename t_sel= typename t_bv::select_1_type,
         typename t_rac_weight = sdsl::int_vector<>,
         typename t_bp_support = sdsl::bp_support_sada<>,
         typename t_bp_rnk10 = sdsl::rank_support_v5<10,2>,
         typename t_bp_sel10 = sdsl::select_support_mcl<10,2>,
         typename t_rmq = sdsl::rmq_succinct_sct<0>,
         typename t_bv_uc = sdsl::bit_vector
         >
         
    class tst {
        
    private:
        void process_input(const std::string& filename, tVPSU& string_weight, uint64_t& N, uint64_t& n) const{
            
            std::ifstream in(filename);
            if ( !in ) {
                cerr << "Error: Could not open file " << filename << endl;
                exit(EXIT_FAILURE);
            }
            
            std::string entry;
            while ( std::getline(in, entry, '\t')) {
                std::transform(entry.begin(), entry.end(), entry.begin(), ::tolower);
                std::string s_weight;
                std::getline(in, s_weight);
                uint64_t weight = stoull(s_weight);
                string_weight.emplace_back(entry + EOS, weight);
            }
            
            std::sort(string_weight.begin(), string_weight.end(), [](const tPSU& a, const tPSU& b) {
                return a.first.compare(b.first) < 0;
            });
            
            auto unique_end = std::unique(string_weight.begin(), string_weight.end(), [](tPSU& a, tPSU& b) {
                if ( a.first.size() != b.first.size() )
                    return false;
                if ( a.first.compare(b.first) == 0 ) {
                    a.second += b.second;
                    return true;
                }
                return false;
            });
            
            string_weight.resize(unique_end-string_weight.begin());
            
            for ( auto it = string_weight.begin() ; it != string_weight.end(); it++ ) {
                N += 1;
                n += it->first.size();
            }
        }
        
    public:
        tst() {}
        ~tst() {}
        tst(const std::string& filename) {
            uint64_t n = 0, N = 0;
            tVPSU string_weight;
            process_input(filename, string_weight, N, n);
            std::cout << n << " " << N << std::endl;
            build_tst_bp(string_weight, N, n);
        }

    private:
        void build_tst_bp(tVPSU& string_weight, uint64_t N, uint64_t n) {
            tnode * root = build_tst(string_weight, 0, N-1, 0, 0);
            num_nodes = count_nodes(root);
            num_leaves = count_leaves(root);
            cout << "num leaves: " << num_leaves+numm << endl;
            cout << "start lookup_test\n";
            lookup_test(root, string_weight);
        }
        
        tnode *build_tst(const tVPSU& string_weight, int64_t first, int64_t last, int64_t index, int64_t level) {
            
            if ( first > last )
                return nullptr;

            char ch;
            uint64_t sx, dx;
            std::tie(sx, dx, ch) = partitionate(string_weight, first, last, index);

            tnode *node = new tnode(ch);

            if ( first <= last && ch != EOS ) {
                node->lonode = build_tst(string_weight, first, sx-1, index, level+1);
                node->eqnode = build_tst(string_weight, sx, dx, index+1, level+1);
                node->hinode = build_tst(string_weight, dx+1, last, index, level+1);
            } else if ( first <= last && ch == EOS ) {
                assert ( first == last || first == last-1 );
                node->hinode = build_tst(string_weight, dx+1, last, index, level+1);
            } else {
                cerr << "wtf...\n";
                exit(EXIT_FAILURE);
            }
            
            if ( level == 2 ) {
                cout << ch << endl;
                compress(node);
            }
            
            return node;
        }
        
        inline void clear (tnode * node) {
            node->lonode = node->eqnode = node->hinode = 0;
            delete node;
        }
        
        void compress (tnode * node) {
            
            if ( node == nullptr )
                return;
            
            while ( !node->lonode && node->eqnode && !node->hinode ) {
                tnode * next = node->eqnode;
                node->label += next->label;
                node->lonode = next->lonode;
                node->eqnode = next->eqnode;
                node->hinode = next->hinode;
                next->eqnode = nullptr;
                clear(next);
            }
            
            if ( !node->lonode && !node->eqnode && node->is_end() && node->hinode ) {
                /* in this case node->hinode is not compressed yet -> rotation! */

                int len = node->label.length();
                char lchar = node->label.at(len-1);
                node->lonode = new tnode(lchar);
                tnode *tmp = node->hinode;
                node->label[len-1] = tmp->label.at(0);
                node->eqnode = tmp->eqnode;
                node->hinode = nullptr;
                tmp->eqnode = nullptr;
                assert(!tmp->lonode & !tmp->eqnode && !tmp->hinode); //by construction
                delete tmp;
            }
            
            if ( node->lonode && !node->eqnode && node->is_end() && !node->hinode ) {
                cout << "y";
            }
            
            compress(node->lonode);
            compress(node->eqnode);
            compress(node->hinode);
        }
        
        /* fa veramente schifo */
        tTUUC partitionate(const tVPSU& string_weight, uint64_t first, uint64_t last, uint64_t index) {
            uint64_t m = first + (last-first)/2;
            char ch = string_weight[m].first.at(index);
            uint64_t sx, dx;
            for ( sx = first; sx < last && string_weight[sx].first.size() > index && string_weight[sx].first.at(index) != ch; sx++ );
            for ( dx = last; dx > first && string_weight[dx].first.size() > index && string_weight[dx].first.at(index) != ch; dx-- );
            return make_tuple(sx,dx,ch);
        }
        
    public:    
        size_t get_nodes() {
            return num_nodes;
        }
        
        size_t get_size() {
            return num_nodes*sizeof(tnode);
        }
        
    private:
        
        inline int eat(const string& a, const string& b) const {
            auto ita = a.begin();
            auto itb = b.begin();
            for ( ; ita != a.end() && itb != b.end()-1 && *ita == *itb ; ita++, itb++ );
            return (ita-a.begin());
        }
        
        bool lookup(tnode *node, size_t index, const string& prefix) {
            if (node == nullptr)
                return false;
            
            int llen = node->label.length();
            if (node->label.at(llen-1) == EOS || index >= prefix.size())
                return true;
            
            int k = eat(prefix.substr(index), node->label);
            //cout << prefix.substr(index) << " ~ " << node->label << " ~ " << k << endl;
            if ( index + k < prefix.length() && llen-1 != k )
                return false;
            if ( index + k == prefix.length() )
                return true;
            
            /* here k == llen-1 */
            index += k;
            if ( (uint8_t) prefix.at(index) < (uint8_t) node->label.at(k) )
                return lookup(node->lonode, index, prefix);
            else if ( (uint8_t) prefix.at(index) == (uint8_t) node->label.at(k) )
                return lookup(node->eqnode, index+1, prefix);
            else
                return lookup(node->hinode, index, prefix);
        }
        
        void lookup_test(tnode *root, tVPSU& string_weight) {
            
            for (auto it = string_weight.begin(); it != string_weight.end(); it++ ) {
                if ( ! lookup(root, 0, it->first) ) {
                    cerr << "error for " << it->first << " , index: " << it - string_weight.begin() << endl;
                    exit(EXIT_FAILURE);
                }
            }
            
            cout << "all correct!\n";
        }
        
        size_t num_nodes = 0;
        size_t num_leaves = 0;
        
        size_t count_leaves(tnode *node) {
            if ( node == nullptr )
                return 0;
            if ( node->is_leaf() )
                return 1;
            return count_leaves(node->lonode) + count_leaves(node->eqnode) + count_leaves(node->hinode);
        }
        
        size_t count_nodes(tnode *node) {
            if ( node == nullptr )
                return 0;
            return 1 + count_nodes(node->lonode) + count_nodes(node->eqnode) + count_nodes(node->hinode); 
        }
        
    };
}
