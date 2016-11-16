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
            process_input(filename, string_weight);
            sort_unique(string_weight);
            std::for_each( string_weight.begin() , string_weight.end(), [&n,&max_weight](const tPSU& a) {
                n+= a.first.size();
                if ( a.second > max_weight ) max_weight = a.second; 
            });
            sdsl::int_vector<> weights ( string_weight.size(), 0, bits::hi(max_weight)+1 );
            tVS strings;
            strings.reserve( string_weight.size() );
            size_t i=0;
            std::for_each( string_weight.begin(), string_weight.end(), [&weights, &strings, &i](tPSU& a) {
                  weights[i] = std::move(a.second);
                  strings.push_back(std::move(a.first));
                  i++;
            });
            decltype(string_weight){}.swap(string_weight);
            m_weight = t_weight( std::move(weights) );
            build_tst_bp(strings, strings.size(), n, max_weight);
            
            m_rmq = t_rmq(&m_weight);            
            assert(count_leaves() == strings.size());
        }
        
        D ( __attribute__((noinline)) )
        tVPSU top_k (std::string prefix, size_t k) const {
            constexpr size_t g = 5; // "guess" constant multiplier
            std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
            std::string new_prefix(prefix.size(), 0);
            auto v       = search(prefix, new_prefix);
            auto range   = prefix_range(v);
            auto top_idx = heaviest_indexes(range, k);
            tVPSU result_list;
            std::string s;
            for (auto idx : top_idx) {
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
                size_in_bytes(m_helper0) +
                size_in_bytes(m_helper1);
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
            
            std::cout << "-- build tst...\n";
            tnode * root  = build_tst(strings);
            delete root;
            
            std::cout << "-- resizing bit vectors...\n";
            m_bp.resize(bp_it-m_bp.begin()); 
            labels.resize(label_it-labels.begin());
            helper0.resize(helper0_it-helper0.begin());
            helper1.resize(helper1_it-helper1.begin());
            start_bv.resize(start_it-start_bv.begin());
            
            std::cout << "-- building data structures...\n";
            m_start_bv   = t_bv(start_bv);
            m_helper0    = t_bv(helper0);
            m_helper1    = t_bv(helper1);
            m_label      = t_label(labels);
            m_start_sel  = t_sel(&m_start_bv);
            m_bp_support = t_bp_support(&m_bp);
            util::init_support(m_bp_rnk10, &m_bp);
            util::init_support(m_bp_sel10, &m_bp);
        }
        
        struct frame {
            frame (tnode *& node, int_t sx, int_t dx, int_t index, bool called, bool markval) : node(node), sx(sx), dx(dx), index(index), called(called), markval(markval) {};
            tnode *&node;
            int_t sx;
            int_t dx;
            int_t index;
            bool called;
            bool markval;
        };
        
        tnode * build_tst (tVS& strings) {
            auto fun = [&] (tnode*& node, bool& b, int_t start, int_t end, int_t index, bool markval) {
                node = rec_build_tst (strings, start, end, index);
                compress(node);
                mark(node, markval);
                b = node != 0;
                delete node;
                node = nullptr;
            };
            
            int_t sx, dx; uint8_t ch;
            bool lo = false, eq = false, hi = false;
            std::stack<frame> stk;
            tnode *root = nullptr;
            stk.emplace(root, 0, strings.size()-1, 0, false, true);

            while ( !stk.empty() ) {
                frame& f = stk.top();
                if (!f.called) {
                    std::tie(sx, dx, ch) = partitionate(strings, f.sx, f.dx, f.index);
                    f.node = new tnode(ch);
                    start_it++;
                    *(start_it++) = 1;
                    *(bp_it++)    = 1;
                    *(label_it++) = ch;
                    lo = f.sx < sx;
                    eq = sx <= dx;
                    hi = dx < f.dx;
                    auto h0 = eq && ( lo != hi );
                    *(helper0_it++) = h0;
                    *(helper1_it++) = h0 ? (lo != 0) : (lo && eq && hi);
                    if ( f.index == 1 ) {
                        bool a, b, c;
                        fun(f.node->lonode, a, f.sx, sx-1, f.index, false);
                        fun(f.node->eqnode, b, sx, dx, f.index+1, true);
                        fun(f.node->hinode, c, dx+1, f.dx, f.index, false);
                    } else if ( f.index < 1 ) {
                        if ( dx < f.dx )
                            stk.emplace(f.node->hinode, dx+1, f.dx, f.index, false, false);
                        stk.emplace(f.node->eqnode, sx, dx, f.index+1, false, true);
                        if  ( f.sx < sx )
                            stk.emplace(f.node->lonode, f.sx, sx-1, f.index, false, false);
                    } else{
                        std::cout << "ERROR\n";
                    }
                } else {
                    stk.pop();
                    bp_it++;
                }
                f.called = true;
            }
            return root;
        }
        
        tnode * rec_build_tst (tVS& strings, int_t first, int_t last, int_t index) {
            if ( last < first ) {
                return nullptr;
            }
            
            uint8_t ch;
            int_t sx, dx;
            std::tie(sx, dx, ch) = partitionate(strings, first, last, index);
            tnode *node = new tnode(ch);
            node->lonode = rec_build_tst (strings, first, sx-1, index);
            if ( sx < dx || (sx == dx && ch != EOS)) {
                node->eqnode = rec_build_tst (strings, sx, dx, index+1);
            }
            if ( sx == dx && ch == EOS ) {
                strings[sx].clear();
            }
            node->hinode = rec_build_tst(strings, dx+1, last, index);

            return node;
        }
        
        void mark(tnode * node, bool help) {
            if ( node == nullptr )
                return;
            for(auto it = node->label.begin(); it != node->label.end(); (*(label_it++) = *(it++)), start_it++);
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
                       ( l2b && ch1 > ch2 );                              //redundant, just for "clarity"
                       
            retval += (!l23 && !l0 && !l1)*(-1);                            //case no mapping
            retval += (l23 || l1)*cv;                                       //cv for (1) (2) (3) (4)
            retval += (l0)*v;                                               //v for l0
            
            /* here we go: for-loop is not executed if !l23 */
            for ( int i = 0; i < len; i++ )
                retval = m_bp_support.find_close(retval)+1;
            return retval;
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
        
        /* build string from node v_from upwards to node v_to (v_to is the parent of the node found via search or 0)*/
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
            if (str.back() == EOS)
                str.pop_back();
            return str;
        }
        
        D ( __attribute__((noinline)) )
        t_range prefix_range(const int64_t& v) const {
            if ( v < 0 ) return {{1,0}};
            return {{m_bp_rnk10(v), m_bp_rnk10(m_bp_support.find_close(v)+1)-1}};
        }
        
        D ( __attribute__((noinline)) )
        t_range prefix_range(const std::string& prefix) const {
            int64_t v = search(prefix);
            if ( v < 0 ) return {{1,0}};
            return {{m_bp_rnk10(v), m_bp_rnk10(m_bp_support.find_close(v)+1)-1}};
        }
        
        D ( __attribute__((noinline)) )
        void label_copy(size_t start, std::string& str, size_t i, size_t len) const {
            for (size_t k = i, j = 0; j < len; j++, k++)
                str[k] = m_label[start+j];
        }
        
        D ( __attribute__((noinline)) )
        int64_t search(const string& prefix, string& str) const {
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
            auto id = node_id(v);
            return m_start_sel(id) + 1 - id;
        }
        
        D ( __attribute__((noinline)) )
        size_t get_end_label(size_t v) const {
            auto id = node_id(v);
            return m_start_sel(id+1) + 1 - (id+1);
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
