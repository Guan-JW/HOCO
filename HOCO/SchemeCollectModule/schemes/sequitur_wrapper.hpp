#ifndef SEQUITUR_WRAPPER_HPP
#define SEQUITUR_WRAPPER_HPP

#include "sequiturBasedEngine/sequitur.h"

extern "C" int sequitur(string output_path, map<string, int>& dic, vector<int> & input, 
                        int** row, int** col, int* rule_num, int doCompress);

template <typename Offset>
struct RULE {
    vector<Ele<Offset>> eleIdx;
};
#endif // SEQUITUR_WRAPPER_HPP