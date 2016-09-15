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
    typedef std::array<size_t,2>                        t_range;
    
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
        
        t_rmq               m_rmq;
        
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
            m_weight = t_weight (N, 0, bits::hi(max_weight)+1);
            for ( size_t i = 0; i < N ; i++ )
                m_weight[i] = string_weight[i].second;
            m_rmq = t_rmq(&m_weight);
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
            tnode * root  = build_tst(string_weight, 0, N-1, 0, 0, 1);
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
            
            assert(count_leaves() == N);
        }
        
        tnode *build_tst(const tVPSU& string_weight, int64_t first, int64_t last, int64_t index, int64_t level, int help) {
            if ( first > last || ( first == last && string_weight[first].first.length() == index ) )
                return nullptr;
            
            constexpr int64_t target_level = 1;
            uint64_t sx, dx; char ch;
            std::tie(sx, dx, ch) = partitionate(string_weight, first, last, index);
            sdsl::bit_vector::iterator it;
            
            if ( level < target_level ) {
                *(bp_it++) = 1;
                start_it++;
                *(label_it++) = ch; 
                *(start_it++) = 1;
                *(helper_it++) = help;
            }
            
            tnode *node = new tnode(ch);
            node->lonode = build_tst(string_weight, first, sx-1, index, level+1, 0);
            if ( level == target_level - 1 ) {
                delete node->lonode;
                node->lonode = nullptr;
            }
            node->eqnode = build_tst(string_weight, sx, dx, index+1, level+1, 1);
            if ( level == target_level - 1 ) {
                delete node->eqnode;
                node->eqnode = nullptr;
            }
            node->hinode = build_tst(string_weight, dx+1, last, index, level+1, 0);
            if ( level < target_level ) {
                bp_it++;
                if ( level == target_level-1 ) {
                    delete node->hinode;
                    node->hinode = nullptr;
                }
            }
            if ( level == target_level ) {
                compress(node);
                mark(node, help);
            }
            return node;
        }
        
        void mark(tnode * node, int help) {
            if ( node == nullptr )
                return;
            
            for(auto it = node->label.begin(); it != node->label.end(); (*(label_it++) = *(it++)), start_it++);
            *(helper_it++) = help;
            *(start_it++) = 1;
            *(bp_it++) = 1;
            mark(node->lonode, 0);
            mark(node->eqnode, 1);
            mark(node->hinode, 0);
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
                char& lchar = node->label.at(len-1);
                assert(lchar == EOS);
                node->lonode = new tnode(lchar); //EOS
                tnode *tmp = node->hinode;
                lchar = tmp->label.at(0);
                node->eqnode = tmp->eqnode;
                tmp->eqnode = nullptr;
                node->hinode = nullptr;
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
            return count_nodes();
        }
        
        size_t get_size() {
            return 0;
        }
        
        inline bool lookup(const string& prefix) const {
            return search(prefix) > 0;
        }
        
        tVPSU top_k(const std::string& prefix, size_t k) const{
            auto range = prefix_range(prefix);
            auto top_idx = heaviest_indexes(range, k);
            tVPSU result_list;
            for (auto idx : top_idx){
                auto str = build_string(idx);
                result_list.push_back(tPSU(str, m_weight[idx]));
            }
            return result_list;
        }
        
    private:
        
        inline int64_t map_to_edge(size_t v, uint8_t ch1, uint8_t ch2) const {
            size_t cv = v+1;
            for ( bool b = false, h = false; m_bp[cv] ; b = h ) {
                /* "eqnode" is always present, when it is encountered b == true */
                h = m_helper[node_id(cv)-1];
                if ( (!b && !h && ch1 < ch2) || (h && ch1 == ch2) || (b && !h && ch1 > ch2)) {
                    return cv;
                }
                cv = m_bp_support.find_close(cv)+1;
            }
            if ( cv == v+1 && ch1 == ch2 ) {
                return v;
            }
            return -1;
        }
        
        std::vector<size_t> heaviest_indexes( t_range range, size_t k ) const {
            typedef std::tuple<t_range, size_t, size_t> t_q;
            auto cmp = [](const t_q& a, const t_q& b) { 
                return get<2>(a) < get<2>(b); 
            };
            
            std::vector<size_t> indexes;
            std::priority_queue<t_q, std::vector<t_q>, decltype(cmp)> q(cmp);
            auto index = m_rmq(range[0], range[1]);
            q.push(make_tuple(range, index, m_weight[index]));
            while ( indexes.size() < k && !q.empty() ) {
                auto t = q.top();
                auto r = get<0>(t);
                auto i = get<1>(t);
                auto w = get<2>(t);
                //TODO limit the queue size too
                if ( r[0] < i ) {
                    auto idx = m_rmq(r[0],i-1);
                    q.push(make_tuple((t_range){{r[0],i-1}}, idx, m_weight[idx]));
                }
                if ( r[1] > i ) {
                    auto idx = m_rmq(i+1,r[1]);
                    q.push(make_tuple((t_range){{i+1,r[1]}}, idx, m_weight[idx])); 
                }
                indexes.push_back(i);
                q.pop();
            }
            return indexes;
        }
        
        inline std::string build_string(size_t idx) const {
            std::string str = "";
            size_t v = m_bp_sel10(idx+1)-1;
            for ( bool b = true; v != 0 ; ) {
                auto lab = get_label(v);
                if ( !b && lab.size() > 1 ) {
                    lab.pop_back();
                    str = lab + str;
                }
                if ( b ) 
                    str = lab + str;
                b = m_helper[node_id(v)-1];
                v = parent(v);
            }
            return str;
        }
        
        t_range prefix_range(const std::string& prefix) const {
            auto v = search(prefix);
            if ( v < 0 )
                return {{0,0}};
            return {{m_bp_rnk10(v), m_bp_rnk10(m_bp_support.find_close(v)+1)-1}};
        }
        
        // lookup for scnt repr
        inline int64_t search(const string& prefix) const {
            int64_t v = 0, i = 0;
            for ( string lab = get_label(v); ; lab = get_label(v) ) {
                auto plen = prefix.length()-i;
                auto llen = lab.length();
                if ( plen < llen ) {
                    return ( !prefix.compare(i, plen, lab, 0, plen) ? v : -1 );
                }
                //useless if blind search
                if ( llen > 1 && prefix.compare(i, llen-1, lab, 0, llen-1) != 0)
                    return -1;
                //there is always a character
                i += llen-1;
                v = map_to_edge(v, prefix.at(i), lab.back());
                //cout << "v = " << v << endl << endl;
                i += (prefix.at(i) == lab.back());
                if ( i == prefix.length() )
                    break;
            }
            return v;
        }
        
        size_t count_leaves() {
            return m_bp_rnk10(m_bp_support.find_close(0)+1);
        }
        
        size_t count_nodes() {
            return (m_bp.size()/2 + 1);
        }
        
        inline size_t node_id(size_t v) const{
            return m_bp_support.rank(v);
        }
        
        inline std::string get_label(size_t v) const {
            size_t i = m_start_sel(node_id(v)) + 1 - node_id(v);
            size_t o = m_start_sel(node_id(v)+1) + 1 - (node_id(v)+1);
            //TODO here string is copied, it's possible to optimize...
            std::string s((char *) m_label.data(), i, o-i);
            return std::move(s);
        }

        inline size_t is_leaf(size_t v) const {
            return m_bp[v+1] == 0;
        }

        inline size_t is_root(size_t v) const {
            return v == 0;
        }

        inline size_t parent(size_t v) const {
            return m_bp_support.enclose(v);
        }
        
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
