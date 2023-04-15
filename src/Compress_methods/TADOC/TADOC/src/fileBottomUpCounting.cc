#include"fileBottomUpCounting.h"

// #define debug
extern struct RULE* rule_full;
extern int* countP;
extern bool* ruleok;
extern int words ;
extern  int* col;
extern   int level1size;
extern  int *split;
extern   int splitNum;
extern  int *splitLocation ;
extern  double time2h5;

extern string getWordDictionary(ifstream& in);

bool sortByCount(const struct WordCount &a, const struct WordCount &b){
  return a.count>b.count;
}


void genLocTbl(int num){
  int rulenum=num-words;
  vector<int> st;
  for(unordered_map<int,int>::iterator it=rule_full[rulenum].rule_index.begin(); it!=rule_full[rulenum].rule_index.end(); it++){
    if(!ruleok[it->first-words])
      genLocTbl(it->first); // dfs to rule ok
  }
	// rule with all subrules ok
  for(unordered_map<int,int>::iterator it=rule_full[rulenum].rule_index.begin(); it!=rule_full[rulenum].rule_index.end(); it++){
    int theruleis = (it->first)-words;
    int numberoftherule = it->second; // subrule and freq
    for(unordered_map<int,int>::iterator itword=rule_full[theruleis].word_index.begin(); itword!=rule_full[theruleis].word_index.end(); itword++){
      int thewordis = itword->first;
      if(countP[thewordis]==0) //  not appeared
        st.push_back(thewordis); 
      countP[thewordis]+=numberoftherule*(itword->second); // add word count 
    }
  }
  for(vector<int>::iterator p=st.begin(); p!=st.end(); p++){
    rule_full[rulenum].word_index[*p]+=countP[*p]; // add direct to the parent node
    countP[*p]=0;
  }
  ruleok[rulenum]=true; // set to true
}


void initialization_basic(char* argv, unordered_map<ulong,string> & dictionary_use, struct RULE*& rule_full, int & words, int & rules, int* & countP){
  char relationDir[100];
  sprintf(relationDir,"%s/fileYyNO.txt",argv);
  ifstream frelation(relationDir);
#ifdef debug
  if(frelation)
      cout << relationDir << endl;
  else
      cout << "WARNING!!!!!!" << endl;
#endif

  frelation >> splitNum;
  split = new int[splitNum];
  memset(split, 0, splitNum);
  splitLocation = new int[splitNum];
  memset(splitLocation, 0, splitNum);
  int tmp;
  for(int i=0; i<splitNum; i++){
    frelation>>tmp>>split[i];
  }
  frelation.close();

  char dictionaryDir[100];
  sprintf(dictionaryDir,"%s/dictionary.dic",argv);
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


  /////////////////////////////////////////////////////////////
  char inputDir[100];
  sprintf(inputDir,"%s/rowCol.dic",argv);
  ifstream finput(inputDir);
  int fileid; finput>>fileid;
  finput>>words>>rules;

  countP=new int[(words+rules)];
  memset(countP,0,sizeof(int)*(words+rules));
  rule_full=new struct RULE[rules];

  ruleok=(bool*)malloc(sizeof(bool)*rules);
  memset(ruleok, 0, sizeof(bool)*rules);




  for(int i=0; i<rules; i++){
    int ruleSize;
    finput>>ruleSize;
    if(i==0){
      level1size = ruleSize;
      col=(int*)malloc(sizeof(int)*ruleSize);
    }
    for(int j=0; j<ruleSize; j++){
      int tempt;
      finput>>tempt;
      
      if(i==0){
        col[j]=tempt; // set col to root elements
      }else{
        if(tempt<words){
          rule_full[i].word_index[tempt]++;
        }else{
          rule_full[i].rule_index[tempt]++; // add map
        }
      }
    }
    if(rule_full[i].rule_index.empty()){
      ruleok[i]=true; // no subrules
    }
  }
finput.close();
}


