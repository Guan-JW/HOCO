
#include"common.h"
#include"fileBottomUp.h"
#include<ctime>

unordered_map<ulong,string> dictionary_use;


int* countP;
struct RULE* rule_full;
bool* ruleok;
int* col;
int words, rules;

  int level1size;
int *split;
int splitNum;
int *splitLocation ;

void traversal_basic(  vector<int>*& indexVec, struct RULE*& rule_full, int & words, int & rules, int* & countP){

  int start=0,end=0,splitCur=0;
  for(int j=0; j<level1size; j++){
      int tempt=col[j];
      if(tempt==split[splitCur]&&splitCur<splitNum){
        splitLocation[splitCur]=j;
        splitCur++;
      }
  }
  for(int j=0; j<level1size; j++){
    if(col[j]>words && !ruleok[col[j]-words]){ // for not processed rules
      genLocTbl(col[j]);
    }
  }

  vector<struct WordCount> *collection=new vector<struct WordCount>[splitNum];

  for(int i=0; i<splitNum; i++){
    if(i!=0)
      start=end;
    end=splitLocation[i];
    vector<int> wordVecWord;
    vector<int> wordVecRule;

    for(int j=start; j<end; j++){
      if(countP[col[j]]==0){
        if(col[j]<words)
          wordVecWord.push_back(col[j]);
        else
          wordVecRule.push_back(col[j]);
      }
      countP[col[j]]++; // file rules and words
    }

    for(vector<int>::iterator it=wordVecRule.begin(); it!=wordVecRule.end(); it++){
      int theruleis = *it-words;
      int numberoftherule = countP[*it];
      countP[*it]=0;
      for(unordered_map<int,int>::iterator itword=rule_full[theruleis].word_index.begin(); itword!=rule_full[theruleis].word_index.end(); itword++){
        if(countP[itword->first]==0)
          wordVecWord.push_back(itword->first);
        countP[itword->first]+=numberoftherule*(itword->second); // count word
      }
    }
    for(vector<int>::iterator it=wordVecWord.begin(); it!=wordVecWord.end(); it++){
      indexVec[*it].push_back(i); // index[word] += file i;
      countP[*it]=0; 
    }
  }
}
   
int main(int argc, char** argv){

	clock_t t1, t2, t3;
    t1 = clock();
    initialization_basic(argv[1], dictionary_use, rule_full, words, rules,
                         countP);
    t2 = clock();
    int size = dictionary_use.size();
    vector<int> *indexVec = (vector<int> *)new vector<int>[size];

    traversal_basic(indexVec, rule_full, words, rules, countP);
	t3 = clock();
	cout << "IO(s): " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "Computation(s): " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl;
    cout << "Total(s): " << (double)(t3 - t1) / CLOCKS_PER_SEC << endl;
  
  return 0;
}
