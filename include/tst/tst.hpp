#pragma once
#include <iostream>
#include <algorithm>
#include <utility>
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
    
    typedef uint32_t                                    uint_t;
    typedef int                                         int_t;
    typedef std::tuple<uint_t, uint_t, char>            tTUUC;
    typedef std::pair<std::string, uint_t>              tPSU;
    typedef std::vector<tPSU>                           tVPSU;
    typedef tPSU *                                      tPSU2;
    typedef std::vector<tPSU2>                          tVPSU2;
    typedef std::vector<std::string *>                  tVS;
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
        
        void sort_unique(tVPSU2& string_weight) {
            cout << "sort strings\n";
            std::sort(string_weight.begin(), string_weight.end(), [](const tPSU2& a, const tPSU2& b) {
                return a->first.compare(b->first) < 0;
            });
            cout << "end sort strings\n";
            cout << "unique strings; from: " << string_weight.size() << endl;
            size_t count = 0;
            auto unique_end = std::unique(string_weight.begin(), string_weight.end(), [&](tPSU2& first, tPSU2& last) {
                //last.first => mind blown!
                if ( first->first.size() != last->first.size() )
                    return false;
                if ( first->first.compare(last->first) == 0 ) {
                    //keep the higher weight, is it really necessary? maybe better a sum?
                    first->second = first->second > last->second ? first->second : last->second; 
                    last->second = first->second > last->second ? first->second : last->second;
                    count++;
                    return true;
                }
                return false;
            });
            /*
            for ( tVPSU2::iterator it = unique_end; it != string_weight.end(); it++ )
                delete *it; */
            string_weight.resize(unique_end - string_weight.begin());
            string_weight.shrink_to_fit();
            cout << "end unique strings; deleted: " << count << endl;
        }
        
        void process_input(const std::string& filename, tVPSU2& string_weight) {
            cout << "process input...\n";
            std::ifstream in(filename);
            if ( !in ) {
                std::cerr << "Error: Could not open file " << filename << endl;
                exit(EXIT_FAILURE);
            }
            std::string entry;
            try {
                while ( std::getline(in, entry, '\t')) {
                    std::transform(entry.begin(), entry.end(), entry.begin(), ::tolower);
                    std::string s_weight;
                    std::getline(in, s_weight);
                    uint_t weight = stoull(s_weight);
                    string_weight.push_back(new tPSU( entry+EOS, weight ));
                }
            } catch ( std::bad_alloc& e ) {
                std::cerr << "Error: std::bad_alloc when filling from file..." << endl;
                std::exit(EXIT_FAILURE);
            }
            in.close();
            cout << "end process input...\n";
        }
        
    public:
        
        tst() {}
        
        ~tst() {}
        
        tst(const std::string& filename) {
            uint_t n = 0, N = 0, max_weight = 0;
            tVPSU2 string_weight;
            process_input(filename,string_weight);
            sort_unique(string_weight);
            for ( auto it = string_weight.begin() ; it != string_weight.end(); it++ ) {
                N += 1;
                n += (*it)->first.size();
                max_weight = max_weight > (*it)->second ? max_weight : (*it)->second;
            }
            m_weight = t_weight (N, 0, bits::hi(max_weight)+1);
            for ( size_t i = 0; i < N ; i++ ) {
                m_weight[i] = string_weight[i]->second;
            }

            cout << "number of: " << N << endl;
            build_tst_bp(string_weight, N, n, max_weight);
            
            m_rmq = t_rmq(&m_weight);
            
            cout << count_leaves() << endl;
            cout << N << endl;
            assert(count_leaves() == N);
        }

    private:
        
        sdsl::bit_vector::iterator bp_it;
        sdsl::bit_vector::iterator start_it;
        sdsl::bit_vector::iterator helper_it;
        sdsl::int_vector<8>::iterator label_it;
        
        void build_tst_bp(tVPSU2& strings, uint64_t N, uint64_t n, uint64_t max_weight) {
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
            std::cout << "start: build tst...\n";
            tnode * root  = build_tst(strings);
            std::cout << "end: build tst...\n";
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
            
        }
        
        tnode *build_tst (tVPSU2& strings) 
        {
            typedef std::tuple<int_t, int_t, int_t, int_t, bool, tnode *, bool> call_t;
            constexpr int_t target_level = 5; // >= 1 , with an higher number should be more efficient in memory consumption?s
            
            std::stack<call_t> stk;
            int_t sx, dx; char ch;
            tnode * root = new tnode(), * node;
            int_t first, last, index, level; bool help, marked;
            
            stk.emplace(0, strings.size()-1, 0, 0, 1, root, 0 <= target_level);
            while ( ! stk.empty() ) {
                
                std::tie(first, last, index, level, help, node, marked) = stk.top();
                
                if ( !marked )
                    stk.pop();  // pointers have not destructors, it's safe to call
                
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
                
                //cout << "[" << first << "," << last << "]" << ". partitionate " << index << " " << strings[first]->first.size() << endl;
                std::tie(sx, dx, ch) = partitionate(strings, first, last, index);
                node->label = ch;
                
                if ( level < target_level ) {
                    start_it++;
                    *(bp_it++) = 1;
                    *(start_it++) = 1;
                    *(label_it++) = ch; 
                    *(helper_it++) = help;
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
                        //cout << sx << ". deleted" << endl;
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
        tTUUC partitionate(const tVPSU2& strings, uint64_t first, uint64_t last, uint64_t index) {
            uint64_t m = first + (last-first)/2;
            uint64_t sx, dx;
            char ch = strings[m]->first.at(index);
            for ( sx = first; sx < last && strings[sx]->first.size() > index && strings[sx]->first.at(index) != ch; sx++ );
            for ( dx = last; dx > first && strings[dx]->first.size() > index && strings[dx]->first.at(index) != ch; dx-- );
            return make_tuple(sx,dx,ch);
        }
        
    public:    
        size_t get_nodes() {
            return count_nodes();
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
        
        bool lookup(string prefix) const {
            std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
            return search(prefix) > 0;
        }
        
        tVPSU top_k(std::string prefix, size_t k) const{
            std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
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
                // "eqnode" is always present for internal nodes: h == true => b = true;
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
            size_t index = m_rmq(range[0], range[1]);
            if ( range[0] != 0 || range[1] != 0 )
                q.push(make_tuple(range, index, m_weight[index]));
            while ( indexes.size() < k && !q.empty() ) {
                auto t = q.top();
                auto r = get<0>(t);
                auto i = get<1>(t);
                auto w = get<2>(t);
                //TODO limit the queue size?
                if ( r[0] < i ) {
                    auto idx = m_rmq(r[0],i-1);
                    //TODO check differences and lvalue/rvalue stuff, I've some confusion about these concepts
                    q.emplace((t_range){{r[0],i-1}}, idx, m_weight[idx]);
                    //q.push(make_tuple((t_range){{r[0],i-1}}, idx, m_weight[idx]));
                }
                if ( r[1] > i ) {
                    auto idx = m_rmq(i+1,r[1]);
                    q.emplace((t_range){{i+1,r[1]}}, idx, m_weight[idx]);
                    //q.push(make_tuple((t_range){{i+1,r[1]}}, idx, m_weight[idx])); 
                }
                indexes.push_back(i);
                q.pop();
            }
            return indexes;
        }
        
        std::string build_string(size_t idx) const {
            const char * data = (const char *) m_label.data();
            std::string str = "";
            size_t v = m_bp_sel10(idx+1)-1;
            size_t i, o;
            std::string lab;
            for ( bool b = true; v != 0 ; ) {
                i = get_start_label(v);
                o = get_end_label(v);
                if ( !b && o-i > 1 ) {
                    //TODO smarter way?
                    lab.assign(data+i, o-i-1);
                    str = std::move(lab) + str;
                }
                if ( b ) {
                    //TODO smarter way?
                    lab.assign(data+i, o-i);
                    str = std::move(lab) + str;
                }
                b = m_helper[node_id(v)-1];
                v = parent(v);
            }
            return str;
        }
        
        t_range prefix_range(const std::string& prefix) const {
            int64_t v = search(prefix);
            if ( v < 0 ) return {{0,0}};
            return {{m_bp_rnk10(v), m_bp_rnk10(m_bp_support.find_close(v)+1)-1}};
        }
        
        // lookup for scnt repr
        int64_t search(const string& prefix) const {
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
        
        size_t count_leaves() {
            return m_bp_rnk10(m_bp_support.find_close(0)+1);
        }
        
        size_t count_nodes() {
            return (m_bp.size()/2 + 1);
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
