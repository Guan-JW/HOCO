#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<cstring>
#include<vector>
#include<map>
#include<set>
#include<unordered_map>
#include<string>
#include<list>
#include<sys/time.h>
#include<queue>
#include<algorithm>

#include<unordered_set>

#include<fstream>
#include <fcntl.h>    // for O_BINARY
#include <unistd.h>
#include <sys/times.h>
#include <limits.h>

using namespace std;



#define ZwiftSpace 1024
#define ZwiftIn2 10
extern int size1st, size2nd, size1stUL, size2ndUL ;
extern vector<int> *indexVec;

struct RULE{
  vector<int> ruleIdx;
  vector<int> wordIdx;
  int numInEdge;
  int curInEdge;
  RULE(){numInEdge=0; curInEdge=0;}
  vector<int> containVec;
  
  unsigned long* fileIdx1st;
  unsigned long** filePtr;
};

bool getBitFirstLyr(unsigned long* fileIdx1st, int i);

void bitMerge(unsigned long* aFileIdx2nd, unsigned long* bFileIdx2nd, int size);

bool hasThisBit(struct RULE* &rule_full, int rule, int file);



void initialization_fileSoA(char* argv, unordered_map<ulong,string> & dictionary_use, struct RULE*& rule_full, int & words, int & rules, int* & countW);


