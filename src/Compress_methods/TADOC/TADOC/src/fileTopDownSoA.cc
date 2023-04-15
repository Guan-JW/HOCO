#include"fileTopDownSoA.h"
int size1st, size2nd, size1stUL, size2ndUL ;
vector<int> *indexVec;


extern string getWordDictionary(ifstream& in);

bool getBitFirstLyr(unsigned long* fileIdx1st, int i){
 int bitOffset = i&0x3f;
 int wordOffset = i>>6;
 return ( fileIdx1st[wordOffset] & (1ul<<bitOffset) );
}
void bitMerge(unsigned long* aFileIdx2nd, unsigned long* bFileIdx2nd, int size){
  for(int i=0; i<size; i++){
    aFileIdx2nd[i]=(aFileIdx2nd[i] | bFileIdx2nd[i]);
  }
}



bool hasThisBit(struct RULE* &rule_full, int rule, int file){
  unsigned long firstLyer = (file>>ZwiftIn2);
  unsigned long firstLyerShift6 = (firstLyer>>6);
  int ZwiftSpaceShift6 = (ZwiftSpace>>6);
  if(!(rule_full[rule].fileIdx1st[firstLyerShift6] & 1ul<<((firstLyer)&0x3f))){//none
    rule_full[rule].fileIdx1st[firstLyerShift6] = 
      rule_full[rule].fileIdx1st[firstLyerShift6]|(1ul<<((firstLyer)&0x3f));

    rule_full[rule].filePtr[firstLyer]=(unsigned long*)malloc(sizeof(unsigned long)*(ZwiftSpaceShift6));
    memset(rule_full[rule].filePtr[firstLyer],0,(sizeof(unsigned long)*(ZwiftSpaceShift6)));
    rule_full[rule].filePtr[(firstLyer)][(file&0x3ff)>>6] = rule_full[rule].filePtr[(firstLyer)][(file&0x3ff)>>6] | (1ul<<((file)&0x3ff));
    return false;
  }
  else{
    if(!(rule_full[rule].filePtr[(firstLyer)][(file&0x3ff)>>6] & (1ul<<((file)&0x3ff)))){
      rule_full[rule].filePtr[(firstLyer)][(file&0x3ff)>>6] = 
        rule_full[rule].filePtr[(firstLyer)][(file&0x3ff)>>6] | (1ul<<((file)&0x3ff));
      return false;
    }
    else  return true;
  }
}



