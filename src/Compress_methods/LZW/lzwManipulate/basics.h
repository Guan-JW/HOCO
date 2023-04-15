#ifndef LZWANALYZE_BASICS_H
#define LZWANALYZE_BASICS_H

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

// #define RES

#define DICMAXSIZE 1000000000
#define debug

struct StringHash {
    size_t operator()(const string& S) const {
        const char* str = S.data();
        size_t hash = 0;
        while (auto ch = (size_t)*str++) {
            hash = 65599 * hash + (ch > 0x40 && ch < 0x5b ? ch | 0x60 : ch);
            // hash = (size_t)ch + (hash << 6) + (hash << 16) - hash;
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

struct TremFreq {
    int n;
    string word;
};

bool a_less_b(const TremFreq &r, const TremFreq &s);
bool a_less_b_dic(const TremFreq &r, const TremFreq &s);

struct rule {
    int p;
    int c;
};

struct BlockOffset {
    ulong char_offset;
    ulong ele_offset;   // if larger than 32767 then bootstrap
    ulong hash_numerator;
    long rfid;
};

struct Ele {
    int id;
    ulong local_char_offset;
    Ele(){id=DICMAXSIZE; local_char_offset=0;}
};

// Compresstion information
struct LzwInfo {
    int dicSize;
    int totalSize;
    int ruleSize;
    int *data;  // input
};

LzwInfo init(const char* fname, vector<string>& dic,
             vector<vector<BlockOffset>>& block_off, 
             vector<vector<vector<Ele>>>& rule_full);

struct VectorHasher { 
    int operator()(const vector<int> &V) const { // hash(vector) = vector[0]
        unsigned int hash = 0;
        //     for(int i=0;i<V.size();i++) {
        // //      hash = (hash<<1+V[i])%19260817;
        //       //hash = ((hash<<1)+V[i])%147483647;
        //       //hash = (unsigned long
        //       long)(hash*131+V[i])%212370440130137957ll;
        //       //hash = (hash*131+V[i])%19260817;
        //       hash = hash + V[i]; // Can be anything
        //     }
        return V[0];
    }
};

void get_dic_rev(vector<string>& dic, map<string,int> &dic_rev);
void get_rules_rev(map<vector<int>, int>& rules_rev);
#endif //LZWANALYZE_BASICS_H
