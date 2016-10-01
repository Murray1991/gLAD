#pragma once

#include <iostream>
#include <algorithm>
#include <utility>
#include <tuple>
#include <cassert>
#include <cstring>
#include <sdsl/vectors.hpp>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/bp_support.hpp>
#include <sdsl/rmq_support.hpp>
#include "tnode.hpp"
#include "utils.hpp"

using namespace std;
using namespace sdsl;

namespace glad {
    
    template< typename t_bv = sdsl::bit_vector,
         typename t_sel= typename t_bv::select_1_type,
         typename t_weight = sdsl::vlc_vector<>,
         typename t_rmq = sdsl::rmq_succinct_sct<0>,
         typename t_bp_support = sdsl::bp_support_sada<>,
         typename t_bp_rnk10 = sdsl::rank_support_v5<10,2>,
         typename t_bp_sel10 = sdsl::select_support_mcl<10,2>
         >
         
    class tst {
        
        typedef sdsl::bit_vector        t_bv_uc;
        typedef sdsl::int_vector<8>     t_label;
        
        t_label             m_label;
        t_weight            m_weight;
        t_bv_uc             m_bp;
        t_bv                m_helper0;
        t_bv                m_helper1;
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
            DEBUG_STDOUT(sizeof(tPSU) << " " << sizeof(string) << " " << sizeof(string*) << endl); 
            sdsl::int_vector<> weights ( string_weight.size(), 0, bits::hi(max_weight)+1 );
            tVS strings;
            strings.reserve( string_weight.size() );
            size_t i=0;
            std::for_each( string_weight.begin(), string_weight.end(), [&weights, &strings, &i](tPSU& a) {
                  weights[i] = std::move(a.second);
                  strings.push_back(std::move(a.first));
                  i++;
            });
            DEBUG_STDOUT("-- erasing string_weight\n");
            decltype(string_weight){}.swap(string_weight);
            m_weight = t_weight( std::move(weights) );
            DEBUG_STDOUT("-- end building strings & weights\n");
            DEBUG_STDOUT("-- N: " << strings.size() << " , n: " << n << " , max_weight: " << max_weight << endl);
            DEBUG_STDOUT("-- start build_tst_bp\n");
            build_tst_bp(strings, strings.size(), n, max_weight);
            DEBUG_STDOUT("-- end build_tst_bp\n");
            
            m_rmq = t_rmq(&m_weight);
            assert(count_leaves() == strings.size());
        }
        
    public:    
        
        D ( __attribute__((noinline)) )
        tVPSU top_k (std::string prefix, size_t k) const {
            constexpr size_t g = 5; // "guess" constant multiplier
            std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
            std::string new_prefix(prefix.size(), 0);
            auto v       = blind_search(prefix, new_prefix);
            auto range   = prefix_range(v);
            auto top_idx = heaviest_indexes(range, k); /* TODO optimization: return v's instead thand idx... */
            tVPSU result_list;
            for (auto idx : top_idx){
                std::string s;
                s.reserve(g*prefix.size());
                s += new_prefix;
                s += std::move( build_string(m_bp_sel10(idx+1)-1, parent(v)) );
                result_list.push_back(tPSU(std::move(s), m_weight[idx]));
            }
            return result_list;
        }
        
        D ( __attribute__((noinline)) )
        size_t get_nodes() {
            return (m_bp.size()/2 + 1);
        }
        
