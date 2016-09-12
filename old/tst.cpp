/*
 * Copyright 2015 <copyright holder> <email>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#include "tst.hpp"

#define DEBUG
#ifdef DEBUG
#define D(x) do { x; } while(0)
#else
#define D(x) do { } while(0)
#endif

using namespace sdsl;
using namespace std;

typedef tuple<int, int, int> triple_t;
typedef tuple<char,int,int> node_t;

vector<node_t> construct_tst(const string filename) {
    
    ifstream dictionary(filename);
    vector<node_t> nodes;
    map<int,string> m;
    int dim = 0;
    
    if (!dictionary) {
        cerr << "file not opened..." << endl;
        exit(1);
    }
    
    for ( string line ; getline(dictionary, line) ; ) {
        if (line.length() == 0)continue;
        m.insert(pair<int,string>(dim++, line));
    }
    
    int index = 0, cx, sx = 0, dx = dim-1;
    char c;

    queue<triple_t> q;
    
    nodes.push_back(node_t(0,1,index));
    q.push(make_tuple(sx,dx,index));
    
    for ( triple_t el = q.front() ; !q.empty() ; q.pop() , el = q.front() ) {
        
        sx = get<0>(el); 
        dx = get<1>(el);
        index = get<2>(el);
        cx = sx + (dx-sx)/2;
        
        //D( cout << "m[cx] = " << m[cx] << " ..." << "<" << sx <<  "," << dx << "," << index << ">" << endl );
        
        // correct character or 0 if length > index
        c = m[cx].length() > index ? m[cx].at(index) : 0;
        while ( sx+1 <= dx && ( m[sx].length() > index ? m[sx].at(index) : 0) != c ) sx++;
        while ( dx-1 >= sx && ( m[dx].length() > index ? m[dx].at(index) : 0) != c ) dx--;

        assert ( (m[sx].length() > index ? m[sx].at(index) : 0) == c );
        assert ( (m[sx].length() > index ? m[sx].at(index) : 0) == c );
        
        int n_children = 0;
        if ( sx - get<0>(el) > 0 ) {
            q.push(make_tuple(get<0>(el), sx-1, index));
            n_children++;
        }
        
        if ( sx != dx || m[cx].length() > index ) {
            q.push(make_tuple(sx,dx,index+1));
            n_children++;
        }
        
        if ( get<1>(el) - dx > 0 ) {
            q.push(make_tuple(dx+1,get<1>(el), index));
            n_children++;
        }
        
        nodes.push_back( node_t(c,n_children, index) );
    }
    
    dictionary.close();
    
    return nodes;       
}

tst::tst(string filename) {
    vector<node_t> nodes = construct_tst(filename);
    bit_vector tmp_bv(2*nodes.size(), 0);
    bit_vector tmp_index_bv(nodes.size(), 0);
    bit_vector::size_type pos = 0;
    
    for (int i=0; i < nodes.size(); i++) {
        tmp_bv[pos++] = 1;
        tmp_index_bv[i] = get<2>(nodes[i])%2;
        pos += get<1>(nodes[i]);
        m_chars.push_back(get<0>(nodes[i]));        //TODO use better data structures
    }
    
    tmp_bv.resize(pos);
    m_bv = bit_vector(std::move(tmp_bv));
    m_index_bv = bit_vector(std::move(tmp_index_bv));
    util::init_support(m_bv_select1, &m_bv);
    util::init_support(m_bv_select0, &m_bv);
}

tst::~tst() {

}

void tst::print() const {
    cout << "print split characters: " << endl;
    for ( int i = 0; i < m_chars.size() ; (cout << "<" << m_chars[i++] << "> ") );
    cout << endl;
    cout << "print degree characters: " << endl;
    cout << m_index_bv << endl;
    cout << "print bit vector: " << endl;
    cout << m_bv << endl;
}

size_t tst::size_in_bytes() const {
    size_t size_bv = (size_t) sdsl::size_in_bytes<bit_vector>(m_bv);
    size_t size_chars = (size_t) sdsl::size_in_bytes<vector<char>>(m_chars);
    size_t size_index = (size_t) sdsl::size_in_bytes<bit_vector>(m_index_bv);
    size_t size_s0 = (size_t) sdsl::size_in_bytes<bit_vector::select_0_type>(m_bv_select0);
    size_t size_s1 = (size_t) sdsl::size_in_bytes<bit_vector::select_1_type>(m_bv_select1);
    
    return size_bv+size_chars+size_index+size_s0+size_s1;
}

//! Returns the root node
tst::node_type tst::root() const {
    return louds_node(0, 0);
}

tst::size_type tst::nodes() const {
    return  (m_bv.size()+1)/2;
}

bool tst::is_leaf(tst::node_type v) const {
    // node is the last leaf        or  has no children, so m_bv[v.pos]==1
    return (v.pos+1 == m_bv.size()) or m_bv[v.pos+1];
}

tst::size_type tst::degree(node_type& v) const {
    if (tst::is_leaf(v)) {  // handles boundary cases
        return 0;
    }
    // position of the next node  - node position - 1
    return m_bv_select1(v.nr+2) - v.pos - 1;
}

tst::node_type tst::child(const node_type& v, size_type i) const {
    size_type pos   = v.pos+i; // go to the position of the child's zero
    // (#bits = pos+1) - (#1-bits = v.nr+1)
    size_type zeros = pos+1 - (v.nr+1);
    return louds_node(zeros, m_bv_select1(zeros+1));
}

tst::node_type tst::parent(const node_type& v) const {
    if (v == root()) {
        return root();
    }
    size_type zero_pos   = m_bv_select0(v.nr);
    size_type parent_nr  = (zero_pos+1) - v.nr - 1;
    return node_type(parent_nr, m_bv_select1(parent_nr+1));
}

tst::size_type tst::id(const node_type& v) const {
    return v.nr;
}

int tst::next(node_type& v, char ch) {

    size_type d = degree(v);
    int index = m_index_bv[id(v)];
    char sch = m_chars[id(v)];
    
    //cout << "compare " << ch << " with " << sch << " ...(" << d << "," << index << ")" << endl << "\t";
    
    bool b = false;

    for ( int i=1; i<=d; i++ ) {
        node_type n = child(v,i);
        //cout << "%" << m_chars[id(n)] << "," << m_index_bv[id(n)] << "% ";
        
        b = m_index_bv[id(n)] == index;
        if ( b && ch < sch ) { 
            //cout << endl;
            return i;
        }
        else if ( ch > sch && !(i-1) && d > 1 )
            continue;
        
        if ( ( !b && sch == ch ) || ( b && ch > sch ) )  {
            //cout << endl;
            return i;
        }
    }
    
    return -1;
}

bool tst::lookup(string prefix) {
    prefix.push_back('\0');
    vector<louds_node> nodes;
    nodes.push_back(child(root(),1));
    
    int index, prev = m_index_bv[id(nodes.back())], curr = prev, t = 0;

    while ( prefix.at(0) && (index = next(nodes.back(),prefix.at(0))) > 0 ) {
        nodes.push_back(child(nodes.back(), index));
        curr = m_index_bv[id(nodes.back())];
        prefix.erase(0, prev != curr);
        prev = curr;
    }
    
    return index < 0 ? false : true;
}
