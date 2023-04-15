#include"basic.h"

extern string getWordDictionary(ifstream& in);
void initialization_basic(char* argv, unordered_map<ulong,string> & dictionary_use, struct RULE*& rule_full, int & words, int & rules, int* & count){
  char dictionaryDir[100];
  char inputDir[100];
  sprintf(dictionaryDir,"%s/dictionary.dic",argv);
  sprintf(inputDir,"%s/rowCol.dic",argv);
  ifstream fin(dictionaryDir);
  ulong tem_num;
  string tem_word;
  while(!fin.eof()){
    tem_num=-1;
    fin>>tem_num;

    fin.get();
    tem_word=getWordDictionary(fin);
    dictionary_use[tem_num]=tem_word;
  }
  fin.close();
  ifstream finput(inputDir);
  int fileid; finput>>fileid;
  finput>>words>>rules;
  count=new int[(words+rules)];
  memset(count,0,sizeof(int)*(words+rules));
  rule_full=new struct RULE[rules];

  for(int i=0; i<rules; i++){
    vector<int> st; // rule elements
    int ruleSize;
    finput>>ruleSize;
    for(int j=0; j<ruleSize; j++){
      int tempt;
      finput>>tempt;
      if(count[tempt]==0)
        st.push_back(tempt);
      count[tempt]++;
    }
    for(vector<int>::iterator p=st.begin(); p!=st.end(); p++){
      int tmp=count[*p];
      count[*p]=0; // return 2 0
      if(*p<words){
        rule_full[i].wordIdx.push_back(*p);  // word index
        rule_full[i].wordFeq.push_back(tmp); // word freq
      }else{
        rule_full[i].ruleIdx.push_back(*p);
        rule_full[i].ruleFeq.push_back(tmp);
      }
    }
    for(vector<int>::iterator it=rule_full[i].ruleIdx.begin(); it!=rule_full[i].ruleIdx.end(); it++){
      rule_full[(*it)-words].numInEdge++; // freq for pointed to 
    }
  }
  finput.close();

}