void initialization_fileSoA(char* argv, unordered_map<ulong,string> & dictionary_use, struct RULE*& rule_full, int & words, int & rules, int* & countW){

  int splitNum;
  char relationDir[100];
  sprintf(relationDir,"%s/fileYyNO.txt",argv);
  ifstream frelation(relationDir);
  frelation>>splitNum;
  int *split = new int[splitNum];
  memset(split, 0, splitNum);
  int *splitLocation = new int[splitNum];
  memset(splitLocation, 0, splitNum);
  int tmp;
  for(int i=0; i<splitNum; i++){
    frelation>>tmp>>split[i];
  }
  frelation.close();
  size1st = ((splitNum>>ZwiftIn2)+1) ;//how many bits in 1st layer
  size2nd = ZwiftSpace ;//how many bits in 2nd layer
  size1stUL = (size1st+64)>>6;
  size2ndUL = (size2nd+sizeof(unsigned long))>>6;

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

  int size=dictionary_use.size();
  indexVec = (vector<int>*) new vector<int>[size];



  ifstream finput(inputDir);
  int fileid; finput>>fileid;
  finput>>words>>rules;
  countW=new int[(words+rules)];
  memset(countW,0,sizeof(int)*(words+rules));
  rule_full=new struct RULE[rules];


  int splitCur=0;
  int* col;
  for(int i=0; i<rules; i++){
    vector<int> st;
    int ruleSize;
    finput>>ruleSize;
    if(i==0){
      col=(int*)malloc(sizeof(int)*ruleSize);
    }
    for(int j=0; j<ruleSize; j++){
      int tempt;
      finput>>tempt;
      if(countW[tempt]==0){
        st.push_back(tempt);
        countW[tempt]=1;
      }
      if(i==0){
        col[j] = tempt;
        if( (tempt==split[splitCur]) && (splitCur<splitNum) ){
          splitLocation[splitCur++]=j;
        }
      }
    }
    for(vector<int>::iterator p=st.begin(); p!=st.end(); p++){
      countW[*p]=0;
      if(*p<words){
        rule_full[i].wordIdx.push_back(*p);
      }else{
        rule_full[i].ruleIdx.push_back(*p);
      }
    }
    
    
  }
  finput.close();

  int start=0,end=0;
  for(int i=0; i<splitNum; i++){
    vector<int> st;
    if(i!=0)
      start=end;
    end=splitLocation[i];
    for(int j=start; j<end; j++){
       int theXis = col[j];
      if( (theXis>=words)&&(countW[theXis]==0) ){
        countW[theXis]=1;
        st.push_back(theXis);
      }
    }
    for(vector<int>::iterator p=st.begin(); p!=st.end(); p++){
      countW[*p]=0;
      rule_full[(*p)-words].numInEdge++;
    }
  }



  for(int i=1; i<rules; i++){
    for(vector<int>::iterator it=rule_full[i].ruleIdx.begin(); it!=rule_full[i].ruleIdx.end(); it++){
      rule_full[(*it)-words].numInEdge++;
    }
  }
  for(int i=1; i<rules; i++){
    vector<int> st;
    queue<int> q;
    for(vector<int>::iterator it=rule_full[i].ruleIdx.begin(); it!=rule_full[i].ruleIdx.end(); it++){
      if(countW[*it]==0){
        st.push_back(*it);
        countW[*it]=1;
        q.push(*it);
      }
    }
    while( !q.empty() ){
      int head = q.front();
      q.pop();
      int rulenum = head - words;
      int tmpSubRuleSize = rule_full[rulenum].ruleIdx.size();
      for(int i2=0; i2<tmpSubRuleSize; i2++){
        int theruleis = rule_full[rulenum].ruleIdx[i2];
        if(countW[theruleis ]==0){
          st.push_back(theruleis);
          countW[theruleis]=1;
          q.push(theruleis);
        }
      }
    }
    for(vector<int>::iterator p=st.begin(); p!=st.end(); p++){
      countW[*p]=0;
      for(vector<int>::iterator it=rule_full[*p-words].ruleIdx.begin(); it!=rule_full[*p-words].ruleIdx.end(); it++){
        vector<int> tmp(rule_full[i].wordIdx.size());
        int subR = *it-words;
        vector<int>::iterator itTMP= set_difference(rule_full[i].wordIdx.begin(),rule_full[i].wordIdx.end(), rule_full[subR].wordIdx.begin(),rule_full[subR].wordIdx.end(), tmp.begin());
        tmp.resize(itTMP-tmp.begin());
        rule_full[i].wordIdx = tmp;
      }
    }
  }

  for(int i=1; i<rules; i++){
    rule_full[i].fileIdx1st = (unsigned long*)malloc(sizeof(unsigned long)*size1stUL  );
    memset(rule_full[i].fileIdx1st,0,(sizeof(unsigned long)*size1stUL));
    rule_full[i].filePtr = (unsigned long**)malloc(sizeof(unsigned long*)* size1st );
    memset(rule_full[i].filePtr,0,(sizeof(unsigned long*)*size1st));
  }
 
  start=0,end=0;
  queue<int> nodeQue;
  
  int wrong=0;
  for(int i=0; i<splitNum; i++){
    vector<int> st;
    if(i!=0)
      start=end;
    end=splitLocation[i];

    int i3ff6=(i&0x3ff)>>6;
    unsigned long i13ff=(1ul<<((i)&0x3ff));
    for(int j=start; j<end; j++){
      int theXis = col[j];
      if(countW[theXis]==0){
        countW[theXis]=1;
        st.push_back(theXis);
      }
    }
    for(vector<int>::iterator p=st.begin(); p!=st.end(); p++){
      countW[*p]=0;
      if(*p<words){
        indexVec[*p].push_back(i);
        
      }else{
        unsigned long firstLyer = (i>>ZwiftIn2);
        unsigned long firstLyerShift6 = (firstLyer>>6);
        int ZwiftSpaceShift6 = (ZwiftSpace>>6);
        rule_full[(*p)-words].fileIdx1st[firstLyerShift6 ] = rule_full[(*p)-words].fileIdx1st[firstLyerShift6] | (1ul<<((firstLyer)&0x3f));
        
        if(rule_full[(*p)-words].filePtr[firstLyer]==NULL){
          rule_full[(*p)-words].filePtr[firstLyer]=(unsigned long*)malloc(sizeof(unsigned long)*(ZwiftSpaceShift6));
          memset(rule_full[(*p)-words].filePtr[firstLyer],0,(sizeof(unsigned long)*(ZwiftSpaceShift6)));
        }
       
        rule_full[(*p)-words].filePtr[(firstLyer)][i3ff6] = rule_full[(*p)-words].filePtr[(firstLyer)][i3ff6] | i13ff;


        rule_full[(*p)-words].containVec.push_back(i);

        rule_full[(*p)-words].curInEdge++;

        if(rule_full[(*p)-words].curInEdge==rule_full[(*p)-words].numInEdge){
          nodeQue.push(*p);

        }
      }
    }
  }
  while(!nodeQue.empty()){
    int head = nodeQue.front();
    nodeQue.pop();
    int rulenum = head - words;

    int tmpSubRuleSize = rule_full[rulenum].ruleIdx.size();
    for(int i=0; i<tmpSubRuleSize; i++){
      int theruleis = rule_full[rulenum].ruleIdx[i] - words;

      //Insert word here
      for(vector<int>::iterator it=rule_full[rulenum].containVec.begin(); it!=rule_full[rulenum].containVec.end(); it++){
        if(!hasThisBit(rule_full, theruleis, (*it) )){
          rule_full[theruleis].containVec.push_back(*it);
        }
      }


      rule_full[theruleis].curInEdge++;
      
      if(rule_full[theruleis].curInEdge==rule_full[theruleis].numInEdge){
        nodeQue.push(rule_full[rulenum].ruleIdx[i]);
      }
    }
  }
  
}

