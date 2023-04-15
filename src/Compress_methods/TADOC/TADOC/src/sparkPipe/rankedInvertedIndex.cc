#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<cstring>
#include<vector>
#include<map>
#include<set>
#include<unordered_set>
#include<unordered_map>
#include<string>
#include<list>
#include<sys/time.h>

#include<pthread.h>
#include<omp.h>
#include<algorithm>

#include<fstream>
#include <fcntl.h>    // for O_BINARY
#include <unistd.h>
#include <sys/times.h>
#include <limits.h>

int threadID;

#define NUMTHREADS 6

using namespace std;

map<string,map<int,int> > storageListV2;

int words, rules;
int total, word1, word2, word3;
vector<int> col;
set<int> splitSet;
int* row;
map<string,int> *seqCount;

int splitNumSepa[NUMTHREADS];
int separate4[NUMTHREADS];//ending in each file, format yY'XXX' the XXX
int *split;//symbols for each separate yy'XXX'
struct WordCount{
    int word;
      int count;
};



void dfs(map<string,int> *seqCount, int rule, int splitNo ){
  if((rule< words)){
    if((splitSet.find(rule)==splitSet.end() )){ 
      total++;
      word1=word2;
      word2=word3;
      word3=rule;
      if(total>2){
        char keyStr[100];
        sprintf(keyStr,"%d|%d|%d",word1,word2,word3);
                string key = string(keyStr);
            if(threadID!=0){
              storageListV2[key][splitNo-1+separate4[threadID-1]]++;
            }else{
              storageListV2[key][splitNo]++;
            }
      }
    }     
  }else{
    int no=rule-words;
    for(int i=row[no]; i<row[no+1]; i++){
      dfs(seqCount, col[i], splitNo );
    }     
  }
}






bool sortByCount(const struct WordCount &a, const struct WordCount &b){
  return a.count>b.count;
}


unordered_map<ulong,string> dictionary_use;
char* argvDIR;
  char dictionaryDir[100];
double timestamp ()
{
    struct timeval tv;
    gettimeofday (&tv, 0);
    return tv.tv_sec + 1e-6*tv.tv_usec;
}

struct RULE{
  unordered_map<int, int> rule_index;
  unordered_map<int, int> word_index;
};
int* count4;
struct RULE* rule_full;
bool* ruleok;

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
cin>>threadID;
splitSet.insert(1);
splitSet.insert(3);
splitSet.insert(90193);

  int splitNum;


  cin>>splitNum;
  split = new int[splitNum];
  memset(split, 0, splitNum);

  int tmp;
  for(int i=0; i<splitNum; i++){
    cin>>tmp>>split[i];
  }


  for(int i=0; i<NUMTHREADS; i++)
    cin>>separate4[i];
 

  splitNumSepa[0]=separate4[0]-1;
  splitNumSepa[1]=separate4[1]-separate4[0];
  splitNumSepa[2]=separate4[2]-separate4[1];
  splitNumSepa[3]=separate4[3]-separate4[2];
  splitNumSepa[4]=separate4[4]-separate4[3];
  splitNumSepa[5]=separate4[5]-separate4[4];


  int ret;


seqCount=new map<string,int>[splitNumSepa[threadID]];

  char inputDir[100];
  cin>>words>>rules;
  row=(int*)malloc(sizeof(int)*(rules+1));
  row[0]=0;
  int cur=0, ruleSize;
  for(int i=1; i<=rules; i++){
    cin>>ruleSize;
    row[i] = ruleSize+row[i-1];
    for(int j=0; j<ruleSize; j++){
      int tmp;
      cin>>tmp;
      col.push_back(tmp);
    }
  }



  int offset=1, splitSymCur=0;
  if(threadID!=0){
    offset = separate4[threadID-1];
    splitSymCur=offset-1;
  }
  int start=0,end=0,splitCur=0 ;
  int *splitLocation = new int[splitNumSepa[threadID]];

  for(int j=row[0]; j<row[1]; j++){
      int tempt=col[j];
      if(tempt==split[splitSymCur]&&splitCur<splitNumSepa[threadID]){
        splitLocation[splitCur]=j;
        splitCur++;
        splitSymCur++;
      }
  }

  
  if(threadID != 0){
    total = 1;
    word3 = split[offset-2];
  }
  else{
    total = 0;
  }
  for(int i=0; i<splitNumSepa[threadID]; i++){
    if(i!=0){
      total=0;
      start=end;
    }
    end=splitLocation[i];
    for(int j=start; j<end; j++){
      int word=col[j];
      if(word < words){
        if(splitSet.find(word)==splitSet.end() ){
          total++;
          word1=word2;
          word2=word3;
          word3=word;
          if(total>2){
            char keyStr[100];
            sprintf(keyStr,"%d|%d|%d",word1,word2,word3);
            string key = string(keyStr);
            if(threadID!=0){
              storageListV2[key][i-1+separate4[threadID-1]]++;
            }else{
              storageListV2[key][i]++;
            }
          }
        }
      }else{
        dfs(seqCount,word,i);
      }
    }
  }

  

  for(map<string, map<int,int> >::iterator it=storageListV2.begin(); it!=storageListV2.end(); it++){
    for(map<int,int>::iterator i=(it->second).begin(); i!=(it->second).end(); i++){
        cout<<it->first<<" "<<i->first<<" "<<i->second<<endl;
    }
  }


    return 0;
}



