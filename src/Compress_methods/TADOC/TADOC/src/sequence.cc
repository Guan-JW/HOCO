#include"sequence.h"

extern int words;
extern vector<int>* tree;
extern int* treeSign;
extern struct RULE* rule_full;
extern set<int> splitSet;
extern int total;
extern int word1,word2,word3;
extern int word1fID,word2fID,word3fID;
extern int word1fLoc,word2fLoc,word3fLoc;
extern map<string,map<int,int> > storageListV2;

extern int* row;
extern vector<int> col;
extern int *split ;
 extern  int splitNum;
extern int *splitLocation ;

extern double time1h5;
extern double time1h6;
extern double time2h5;

extern string getWordDictionary(ifstream& in);

void dfsDedup(int rule, int splitNo, int father){
  int no=rule-words;
  for(vector<int>::iterator iter=tree[no].begin(); iter!=tree[no].end(); iter++){
    if((*iter>=words) &&(treeSign[*iter-words]==0)){
      dfsDedup(*iter, splitNo, rule);
    }   
  }
  int totalNumTree = tree[no].size();
  for(int iter=0; iter<totalNumTree; iter++){
    int content  = tree[no][iter];
    if((content >=words) ){
      if(tree[content -words].size()<100){

        tree[no].erase(tree[no].begin()+iter);
        tree[no].insert(tree[no].begin()+iter, tree[content-words].begin(), tree[content-words].end());
        iter+=tree[content-words].size()-1;
      }
    }
  }

  treeSign[no]=1;

}

void dfs(int rule, int splitNo, int father){
  int no=rule-words;

  int i=0;
  for(vector<int>::iterator iter=tree[no].begin(); iter!=tree[no].end(); iter++, i++){

    if((*iter)>=words){
      rule_full[(*iter)-words].fileCount[splitNo]+=1;


      dfs((*iter), splitNo, rule);
    }
    else{
      if((splitSet.find((*iter))==splitSet.end() )){
        total++;

        word1=word2;
        word2=word3;
        word3=(*iter);

        word1fID=word2fID;
        word2fID=word3fID;
        word3fID=rule;

        word1fLoc=word2fLoc;
        word2fLoc=word3fLoc;
        word3fLoc=i;



        if(total>2){
          char keyStr[100];
          sprintf(keyStr,"%d|%d|%d",word1,word2,word3);

          if( (word1fID==word2fID)&&(word2fID==word3fID)&& (word1fLoc<word2fLoc) && (word2fLoc<word3fLoc)){
            if(rule_full[no].finished==false)//zf
               rule_full[no].seqCount[string(keyStr)]++;

          }
          else{

             storageListV2[string(keyStr)][splitNo]++;
          
          }
        }
      }
    }
  }
  if(rule_full[no].finished==false)
    rule_full[no].finished=true;
}


void initialization_basic(char* argv, unordered_map<ulong,string> & dictionary_use, struct RULE*& rule_full, int & words, int & rules){

  char relationDir[100];
  sprintf(relationDir,"%s/fileYyNO.txt",argv);
  ifstream frelation(relationDir);
  frelation>>splitNum;
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
  char inputDir[100];
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
    if( tem_word == string(" ") || tem_word == string("\t") || tem_word == string("\n") ) {
      splitSet.insert(tem_num);
    }
  }
  fin.close();


  sprintf(inputDir,"%s/rowCol.dic",argv);
  ifstream finput(inputDir);
  int fileid; finput>>fileid;
  finput>>words>>rules;
  rule_full=new struct RULE[rules];
  row=(int*)malloc(sizeof(int)*(rules+1));
  row[0]=0;
  int cur=0;
  int ruleSize;

  tree=new vector<int>[rules];
  treeSign=new int[rules];
  memset(treeSign,0,sizeof(int)*rules);


  for(int i=1; i<=rules; i++){
     finput>>ruleSize;
     row[i]=ruleSize+row[i-1];
    for(int j=0; j<ruleSize; j++){
      int tmp;
      finput>>tmp;
      col.push_back(tmp);
      tree[i-1].push_back(tmp);
    }
  }

}





