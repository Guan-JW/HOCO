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
extern unsigned long** fileIdx1st ;
extern unsigned long*** filePtr ;
extern int size1st, size2nd, size1stUL, size2ndUL ;
extern vector<int> *indexVec;

struct RULE{
  vector<int> ruleIdx;
 // vector<int> ruleFeq;
  vector<int> wordIdx;
  //vector<int> wordFeq;
  int numInEdge;
  int curInEdge;
  //unordered_set<int> contain;
  //unordered_set<int> containbit;
  //int weight;
  RULE(){numInEdge=0; curInEdge=0;}
  //RULE(){numInEdge=0; curInEdge=0;weight=0;}
  vector<int> containVec;
  
  unsigned long* fileIdx1st;
  unsigned long** filePtr;
};

bool getBitFirstLyr(unsigned long* fileIdx1st, int i);

void bitMerge(unsigned long* aFileIdx2nd, unsigned long* bFileIdx2nd, int size);


bool hasThisBit(int rule, int file);


void initialization_fileAoS(char* argv, unordered_map<ulong,string> & dictionary_use, struct RULE*& rule_full, int & words, int & rules, int* & countW);
  
