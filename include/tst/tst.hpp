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
        
        bool lookup(tnode *node, size_t index, const string& prefix) {
            
            if ( node == nullptr )
                return false;
            
            if ( node->label.at(0) == EOS || index >= prefix.size())
                return true;
            
            if ( prefix.at(index) < node->label.at(0) )
                return lookup(node->lonode, index, prefix);
            else if ( prefix.at(index) == node->label.at(0) )
                return lookup(node->eqnode, index+1, prefix);
            else
                return lookup(node->hinode, index, prefix);
        }
        
        void lookup_test(tnode *root, tVPSU& string_weight) {
            for (auto it = string_weight.begin(); it != string_weight.end(); it++ ) {
                if ( ! lookup(root, 0, it->first) ) {
                    cerr << "error for " << it->first << endl;
                    exit(EXIT_FAILURE);
                }
            }
            
            cout << "all correct!\n";
        }
        
        size_t num_nodes = 0;
        
        size_t count_nodes(tnode *node) {
            if ( node == nullptr )
                return 0;
            else return 1 + count_nodes(node->lonode) + count_nodes(node->eqnode) + count_nodes(node->hinode); 
        }
        
        void build_tst_bp(tVPSU& string_weight, uint64_t N, uint64_t n) {
            tnode * root = build_tst(string_weight, 0, N-1, 0);
            num_nodes = count_nodes(root);
            lookup_test(root, string_weight);
        }
        
        tnode *build_tst(const tVPSU& string_weight, uint64_t first, uint64_t last, uint64_t index) {
            
            if ( first == 0 ) 
                cout << first << " ~ " << last << endl;
            
            if ( first > last )
                return nullptr;

            char ch;
            uint64_t sx, dx;
            std::tie(sx, dx, ch) = partitionate(string_weight, first, last, index);
            tnode *node = new tnode(ch);

            if ( first <= last && ch != EOS ) {
                node->lonode = build_tst(string_weight, first, sx-1, index);
                node->eqnode = build_tst(string_weight, sx, dx, index+1);
                node->hinode = build_tst(string_weight, dx+1, last, index);
            } else if ( first <= last && ch == EOS ) {
                assert ( first == last || first == last-1 );
                node->hinode = build_tst(string_weight, dx+1, last, index);
            } else {
                cerr << "wtf...\n";
                exit(EXIT_FAILURE);
            }
            
            if ( index == 1 ) {
                //compress...
                //build recursevily bp...
            }
            
            return node;
        }
        
        /* fa veramente schifo */
        tTUUC partitionate(const tVPSU& string_weight, uint64_t first, uint64_t last, uint64_t index) {
            uint64_t m = first + (last-first)/2;
            try {
            char ch = string_weight[m].first.at(index);
            uint64_t sx, dx;
            for ( sx = first; sx <= last && string_weight[sx].first.size() > index && string_weight[sx].first.at(index) != ch; sx++ );
            for ( dx = last; dx >= first && string_weight[dx].first.size() > index && string_weight[dx].first.at(index) != ch; dx-- );
            return make_tuple(sx,dx,ch);
            } catch (const std::out_of_range& e) {
                cerr << "error\n";
                cerr << string_weight[m].first << endl;
                cerr << first << " , " << last << endl;
                cerr << index << " - " << string_weight[m].first.size() << endl;
                exit(EXIT_FAILURE);
            } 
        }
        
    public:    
        size_t get_nodes() {
            return num_nodes;
        }
        
        size_t get_size() {
            return num_nodes*sizeof(tnode);
        }
        
    };
}
