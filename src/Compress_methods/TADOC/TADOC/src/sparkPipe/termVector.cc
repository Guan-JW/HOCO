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

int splitNumSepa[NUMTHREADS];
int separate4[NUMTHREADS];//ending in each file, format yY'XXX' the XXX
int *split;//symbols for each separate yy'XXX'
struct WordCount{
    int word;
      int count;
};
vector<struct WordCount> *collection;



vector<struct WordCount> *collection1;
vector<struct WordCount> *collection2;
vector<struct WordCount> *collection3;
vector<struct WordCount> *collection4;

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
int* row[NUMTHREADS];
int* col;
int words, rules;

void genLocTbl(int num){
  int rulenum=num-words;
  vector<int> st;
  for(unordered_map<int,int>::iterator it=rule_full[rulenum].rule_index.begin(); it!=rule_full[rulenum].rule_index.end(); it++){
    if(!ruleok[it->first-words]){
      genLocTbl(it->first );
     }
  }

  for(unordered_map<int,int>::iterator it=rule_full[rulenum].rule_index.begin(); it!=rule_full[rulenum].rule_index.end(); it++){
    int theruleis = (it->first)-words;
    int numberoftherule = it->second;
    for(unordered_map<int,int>::iterator itword=rule_full[theruleis].word_index.begin(); itword!=rule_full[theruleis].word_index.end(); itword++){
      int thewordis = itword->first;
    
      if(count4[thewordis]==0)
        st.push_back(thewordis);
      count4[thewordis]+=numberoftherule*(itword->second);
    }
  }
    
  for(vector<int>::iterator p=st.begin(); p!=st.end(); p++){
    rule_full[rulenum].word_index[*p]+=count4[*p];
    count4[*p]=0;
  }
  ruleok[rulenum]=true;
}



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




  char inputDir[100];
  cin>>words>>rules;

  collection=new vector<struct WordCount>[splitNumSepa[threadID]];


  count4=new int[(words+rules)];
  rule_full=new struct RULE[rules];
  int tempt;

  ruleok=(bool*)malloc(sizeof(bool)*rules);
  memset(ruleok, 0, sizeof(bool)*rules);


  int level1size;
  for(int i=0; i<rules; i++){
    int ruleSize;
    cin>>ruleSize;
    if(i==0){
      level1size = ruleSize;
      col=(int*)malloc(sizeof(int)*ruleSize);
    }
    for(int j=0; j<ruleSize; j++){
      int tempt;
      cin>>tempt;
      if(i==0){
        col[j]=tempt;
      }else{
        if(tempt<words){
          rule_full[i].word_index[tempt]++;
        }else{
          rule_full[i].rule_index[tempt]++;
        }
      }
    }
    if(rule_full[i].rule_index.empty()){
      ruleok[i]=true;
    }
  }

  int offset=1, splitSymCur=0;
  if(threadID!=0){
    offset = separate4[threadID-1];
    splitSymCur=offset-1;
  }
  int start=0,end=0,splitCur=0 ;
  int *splitLocation = new int[splitNumSepa[threadID]];

  for(int j=0; j<level1size ; j++){
      int tempt=col[j];
      if(tempt==split[splitSymCur]&&splitCur<splitNumSepa[threadID]){
        splitLocation[splitCur]=j;
        splitCur++;
        splitSymCur++;
      }
  }

      
  for(int j=0; j<level1size ; j++){
    if(col[j]>words && !ruleok[col[j]-words]){
      genLocTbl(col[j]);
    }
  }

  
  if(threadID!=0)
    collection[0].push_back( (struct WordCount){split[offset-2], 1} );


  for(int i=0; i<splitNumSepa[threadID]; i++){
    if(i!=0)
      start=end;
    end=splitLocation[i];
    vector<int> wordVecWord;
    vector<int> wordVecRule;

    for(int j=start; j<end; j++){
      if(count4[col[j]] == 0){
        if(col[j]<words)
          wordVecWord.push_back(col[j]);
        else
          wordVecRule.push_back(col[j]);
      }
      count4[col[j]] ++;
    }
    for(vector<int>::iterator it=wordVecRule.begin(); it!=wordVecRule.end(); it++){
      int theruleis = *it-words;
      int numberoftherule = count4[*it];
      count4[*it]=0;
      for(unordered_map<int,int>::iterator itword=rule_full[theruleis].word_index.begin(); itword!=rule_full[theruleis].word_index.end(); itword++){
        if(count4[itword->first]==0)
          wordVecWord.push_back(itword->first);
        count4[itword->first]+=numberoftherule*(itword->second);
      }
    }
    for(vector<int>::iterator it=wordVecWord.begin(); it!=wordVecWord.end(); it++){
      collection[i].push_back((struct WordCount){*it, count4[*it]});
      count4[*it]=0;
    }
    sort(collection[i].begin(),collection[i].end(),sortByCount);
  }





















   for(int i=0; i<splitNumSepa[threadID]; i++){
     for(vector<struct WordCount>::iterator it=collection[i].begin(); it!=collection[i].end(); it++){
      if(threadID != 0)
	      cout<<i-1+separate4[threadID-1]<<" "<<it->word<<" "<<it->count<<endl;
      else
        cout<<i<<" "<<it->word<<" "<<it->count<<endl;
     }
   }



    return 0;
}



