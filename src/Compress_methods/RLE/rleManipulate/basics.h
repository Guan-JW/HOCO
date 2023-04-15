#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <unordered_map>
#include <vector>

using namespace::std;

struct BlockOffset {
    ulong char_offset;
    ulong ele_offset;   // if larger than 32767 then bootstrap
    ulong hash_numerator;
    long rfid;
};

struct Ele {
    int id;
    ulong local_char_offset;
    Ele(){id=0; local_char_offset=0;}
};

void initialize(const char* fname,
                vector<vector<BlockOffset>>& block_off, 
             vector<vector<vector<Ele>>>& rule_full);