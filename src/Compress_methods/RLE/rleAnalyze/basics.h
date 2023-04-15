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

struct Ele {
    int id;
    int length;
    Ele(){id=0; length=0;}
};

struct TremFreq {
    int n;
    string word;
};


struct StringHash {
    size_t operator()(const string& S) const {
        const char* str = S.data();
        size_t hash = 0;
        while (auto ch = (size_t)*str++) {
            hash = 65599 * hash + (ch > 0x40 && ch < 0x5b ? ch | 0x60 : ch);
        }
        return hash;
    }
};

struct EqualInLower {
    bool operator()(const string& S1, const string& S2) const {
        if (S1.size() != S2.size()) return false;
        size_t n = S1.size();
        for (size_t i = 0; i < n; i++)
            if (tolower(S1[i]) != tolower(S2[i])) return false;
        return true;
    }
};

void initialize(const char* fname, vector<string>& dic);
bool a_less_b(const TremFreq &r, const TremFreq &s);