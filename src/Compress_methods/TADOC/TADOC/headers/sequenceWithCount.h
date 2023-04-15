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


extern int words, rules;
extern  vector<int>* tree;
extern  int* treeSign;
extern  struct RULE* rule_full;
extern  set<int> splitSet;
extern  int total;
extern  int word1,word2,word3;
extern  int word1fID,word2fID,word3fID;
extern  int word1fLoc,word2fLoc,word3fLoc;
extern  map<string,int> *seqCount;
extern  int splitNum;
extern  int *split ;
extern  int *splitLocation ;
extern  int* row;
extern  vector<int> col;
extern  double time2_1;
extern  double time1h5;
extern  double time1h6;
extern  double time2h5;

struct RULE{
  unordered_map<int, int> rule_index;
  unordered_map<int, int> word_index;
  unordered_map<string,int> seqCount;
  unordered_map<int,int> fileCount;
  bool finished;
  RULE(){finished=false;}
};

void dfsDedup(int rule, int splitNo, int father);


void dfs(int rule, int splitNo, int father);
  

void initialization_basic(char* argv, unordered_map<ulong,string> & dictionary_use, struct RULE*& rule_full, int & words, int & rules);


