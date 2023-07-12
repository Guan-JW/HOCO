#pragma once
#include <map>
#include <vector>

extern "C" int sequitur(string output_path, map<string, int>& dic, vector<int> & input, 
                        int** row, int** col, int* rule_num, int doCompress=0);
