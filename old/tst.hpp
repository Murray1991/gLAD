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

#ifndef TST_H
#define TST_H

#include <sdsl/bit_vectors.hpp>
#include <stdint.h>
#include <sdsl/louds_tree.hpp>
#include <sdsl/util.hpp>

#include <iostream>
#include <fstream>
#include <tuple>
#include <cassert>

// the configured options and settings for Tutorial
#define tst_VERSION_MAJOR 1// tst_VERSION_MAJOR
#define tst_VERSION_MINOR 0// tst_VERSION_MINOR

class tst
{
public:
    typedef sdsl::bit_vector                  bit_vec_t;
    typedef sdsl::louds_node                  node_type;
    typedef sdsl::bit_vector::size_type       size_type;
    typedef sdsl::bit_vector::select_1_type   select_1_t;
    typedef sdsl::bit_vector::select_0_type   select_0_t;

private:
    node_type root() const;
    
    
    bool is_leaf(tst::node_type v) const;
    size_type degree(tst::node_type& v) const;
    node_type child(const tst::node_type& v, tst::size_type i) const;
    node_type parent(const node_type& v)const;
    size_type id(const node_type& v)const;
    int next(node_type& v, char ch);
public:
    tst(std::string filename);
    ~tst();
    
    bool lookup(std::string prefix);
    size_type nodes() const;
    size_t size_in_bytes() const;
    void print() const;
private:
    bit_vec_t m_bv;
    select_0_t m_bv_select0;
    select_1_t m_bv_select1;
    
    bit_vec_t m_index_bv;
    
    std::vector<char> m_chars;       //TODO use better data structures for chars and indexes    
};

#endif // TST_H
