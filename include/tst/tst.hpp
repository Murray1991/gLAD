#pragma once

#include <iostream>
#include <algorithm>
#include <utility>
#include <tuple>
#include <cassert>
#include <sdsl/vectors.hpp>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/bp_support.hpp>
#include <sdsl/rmq_support.hpp>
#include "tnode.hpp"
#include "utils.hpp"

using namespace std;
using namespace sdsl;

namespace glad {
    
    template< typename t_bv = sdsl::rrr_vector<>,
         typename t_sel= typename t_bv::select_1_type,
         typename t_weight = sdsl::vlc_vector<>,
         typename t_rmq = sdsl::rmq_succinct_sct<0>,
         typename t_bp_support = sdsl::bp_support_sada<>,
         typename t_bp_rnk10 = sdsl::rank_support_v5<10,2>,
         typename t_bp_sel10 = sdsl::select_support_mcl<10,2>
         >
         
    class tst {
        
        typedef std::vector<string*>    tVS;
        typedef sdsl::bit_vector        t_bv_uc;
        typedef sdsl::int_vector<8>     t_label;
        
        t_label             m_label;
        t_weight            m_weight;
        t_bv_uc             m_bp;
        t_bv                m_helper;
        t_bp_support        m_bp_support;
        t_bv                m_start_bv;
        t_sel               m_start_sel;
        t_bp_rnk10          m_bp_rnk10;
        t_bp_sel10          m_bp_sel10;
        t_rmq               m_rmq;

    public:
        
        tst() {}
        
        ~tst() {}
        
        tst(const std::string& filename) {
            uint_t n = 0, N = 0, max_weight = 0;
            tVPSU string_weight;
            DEBUG_STDOUT("-- start getting input\n");
            process_input(filename, string_weight);
            DEBUG_STDOUT("-- end getting input\n");
            DEBUG_STDOUT("-- start processing input (sort + unique)\n");
            sort_unique(string_weight);
            DEBUG_STDOUT("-- end processing input\n");
            std::for_each( string_weight.begin() , string_weight.end(), [&n,&max_weight](const tPSU& a) {
                n+= a.first.size();
                if ( a.second > max_weight ) max_weight = a.second; 
            });
            
            DEBUG_STDOUT("-- building strings & weights\n");
            sdsl::int_vector<> weights ( string_weight.size(), 0, bits::hi(max_weight)+1 );
            tVS strings;
            strings.reserve( string_weight.size() );
            size_t i=0;
            std::for_each( string_weight.begin(), string_weight.end(), [&weights, &strings, &i](tPSU& a) {
                  weights[i] = std::move(a.second);
                  strings.push_back(new std::string(std::move(a.first)));
                  i++;
            });
            DEBUG_STDOUT("-- erasing string_weight\n");
            decltype(string_weight){}.swap(string_weight);
            DEBUG_STDOUT("-- end building strings & weights\n");
            DEBUG_STDOUT("-- N: " << strings.size() << " , n: " << n << " , max_weight: " << max_weight << endl);
            DEBUG_STDOUT("-- start build_tst_bp\n");
            build_tst_bp(strings, strings.size(), n, max_weight);
            DEBUG_STDOUT("-- end build_tst_bp\n");
            
            m_weight = t_weight( weights );
            m_rmq = t_rmq(&m_weight);
            assert(count_leaves() == strings.size());
        }
        
    public:    
        
        tVPSU top_k (std::string prefix, size_t k) const {
            std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
            auto range = prefix_range(prefix);
            auto top_idx = heaviest_indexes(range, k);
            tVPSU result_list;
            for (auto idx : top_idx){
                auto str = build_string(idx);
                result_list.push_back(tPSU(std::move(str), m_weight[idx]));
            }
            return result_list;
        }
        
        bool lookup(string prefix) const {
            std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
            return search(prefix) > 0;
        }
        
        size_t get_nodes() {
            return (m_bp.size()/2 + 1);
        }
        
