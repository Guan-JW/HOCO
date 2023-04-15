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


#include<fstream>
#include <fcntl.h>    // for O_BINARY
#include <unistd.h>
#include <sys/times.h>
#include <limits.h>

using namespace std;
unordered_map<ulong,string> dictionary_use;

double timestamp ()
{
    struct timeval tv;
    gettimeofday (&tv, 0);
    return tv.tv_sec + 1e-6*tv.tv_usec;
}

struct RULE{
  vector<int> ruleIdx;
  vector<int> ruleFeq;
  vector<int> wordIdx;
  vector<int> wordFeq;
  int numInEdge;
  int curInEdge;
  int weight;
  RULE(){numInEdge=0; curInEdge=0;weight=0;}
};
int* count;
struct RULE* rule_full;
int words, rules;

string getWordDictionary(ifstream& in)
{
    int c;
  char t;
    string word;
    t = in.get();
    if( t == ' ' || t == '\t' || t == '\n' ) {
      return string(&t,1);
    }
    word += t;

    while( !in.eof() )
    {
        c = in.get();

        if( c == '\n' ) {
          break;
        }
        word += c;
    }

    return word;
}


int main(int argc, char** argv){

  char inputDir[100];
  sprintf(inputDir,"%s/rowCol.dic",argv[1]);
  
  cin>>words>>rules;
  count=new int[(words+rules)];
  memset(count,0,sizeof(int)*(words+rules));
  rule_full=new struct RULE[rules];


  for(int i=0; i<rules; i++){
    vector<int> st;
    int ruleSize;
    cin>>ruleSize;
    for(int j=0; j<ruleSize; j++){
      int tempt;
      cin>>tempt;
      if(count[tempt]==0)
        st.push_back(tempt);
      count[tempt]++;
    }
    for(vector<int>::iterator p=st.begin(); p!=st.end(); p++){
      int tmp=count[*p];
      count[*p]=0;
      if(*p<words){
        rule_full[i].wordIdx.push_back(*p);
        rule_full[i].wordFeq.push_back(tmp);
      }else{
        rule_full[i].ruleIdx.push_back(*p);
        rule_full[i].ruleFeq.push_back(tmp);
      }
    }
    for(vector<int>::iterator it=rule_full[i].ruleIdx.begin(); it!=rule_full[i].ruleIdx.end(); it++){
      rule_full[(*it)-words].numInEdge++;
    }
  }


  rule_full[0].weight=1;
  queue<int> nodeQue;
  nodeQue.push(words);
  while(!nodeQue.empty()){
    int head = nodeQue.front();
    nodeQue.pop();
    int rulenum = head - words;
    int tmpSubRuleSize = rule_full[rulenum].ruleIdx.size();
    for(int i=0; i<tmpSubRuleSize; i++){
      int theruleis = rule_full[rulenum].ruleIdx[i] - words;
      int numoftherule = rule_full[rulenum].ruleFeq[i];
      rule_full[theruleis].weight+=rule_full[rulenum].weight*numoftherule;
      rule_full[theruleis].curInEdge++;
      if(rule_full[theruleis].curInEdge==rule_full[theruleis].numInEdge){
        nodeQue.push(rule_full[rulenum].ruleIdx[i]);
      }
    }
  }

  for(int i=0; i<rules; i++){
    int tmpSubRuleSize = rule_full[i].wordIdx.size();
    for(int j=0; j<tmpSubRuleSize; j++){
        count[rule_full[i].wordIdx[j]] += rule_full[i].wordFeq[j]*rule_full[i].weight;
    }
  }

  for(int i=0; i<words; i++)
    if(count[i]!=0)
      cout<<i<<" "<<count[i]<<endl;

  return 0;
}
