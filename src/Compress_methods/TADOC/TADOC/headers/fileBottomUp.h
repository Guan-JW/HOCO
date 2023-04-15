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



extern int words;
 extern struct RULE* rule_full;
 extern bool* ruleok;
 extern int* countP;
 extern int* col;
 extern  int level1size;
extern int *split;
extern int splitNum;
extern int *splitLocation ;

extern double time2h5;

struct RULE{
  unordered_map<int, int> rule_index;
  unordered_map<int, int> word_index;
};

struct WordCount{
  int word;
  int count;
};
bool sortByCount(const struct WordCount &a, const struct WordCount &b);

void genLocTbl(int num);

void initialization_basic(char* argv, unordered_map<ulong,string> & dictionary_use, struct RULE*& rule_full, int & words, int & rules, int* & countP);


