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
            m_helper.resize(helper_it-m_helper.begin());
            m_label.resize(label_it-m_label.begin()); 
            start_bv.resize(start_it-start_bv.begin());
            
            m_start_bv   = t_bv(start_bv);
            m_start_sel  = t_sel(&m_start_bv);
            m_bp_support = t_bp_support(&m_bp);
            m_bp_rnk10   = t_bp_rnk10(&m_bp);
            m_bp_sel10   = t_bp_sel10(&m_bp);
            
            num_leaves = count_leaves(root);
            cout << "leaves: " << num_leaves << " ; strings: " << N << endl;
            lookup_test(string_weight);
        }
        
        tnode *build_tst(const tVPSU& string_weight, int64_t first, int64_t last, int64_t index, int64_t level) {
            
            constexpr int64_t target_level = 1;
            
            if ( first > last || ( first == last && string_weight[first].first.length() == index ) )
                return nullptr;
            
            char ch;
            uint64_t sx, dx;
            std::tie(sx, dx, ch) = partitionate(string_weight, first, last, index);
            sdsl::bit_vector::iterator it;
            
            if ( level < target_level ) {
                *(bp_it++) = 1;
                start_it++;
                *(label_it++) = ch; 
                *(start_it++) = 1;
                it = helper_it++;
            }
            
            tnode *node = new tnode(ch);
            node->lonode = build_tst(string_weight, first, sx-1, index, level+1);
            if ( level == target_level - 1 ) {
                delete node->lonode;
                node->lonode = nullptr;
            }
            
            node->eqnode = build_tst(string_weight, sx, dx, index+1, level+1);
            if ( level == target_level - 1 ) {
                delete node->eqnode;
                node->eqnode = nullptr;
            }
            
            node->hinode = build_tst(string_weight, dx+1, last, index, level+1);
            if ( level < target_level ) {
                bp_it++;
                *it = (node->hinode != nullptr);
                if ( level == target_level-1 ) {
                    delete node->hinode;
                    node->hinode = nullptr;
                }
            }
            
            if ( level == target_level ) {
                compress(node);
                mark(node);
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
        
        int64_t map_to_edge(size_t v, char ch1, char ch2) {
            // TODO reimplement for efficiency reasons
            // m_helper[a_id] = 1 iff node with id==a_id has ( lo,eq,hi || !lo, eq, hi )
            // m_helper[a_id] = 0 iff node with id==a_id has ( lo,eq,!hi || !lo, eq, !hi )
            // TODO orcaputt che casino -> migliorare
            auto n = children(v);
            auto k = n.size();
            // TODO mah, forse meglio aggiungere un bit a m_helper ? piu' pulito?
            auto h = m_helper[node_id(v)-1];
            /*
            cout << "ch1: " << ch1 << " ~ ch2: " << ch2 << " ~ h: " << h << " ~ k: " << k << " ~ node_id: " << node_id(v) << " ~ ch1 < ch2: " << ((uint8_t) ch1<(uint8_t) ch2) << endl;
            for ( auto it=n.begin() ; it != n.end() ; it++ )
                    cout << *it << " (" << get_label(*it) << ") , ";
            cout << endl;
            */
            
            if ( k && (uint8_t) ch1 > (uint8_t) ch2 && h==1 )
                return n[1 + (k==3)];
            if ( k && (uint8_t) ch1 < (uint8_t) ch2 && ( (h==1 && k==3) || (h==0 && k > 1) ) )
                return n[0];
            if ( k && ch1 == ch2 )
                return n[0 + (k==3 || (k==2 && h==0))];
            if ( !k && ch1!=ch2 )
                return -2;
            if ( !k && ch1==ch2 )
                return -1;
            
            return -2;
        }
        
        // lookup for scnt repr
        bool lookup(const string& prefix) {
            int64_t v = 0, i = 0;
            for ( string lab = get_label(v); ; lab = get_label(v) ) {
                auto plen = prefix.length()-i;
                auto llen = lab.length();
                auto len = plen < llen ? plen : llen-1;
                auto b = !prefix.compare(i, len, lab, 0, len);
                if ( plen < llen )
                    return b;
                //useless if blind search
                if ( len > 0 && !b )
                    return false;
                i += len;
                v = map_to_edge(v, prefix.at(i), lab.back());
                if ( v < 0 ) break;
                i += (prefix.at(i) == lab.back());
            }
            return v == -1;
        }
        
        void lookup_test(tVPSU& string_weight) {
            constexpr int levels = 1;
            for (auto it = string_weight.begin(); it != string_weight.begin() + 2000; it++ ) {
                for ( int i = 0; i < levels; i++ ) {
                    if ( it->first.size() > i ) {
                        if ( ! lookup(it->first.substr(0, it->first.size()-i) ) ) {
                            cerr << "error for " << it->first.substr(0, it->first.size()-i) << " , index: " << it - string_weight.begin() << endl;
                            exit(EXIT_FAILURE);
                        }
                    }
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
        
        std::string get_label(size_t v) {
            size_t i = m_start_sel(node_id(v)) + 1 - node_id(v);
            size_t o = m_start_sel(node_id(v)+1) + 1 - (node_id(v)+1);
            std::string s((char *) m_label.data(), i, o-i);
            return s;
        }

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
        
        // Return all children of v
        std::vector<size_t> children(size_t v) const {
            std::vector<size_t> res;
            size_t cv = v+1;
            while ( m_bp[cv] ) {
                res.push_back(cv);
                cv = m_bp_support.find_close(cv) + 1;
            }
            return res;
        }
        
    };
}