        size_t get_size() {
            size_t size = size_in_bytes(m_label) +
                size_in_bytes(m_bp) +
                size_in_bytes(m_bp_support) +
                size_in_bytes(m_bp_rnk10) +
                size_in_bytes(m_bp_sel10) +
                size_in_bytes(m_start_bv) +
                size_in_bytes(m_start_sel) +
                size_in_bytes(m_weight) +
                size_in_bytes(m_rmq) +
                size_in_bytes(m_helper);
            return size;
        }
        
    private:
        
        sdsl::bit_vector::iterator bp_it;
        sdsl::bit_vector::iterator start_it;
        sdsl::bit_vector::iterator helper_it;
        sdsl::int_vector<8>::iterator label_it;
        
        void build_tst_bp(tVS& strings, uint64_t N, uint64_t n, uint64_t max_weight) {
            t_bv_uc                   start_bv(2*N+n+2, 0);
            t_bv_uc                   helper(n+N,0);
            int_vector<8> labels      = int_vector<8>(n);
            m_bp                      = t_bv_uc(2*2*N, 0);
            
            bp_it                     = m_bp.begin();
            start_it                  = start_bv.begin();
            label_it                  = labels.begin();
            helper_it                 = helper.begin();
            *(start_it++) = 1;
            
            tnode * root  = build_tst(strings);
            delete root;
            
            DEBUG_STDOUT("-- resizing...\n");
            m_bp.resize(bp_it-m_bp.begin()); 
            labels.resize(label_it-labels.begin()); 
            helper.resize(helper_it-helper.begin());
            start_bv.resize(start_it-start_bv.begin());
            
            DEBUG_STDOUT("-- building data structures...\n");
            m_start_bv   = t_bv(start_bv);
            m_helper     = t_bv(helper);
            m_label      = t_label(labels);
            m_start_sel  = t_sel(&m_start_bv);
            m_bp_support = t_bp_support(&m_bp);
            util::init_support(m_bp_rnk10, &m_bp);
            util::init_support(m_bp_sel10, &m_bp);
        }
        
        tnode *build_tst (tVS& strings) 
        {
            typedef std::tuple<int_t, int_t, int_t, int_t, bool, tnode *, bool> call_t;
            constexpr int_t target_level = 5; // >= 1 , with an higher number should be more efficient in memory consumption?
            
            std::stack<call_t> stk;
            int_t sx, dx; char ch;
            tnode * root = new tnode(), * node;
            int_t first, last, index, level; bool help, marked;
            
            stk.emplace(0, strings.size()-1, 0, 0, 1, root, 0 <= target_level);
            while ( ! stk.empty() ) 
            {
                std::tie(first, last, index, level, help, node, marked) = stk.top();
                
                if ( !marked ) {
                    stk.pop();
                }
                if ( marked && !node->is_leaf() ) {
                    stk.pop();
                    if ( level < target_level )
                        bp_it++;
                    if ( level == target_level ) {
                        compress(node);
                        mark(node, help);
                        delete node->hinode;
                        delete node->eqnode;
                        delete node->lonode;
                        node->hinode = node->eqnode = node->lonode = nullptr;
                    }
                    continue;
                }
            
                std::tie(sx, dx, ch) = partitionate(strings, first, last, index);
                node->label = ch;
                if ( level < target_level ) {
                    start_it++;
                    *(bp_it++)      = 1;
                    *(start_it++)   = 1;
                    *(label_it++)   = ch; 
                    *(helper_it++)  = help;
                }
                if ( dx < last && !node->hinode ) {
                    node->hinode = new tnode();
                    stk.emplace(dx+1, last, index, level+1, 0, node->hinode, level+1 <= target_level);     //hinode
                }
                if ( sx <= dx && !node->eqnode ) {
                    if ( sx < dx || (sx == dx && ch != EOS)) {
                        node->eqnode = new tnode();
                        stk.emplace(sx, dx, index+1, level+1, 1, node->eqnode, level+1 <= target_level);       //eqnode
                    }
                    if (sx == dx && ch == EOS) {
                        if ( marked ) {
                            if ( level < target_level )
                                bp_it++;
                            else {
                                mark(node,help);
                            }
                            stk.pop();
                        }
                        /* delete the string without resize the container */
                        delete strings[sx];
                        strings[sx] = nullptr;
                    }
                }
                if ( first < sx && !node->lonode ) {
                    node->lonode = new tnode();
                    stk.emplace(first, sx-1, index, level+1, 0, node->lonode, level+1 <= target_level);    //lonode
                }
            }
            return root;
        }
        
