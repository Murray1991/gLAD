
#pragma once

#include <algorithm>
#include <set>
#include <sdsl/bit_vectors.hpp>

using namespace sdsl;
using namespace std;

namespace glad {
    
#ifdef DEBUG
    #define D(x) (x)
    #define DEBUG_STDERR(x) (std::cerr << x)
    #define DEBUG_STDOUT(x) (std::cout << x)
#else 
    #define D(x)
    #define DEBUG_STDERR(x)
    #define DEBUG_STDOUT(x)
#endif
    
    #define EOS ((char) 3)
    
    typedef uint32_t                                    uint_t;
    typedef int                                         int_t;
    typedef std::tuple<uint_t, uint_t, char>            tTUUC;
    typedef std::pair<std::string, uint_t>              tPSU;
    typedef std::vector<tPSU>                           tVPSU;
    typedef std::vector<std::string>                  tVS;
    typedef std::array<size_t,2>                        t_range;
    
    void sort_unique(tVPSU& string_weight) {
            std::sort(string_weight.begin(), string_weight.end(), [](const tPSU& a, const tPSU& b) {
                int res = a.first.compare(b.first);
                if ( res == 0)
                    return a.second > b.second;
                return res < 0;
            });
            auto unique_end = std::unique(string_weight.begin(), string_weight.end(), [](const tPSU& a, const tPSU& b){
                return (a.first.size() == b.first.size() && a.first.compare(b.first) == 0);
            });
            string_weight.resize(unique_end-string_weight.begin());
        }
        
        void process_input(const std::string& filename, tVPSU& string_weight) {
            std::ifstream in(filename);
            if ( !in ) {
                std::cerr << "Error: Could not open file " << filename << endl;
                exit(EXIT_FAILURE);
            }
            std::string entry;
            try {
                while ( std::getline(in, entry, '\t')) {
                    std::transform(entry.begin(), entry.end(), entry.begin(), ::tolower);
                    entry += EOS;
                    std::string s_weight;
                    std::getline(in, s_weight);
                    uint_t weight = stoull(std::move(s_weight));
                    string_weight.push_back(tPSU( std::move(entry), weight ));
                }
            } catch ( std::bad_alloc& e ) {
                std::cerr << "Error: std::bad_alloc when filling from file..." << endl;
                std::exit(EXIT_FAILURE);
            }
            in.close();
        }
}