        D ( __attribute__((noinline)) )
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
                size_in_bytes(m_helper0);
            return size;
        }
        
    private:
        
        sdsl::bit_vector::iterator bp_it;
        sdsl::bit_vector::iterator start_it;
        sdsl::bit_vector::iterator helper0_it;
        sdsl::bit_vector::iterator helper1_it;
        sdsl::int_vector<8>::iterator label_it;
        
        void build_tst_bp(tVS& strings, uint64_t N, uint64_t n, uint64_t max_weight) {
            t_bv_uc                   start_bv(2*N+n+2, 0);
            t_bv_uc                   helper0(n+N,0);
            t_bv_uc                   helper1(n+N,0);
            int_vector<8> labels      = int_vector<8>(n);
            m_bp                      = t_bv_uc(2*2*N, 0);
            
            bp_it                     = m_bp.begin();
            start_it                  = start_bv.begin();
            label_it                  = labels.begin();
            helper0_it                = helper0.begin();
            helper1_it                = helper1.begin();
            *(start_it++) = 1;
            
            tnode * root  = build_tst(strings);
            delete root;
            
            DEBUG_STDOUT("-- resizing...\n");
            m_bp.resize(bp_it-m_bp.begin()); 
            labels.resize(label_it-labels.begin()); 
            helper0.resize(helper0_it-helper0.begin());
            helper1.resize(helper1_it-helper1.begin());
            start_bv.resize(start_it-start_bv.begin());
            
            DEBUG_STDOUT("-- building data structures...\n");
            m_start_bv   = t_bv(start_bv);
            m_helper0    = t_bv(helper0);
            m_helper1    = t_bv(helper1);
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
                // put this if at the end, should be safe...
                if ( level < target_level ) {
                    start_it++;
                    *(bp_it++)      = 1;
                    *(start_it++)   = 1;
                    *(label_it++)   = ch; 
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
                        /* clear string added */
                        strings[sx].clear();
                    }
                }
                if ( first < sx && !node->lonode ) {
                    node->lonode = new tnode();
                    stk.emplace(first, sx-1, index, level+1, 0, node->lonode, level+1 <= target_level);    //lonode
                }
                // put this if at the end, should be safe...
                if ( level < target_level ) {

                    //TODO check!
                    if ( ! node->is_leaf() ) {
                        *helper0_it = node->eqnode && ( (node->lonode != 0) != (node->hinode != 0 ) );
                        *helper1_it = *helper0_it ? (node->lonode != 0) : (node->lonode && node->eqnode && node->hinode);
                        helper0_it++; helper1_it++;
                    }

                }
            }
            return root;
        }
        
        void mark(tnode * node, bool help) {
            if ( node == nullptr )
                return;
            for(auto it = node->label.begin(); it != node->label.end(); (*(label_it++) = *(it++)), start_it++);

            //TODO check!
            if ( ! node->is_leaf() ) {
                *helper0_it = node->eqnode && ( (node->lonode != 0) != (node->hinode != 0 ) );
                *helper1_it = *helper0_it ? (node->lonode != 0) : (node->lonode && node->eqnode && node->hinode);
                helper0_it++; helper1_it++;
            }

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
            char ch = strings[m].at(index);
            for ( sx = first; sx < last && strings[sx].size() > index && strings[sx].at(index) != ch; sx++ );
            for ( dx = last; dx > first && strings[dx].size() > index && strings[dx].at(index) != ch; dx-- );
            return make_tuple(sx,dx,ch);
        }
        
        D ( __attribute__((noinline)) )
        int64_t map_to_edge(const size_t v, const uint8_t ch1, const uint8_t ch2) const {
            const size_t idx = node_id(v)-m_bp_rnk10(v+1)-1;
            const bool h0 = m_helper0[idx];
            const bool h1 = m_helper1[idx];
            const bool nl = m_bp[v+1];                                      // true if v is not a leaf
            const size_t cv = v+1;
            int64_t retval = 0;
            
            auto l3  = !h0 && h1 && nl;                                     //(1) case 3 children
            auto l2a = h0 && h1 && nl && ch1 <= ch2;                        //(2) case 2 children with lonode
            auto l2b = h0 && !h1 && nl && ch1 >= ch2;                       //(3) case 2 children with hinode
            auto l1  = !h0 && !h1 && nl && ch1 == ch2;                      //(4) case 1 children
            auto l0  = !h0 && !h1 && !nl && ch1 == ch2;                     //(5) case leaf
            auto l23 = l3 || l2a || l2b;
            auto len = ( l3 && ch1 == ch2 ) + ( l3 && ch1>ch2 )*2 + \
                       ( l2a && ch1 == ch2 ) + \
                       ( l2b && ch1 > ch2 );                                //redundant, just for "clarity"
                       
            retval += (!l23 && !l0 && !l1)*(-1);                            //case no mapping
            retval += (l23 || l1)*cv;                                       //cv for (1) (2) (3) (4)
            retval += (l0)*v;                                               //v for l0
            
            /* here we go: for-loop is not executed if !l23 */
            for ( int i = 0; i < len; i++ )
                retval = m_bp_support.find_close(retval)+1;
            return retval;
        }
        
        D ( __attribute__((noinline)) )
        int64_t map_to_edge2(const size_t v, const uint8_t ch1, const uint8_t ch2) const {
            const size_t idx = node_id(v)-m_bp_rnk10(v+1)-1;
            const size_t h0 = m_helper0[idx];
            const size_t h1 = m_helper1[idx];
            size_t cv = v+1;
            size_t n = m_bp[v+1];       // check if leaf...          
            if ( !h0 && h1 && n ) {          //case of 3 children
                for ( int i=0; i < (ch1==ch2)+(ch1>ch2)*2; i++ )
                    cv = m_bp_support.find_close(cv)+1;
                return cv;
            } else if ( h0 && h1 && n && ch1 <= ch2) {    //case of 2 children with lonode
                for ( int i=0; i < (ch1==ch2); i++ )
                    cv = m_bp_support.find_close(cv)+1;
                return cv;
            } else if ( h0 && !h1 && n && ch1 >= ch2 ) {   //case of 2 children with hinode
                for ( int i=0; i < (ch1>ch2); i++ )
                    cv = m_bp_support.find_close(cv)+1;
                return cv;
            } else if ( !h0 && !h1 && n && ch1 == ch2 ) {    //case of 1 child
                return cv;
            } else if ( !h0 && !h1 && !n && ch1 == ch2 )    //case is a leaf
                return v;
            return -1;
        }

        D ( __attribute__((noinline)) )
        std::vector<size_t> heaviest_indexes( const t_range& range, const size_t k ) const {
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
        
        D ( __attribute__((noinline)) )
        bool check_if_eqnode(const size_t v, const size_t p) const {
            const size_t idx = node_id(p)-m_bp_rnk10(p+1)-1;
            const size_t h0 = m_helper0[idx];
            const size_t h1 = m_helper1[idx];
            const size_t cv = p+1;
            
            return ( ( !h1 && ( h0 || m_bp[cv] ) && cv == v ) || \
                     ( h1 && m_bp_support.find_close(cv)+1 == v ));
        }
        
        /* build string from node v_from upwards to node v_to (v_to is the parent of the node found via blind search or 0)*/
        /* attempt to optimization... should be better than previous solution... */
        D ( __attribute__((noinline)) )
        std::string build_string(const size_t v_from, const size_t v_to) const {
            const char * data = (const char *) m_label.data();
            std::vector<bool> b; b.reserve(50);    // "guess size"
            std::vector<size_t> v; v.reserve(50);    //"guess size"
            v.push_back(0);
            b.push_back(true);
            for ( size_t k = v_from ; k != v_to ; ) {
                auto p = parent(k);
                v.push_back(k);
                b.push_back(check_if_eqnode(k, p));
                k = p;
            }
            std::string str; 
            str.reserve(100); //guess
            size_t start, end;
            const size_t last = v.size()-1;
            for ( size_t i = last; i > 0; i-- ) {
                start   =  get_start_label(v[i]);
                end     =  get_end_label(v[i]);
                str.append(data + start, end - start - !b[i-1]);
            }
            return str;
        }
        
        /* build string from node v_from upwards to node v_to (v_to is the parent of the node found via blind search or 0)*/
        /* attempt to optimization... should be better than previous solution... */
        D ( __attribute__((noinline)) )
        std::string build_string3(const size_t v_from, const size_t v_to) const {
            const char * data = (const char *) m_label.data();
            std::vector<bool> b; b.reserve(50);    // "guess size"
            std::vector<size_t> v; v.reserve(50);    //"guess size"
            v.push_back(0);
            b.push_back(true);
            
            for ( size_t k = v_from; k != v_to; ) {
                auto p = parent(k);
                v.push_back(k);
            }
            for ( size_t k = v_from ; k != v_to ; ) {
                auto p = parent(k);
                v.push_back(k);
                b.push_back(check_if_eqnode(k, p));
                k = p;
            }
            std::string str; 
            str.reserve(100); //guess
            size_t start, end;
            const size_t last = v.size()-1;
            for ( size_t i = last; i > 0; i-- ) {
                start   =  get_start_label(v[i]);
                end     =  get_end_label(v[i]);
                str.append(data + start, end - start - !b[i-1]);
            }
            return str;
        }
        
        /* build string from node v_from upwards to node v_to (v_to is the parent of the node found via blind search or 0)*/
        D ( __attribute__((noinline)) )
        std::string build_string2(size_t v_from, size_t v_to) const {
            const char * data = (const char *) m_label.data();
            std::string str = "";
            size_t v = v_from;
            size_t i, o, p;
            bool b = true;
            //here we have loop-carried dependencies, because of b => TODO optimize!
            for ( b = true; v != v_to ; ) {
                i = get_start_label(v);
                o = get_end_label(v);
                std::string lab(data+i, o - i - !b);    //at the beginning is true => start from a leaf!
                str = std::move(lab) + str;
                p = parent(v);
                b = check_if_eqnode(v,p);               //check if this node for the parent is the eqnode
                v = p;                                  //go up!
            }
            return str;
        }
        
        D ( __attribute__((noinline)) )
        t_range prefix_range(const int64_t& v) const {
            if ( v < 0 ) return {{1,0}};
            return {{m_bp_rnk10(v), m_bp_rnk10(m_bp_support.find_close(v)+1)-1}};
        }
        
        D ( __attribute__((noinline)) )
        t_range prefix_range(const std::string& prefix) const {
            int64_t v = blind_search(prefix);
            if ( v < 0 ) return {{1,0}};
            return {{m_bp_rnk10(v), m_bp_rnk10(m_bp_support.find_close(v)+1)-1}};
        }
        
        /* actually this kind of "blind search" is useful if a string is not represented in the tst */
        D ( __attribute__((noinline)) )
        int64_t blind_search(const string& prefix, string& str) const {
            int64_t v = 0, i = 0;
            const char * data = (const char *) m_label.data();
            const size_t pref_len = prefix.size();
            size_t plen  = pref_len-i;
            size_t start = get_start_label(v);
            size_t end   = get_end_label(v);
            size_t llen  = end-start;
            std::strncpy(&str[i], data+start, (llen <= plen )*llen + ( plen < llen )*plen);
            while ( plen >= llen && i != pref_len && v >= 0 ) {
                i     += llen-1;
                v     = map_to_edge(v, prefix.at(i), data[end-1]);
                if ( v < 0 ) return v;  //TODO is it possible to optimize?
                i     += (prefix.at(i) == data[end-1]);
                plen  = pref_len-i;
                start = get_start_label(v);
                end   = get_end_label(v);
                llen  = end-start;
                std::strncpy(&str[i], data+start, (llen <= plen )*llen + ( plen < llen )*plen);
            }
            /* here I have to match if the string is correct... */
            if ( v > 0 && prefix.compare(str) == 0 ) {
                for ( ; plen != 0 && llen != 0 ; plen--, llen-- ) 
                    str.pop_back();
                return v;
            }
            return -1;
        }
        
        D ( __attribute__((noinline)) )
        size_t count_leaves() {
            return m_bp_rnk10(m_bp_support.find_close(0)+1);
        }
        
        D ( __attribute__((noinline)) )
        size_t node_id(const size_t v) const{
            return m_bp_support.rank(v);
        }
        
        D ( __attribute__((noinline)) )
        string get_label(const size_t v) const {
            const char * data = (const char *) m_label.data();
            auto i = get_start_label(v);
            auto o = get_end_label(v);
            string s(data+i, o-i);
            return s;
        }
        
        D ( __attribute__((noinline)) )
        size_t get_start_label(size_t v) const {
            return m_start_sel(node_id(v)) + 1 - node_id(v);
        }
        
        D ( __attribute__((noinline)) )
        size_t get_end_label(const size_t v) const {
            return m_start_sel(node_id(v)+1) + 1 - (node_id(v)+1);
        }
        
        D ( __attribute__((noinline)) )
        size_t is_leaf(const size_t v) const {
            return m_bp[v+1] == 0;
        }
        
        D ( __attribute__((noinline)) )
        size_t is_root(const size_t v) const {
            return v == 0;
        }

        D ( __attribute__((noinline)) )
        size_t parent(const size_t v) const {
            return m_bp_support.enclose(v);
        }
        
        D ( __attribute__((noinline)) )
        std::vector<size_t> children(const size_t v) const {
            std::vector<size_t> res;
            for ( size_t cv = v+1; m_bp[cv]; ) {
                res.push_back(cv);
                if ( res.size() == 3 )
                    break;
                cv = m_bp_support.find_close(cv)+1; 
            }
            return res;
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
            written_bytes += m_helper0.serialize(out, child, "helper");
            written_bytes += m_helper1.serialize(out, child, "helper1");
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
            m_helper0.load(in);
            m_helper1.load(in);
        }
    };
}
