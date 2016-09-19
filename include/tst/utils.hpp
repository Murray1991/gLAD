
#pragma once

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
    typedef tPSU *                                      tPSU2;
    typedef std::vector<tPSU2>                          tVPSU2;
    typedef std::vector<std::string *>                  tVS;
    typedef std::array<size_t,2>                        t_range;
    
    void sort_unique(tVPSU2& string_weight) {
            std::sort(string_weight.begin(), string_weight.end(), [](const tPSU2& a, const tPSU2& b) {
                return a->first.compare(b->first) < 0;
            });
            auto unique_end = std::unique(string_weight.begin(), string_weight.end(), [](tPSU2& first, tPSU2& last) {
                //last.first => mind blown!
                if ( first->first.size() != last->first.size() )
                    return false;
                if ( first->first.compare(last->first) == 0 ) {
                    //keep the higher weight, is it really necessary? maybe better a sum?
                    first->second = first->second > last->second ? first->second : last->second; 
                    last->second = first->second > last->second ? first->second : last->second;
                    return true;
                }
                return false;
            });
            // Sorry...
            //for ( tVPSU2::iterator it = unique_end; it != string_weight.end(); it++ )
            //    delete *it;
            string_weight.resize(unique_end - string_weight.begin());
            string_weight.shrink_to_fit();
        }
        
        void process_input(const std::string& filename, tVPSU2& string_weight) {
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
                    uint_t weight = stoull(std::move(s_weight));
                    string_weight.push_back(new tPSU( std::move(entry) + EOS, weight ));
                }
            } catch ( std::bad_alloc& e ) {
                std::cerr << "Error: std::bad_alloc when filling from file..." << endl;
                std::exit(EXIT_FAILURE);
            }
            in.close();
        }
    
    //TODO reimplement something like std::unique and use two parallel vectors for strings and weights

}
