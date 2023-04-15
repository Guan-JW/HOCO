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


struct RULE{
  vector<int> ruleIdx;
  vector<int> ruleFeq;
  vector<int> wordIdx;
  vector<int> wordFeq;
  //unordered_map<int, int> rule_index;
  //unordered_map<int, int> word_index;
  int numInEdge;
  int curInEdge;
  int weight;
  RULE(){numInEdge=0; curInEdge=0;weight=0;}
};

void initialization_basic(char* argv, unordered_map<ulong,string> & dictionary_use, struct RULE*& rule_full, int & words, int & rules, int* & count);