        void mark(tnode * node, bool help) {
            if ( node == nullptr )
                return;
            for(auto it = node->label.begin(); it != node->label.end(); (*(label_it++) = *(it++)), start_it++);
            *(helper_it++)  = help;
            *(start_it++)   = 1;
            *(bp_it++)      = 1;
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
        
        // TODO more "elegant" way? maybe using stl
        inline tTUUC partitionate(const tVS& strings, uint64_t first, uint64_t last, uint64_t index) {
            uint64_t m = first + (last-first)/2;
            uint64_t sx, dx;
            char ch = strings[m]->at(index);
            for ( sx = first; sx < last && strings[sx]->size() > index && strings[sx]->at(index) != ch; sx++ );
            for ( dx = last; dx > first && strings[dx]->size() > index && strings[dx]->at(index) != ch; dx-- );
            return make_tuple(sx,dx,ch);
        }
        
        inline int64_t map_to_edge(size_t v, uint8_t ch1, uint8_t ch2) const {
            size_t cv = v+1;
            for ( bool b = false, h = false; m_bp[cv] ; b = h ) {
                // "eqnode" is always present for internal nodes: (h == true) => (b = true)
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
        
        std::vector<size_t> heaviest_indexes( t_range& range, size_t k ) const {
            typedef std::tuple<t_range, size_t, size_t> t_q;
            auto cmp = [](const t_q& a, const t_q& b) { 
                return get<2>(a) < get<2>(b); 
            };
            std::vector<size_t> indexes;
            std::priority_queue<t_q, std::vector<t_q>, decltype(cmp)> q(cmp);
            if ( range[0] <= range[1] ) {
                size_t index = m_rmq(range[0], range[1]);
                q.push(make_tuple(range, index, m_weight[index]));
            }
            while ( indexes.size() < k && !q.empty() ) {
                auto t = q.top();
                auto r = get<0>(t);
                auto i = get<1>(t);
                auto w = get<2>(t);
                if ( r[0] < i ) {
                    auto idx = m_rmq(r[0],i-1);
                    q.emplace((t_range){{r[0],i-1}}, idx, m_weight[idx]);
                }
                if ( r[1] > i ) {
                    auto idx = m_rmq(i+1,r[1]);
                    q.emplace((t_range){{i+1,r[1]}}, idx, m_weight[idx]);
                }
                indexes.push_back(i);
                q.pop();
            }
            return indexes;
        }
        
        inline std::string build_string(size_t idx) const {
            const char * data = (const char *) m_label.data();
            std::string str = "";
            size_t v = m_bp_sel10(idx+1)-1;
            size_t i, o;
            bool b = true;
            std::string lab;
            for ( b = true; v != 0 ; ) {
                i = get_start_label(v);
                o = get_end_label(v);
                if ( !b && o-i > 1 ) {
                    lab.assign(data+i, o-i-1);
                    str = std::move(lab) + str;
                }
                if ( b ) {
                    lab.assign(data+i, o-i);
                    str = std::move(lab) + str;
                }
                b = m_helper[node_id(v)-1];
                v = parent(v);
            }
            //TODO here assumption that first node has only one character...
            if ( b ) str = *(data)+str;
            return str;
        }
        
        /* build string from node v_from upwards to node v_to */
        inline std::string build_string(size_t v_from, size_t v_to) const {
            const char * data = (const char *) m_label.data();
            std::string str = "";
            size_t v = v_from;
            size_t i, o;
            bool b = true; /* starting node is assumpted to be true... */
            std::string lab;
            for ( b = true; v != v_to ; ) {
                i = get_start_label(v);
                o = get_end_label(v);
                if ( !b && o-i > 1 ) {
                    lab.assign(data+i, o-i-1);
                    str = std::move(lab) + str;
                }
                if ( b ) {
                    lab.assign(data+i, o-i);
                    str = std::move(lab) + str;
                }
                b = m_helper[node_id(v)-1];
                v = parent(v);
            }
            //TODO here assumption that first node has only one character...
            if ( b ) str = *(data)+str;
            return str;
        }
        
        inline t_range prefix_range(const size_t& v) const {
            if ( v < 0 ) return {{1,0}};
            return {{m_bp_rnk10(v), m_bp_rnk10(m_bp_support.find_close(v)+1)-1}};
        }
        
        inline t_range prefix_range(const std::string& prefix) const {
            int64_t v = blind_search(prefix);
            if ( v < 0 ) return {{1,0}};
            return {{m_bp_rnk10(v), m_bp_rnk10(m_bp_support.find_close(v)+1)-1}};
        }
        
        /* actually this kind of "blind search" is useful if a string is not represented in the tst */
        int64_t blind_search(const string& prefix) const {
            int64_t v = 0, i = 0;
            const char * data = (const char *) m_label.data();
            const size_t pref_len = prefix.size();
            size_t plen, start, end, llen;
            while ( i != pref_len && v >= 0 ) {
                plen  = pref_len-i;
                start = get_start_label(v);
                end   = get_end_label(v);
                llen  = end-start;
                if ( plen < llen )
                    break;
                i += llen-1;
                v = map_to_edge(v, prefix.at(i), data[end-1]);
                i += (prefix.at(i) == data[end-1]);
            }
            /* here I have to match if the string is correct... */
            if ( v > 0 && build_string(v,0).find(prefix) != 0 )
                return -1;
            
            return v;
        }
        
        /* complete search */
        inline int64_t search(const string& prefix) const {
            int64_t v = 0, i = 0;
            const char * data = (const char *) m_label.data();
            const size_t pref_len = prefix.size();
            size_t plen, start, end, llen;
            while ( i != pref_len && v >= 0 ) {
                plen = pref_len-i;
                start = get_start_label(v);
                end = get_end_label(v);
                llen = end-start;
                if ( plen < llen ) {
                    return ( !prefix.compare(i, plen, data+start, plen) ? v : -1 );
                }
                if ( llen > 1 && prefix.compare(i, llen-1, data+start, llen-1) != 0)
                    return -1;
                i += llen-1;
                v = map_to_edge(v, prefix.at(i), data[end-1]);
                i += (prefix.at(i) == data[end-1]);
            }
            return v;
        }
        
        inline size_t count_leaves() {
            return m_bp_rnk10(m_bp_support.find_close(0)+1);
        }
        
        inline size_t node_id(size_t v) const{
            return m_bp_support.rank(v);
        }
        
        inline size_t get_start_label(size_t v) const {
            return m_start_sel(node_id(v)) + 1 - node_id(v);
        }
        
        inline size_t get_end_label(size_t v) const {
            return m_start_sel(node_id(v)+1) + 1 - (node_id(v)+1);
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
        
    public:
        
        typedef size_t size_type;
        
        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="") const {
            using namespace sdsl;
            auto child = structure_tree::add_child(v, name, util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_label.serialize(out, child, "labels");
            written_bytes += m_bp.serialize(out, child, "bp");
            written_bytes += m_bp_support.serialize(out, child, "bp_support");
            written_bytes += m_bp_rnk10.serialize(out, child, "bp_rnk10");
            written_bytes += m_bp_sel10.serialize(out, child, "bp_sel10");
            written_bytes += m_start_bv.serialize(out, child, "start_bv");
            written_bytes += m_start_sel.serialize(out, child, "start_sel");
            written_bytes += m_weight.serialize(out, child, "weight");
            written_bytes += m_rmq.serialize(out, child, "rmq");
            written_bytes += m_helper.serialize(out, child, "helper");
            structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void load(std::istream& in) {
            m_label.load(in);
            m_bp.load(in);
            m_bp_support.load(in, &m_bp);
            m_bp_rnk10.load(in, &m_bp);
            m_bp_sel10.load(in, &m_bp);
            m_start_bv.load(in);
            m_start_sel.load(in,&m_start_bv);
            m_weight.load(in);
            m_rmq.load(in);
            m_helper.load(in);
        }
    };
}
