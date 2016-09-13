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
using namespace sdsl;

namespace glad {
 
    #define EOS ((char) 3)
    
    typedef std::tuple<uint64_t, uint64_t, char>        tTUUC;
    typedef std::pair<std::string, uint64_t>            tPSU;
    typedef std::vector<tPSU>                           tVPSU;
    
    template<typename t_bv = sdsl::sd_vector<>,
         typename t_sel= typename t_bv::select_1_type,
         typename t_weight = sdsl::int_vector<>,
         typename t_bp_support = sdsl::bp_support_sada<>,
         typename t_bp_rnk10 = sdsl::rank_support_v5<10,2>,
         typename t_bp_sel10 = sdsl::select_support_mcl<10,2>,
         typename t_rmq = sdsl::rmq_succinct_sct<0>,
         typename t_bv_uc = sdsl::bit_vector
         >
         
    class tst {
        
        typedef sdsl::int_vector<8> t_label;
        
        t_label             m_label;
        t_weight            m_weight;
        t_bv_uc             m_bp;
        t_bv_uc             m_helper;
        t_bp_support        m_bp_support;
        
        t_bv                m_start_bv;
        t_sel               m_start_sel;
        t_bp_rnk10          m_bp_rnk10;
        t_bp_sel10          m_bp_sel10;
        
    private:
        
        void process_input(const std::string& filename, tVPSU& string_weight) const{
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
        }
        
    public:
        
        tst() {}
        
        ~tst() {}
        
        tst(const std::string& filename) {
            uint64_t n = 0, N = 0, max_weight = 0;
            tVPSU string_weight;
            process_input(filename, string_weight);
            for ( auto it = string_weight.begin() ; it != string_weight.end(); it++ ) {
                N += 1;
                n += it->first.size();
                max_weight = max_weight > it->second ? max_weight : it->second;
            }
            //m_weight = t_weight (N, 0, bits::hi(max_weight)+1);
            //for ( size_t i = 0; i < N ; m_weight[i] = string_weight[i++].second );
            build_tst_bp(string_weight, N, n, max_weight);
        }

    private:
        
        sdsl::bit_vector::iterator bp_it;
        sdsl::bit_vector::iterator start_it;
        sdsl::bit_vector::iterator helper_it;
        sdsl::int_vector<8>::iterator label_it;
        
        void build_tst_bp(tVPSU& string_weight, uint64_t N, uint64_t n, uint64_t max_weight) {
            //TODO check the initialization values...
            t_bv_uc start_bv(2*N+n+2, 0);
            m_label     = t_label(n);
            m_bp        = t_bv_uc(2*2*N, 0);
            m_helper    = t_bv_uc(n+N, 0);
            
            bp_it       = m_bp.begin();
            start_it    = start_bv.begin();
            label_it    = m_label.begin();
            helper_it   = m_helper.begin();

            *(start_it++) = 1;
            tnode * root  = build_tst(string_weight, 0, N-1, 0, 0);
            delete root;
            
            m_bp.resize(bp_it-m_bp.begin()); 
            m_label.resize(label_it-m_label.begin()); 
            start_bv.resize(start_it-start_bv.begin());
            
            m_start_bv   = t_bv(start_bv);
            m_start_sel  = t_sel(&m_start_bv);
            m_bp_support = t_bp_support(&m_bp);
            m_bp_rnk10   = t_bp_rnk10(&m_bp);
            m_bp_sel10   = t_bp_sel10(&m_bp);
            
            num_leaves = count_leaves(root);
            cout << "leaves: " << num_leaves << " ; strings: " << N << endl;
        }
        
        tnode *build_tst(const tVPSU& string_weight, int64_t first, int64_t last, int64_t index, int64_t level) {
            
            if ( first > last || ( first == last && string_weight[first].first.length() == index ) )
                return nullptr;
            
            char ch;
            uint64_t sx, dx;
            std::tie(sx, dx, ch) = partitionate(string_weight, first, last, index);
            sdsl::bit_vector::iterator it;
            
            if ( level < 2 ) {
                *(bp_it++) = 1;
                start_it++;
                *(label_it) = ch; 
                *(start_it++) = 1;
                it = helper_it++;
            }
            
            tnode *node = new tnode(ch);
            node->lonode = build_tst(string_weight, first, sx-1, index, level+1);
            node->eqnode = build_tst(string_weight, sx, dx, index+1, level+1);
            node->hinode = build_tst(string_weight, dx+1, last, index, level+1);
            
            if ( level < 2 ) {
                bp_it++;
                *it = (node->hinode != nullptr);
            }
            
            if ( level == 2 ) {
                compress(node);
                mark(node);
                delete node;
                node = nullptr;
            }
            
            return node;
        }
        
        void mark(tnode * node) {
            
            if ( node == nullptr )
                return;
            
            for(auto it = node->label.begin(); it != node->label.end(); (*(label_it++) = *(it++)), start_it++);
            *(helper_it++) = (node->hinode != nullptr);
            *(start_it++) = 1;
            *(bp_it++) = 1;
            mark(node->lonode);
            mark(node->eqnode);
            mark(node->hinode);
            
            bp_it++;
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
            
            compress(node->lonode);
            compress(node->eqnode);
            compress(node->hinode);
        }
        
        /* fa veramente schifo */
        tTUUC partitionate(const tVPSU& string_weight, uint64_t first, uint64_t last, uint64_t index) {
            uint64_t m = first + (last-first)/2;
            uint64_t sx, dx;
            char ch = string_weight[m].first.at(index);
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
            return m_bp_rnk10(m_bp_support.find_close(0)+1);
        }
        
        size_t count_nodes(tnode *node) {
            if ( node == nullptr )
                return 0;
            return 1 + count_nodes(node->lonode) + count_nodes(node->eqnode) + count_nodes(node->hinode); 
        }
        
        // Map node v to its unique identifier. node_id : v -> [1..N]
        size_t node_id(size_t v) const{
            return m_bp_support.rank(v);
        }

        // Get edge label leading to node v with node_id(v) = v_id
        /*t_edge_label edge(size_t v_id) const{
            size_t begin = m_start_sel(v_id) + 1 - v_id;
            size_t end   = m_start_sel(v_id+1) + 1 - (v_id+1);
            return t_edge_label(&m_labels, begin, end);
        }*/

        // Check if v is a leaf
        size_t is_leaf(size_t v) const {
            return m_bp[v+1] == 0;
        }

        // Check if v is the root node
        size_t is_root(size_t v) const {
            return v == 0;
        }

        // Return parent of v
        size_t parent(size_t v) const {
            return m_bp_support.enclose(v);
        }
        
    };
}
