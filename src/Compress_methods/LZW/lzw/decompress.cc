#include<stdio.h>
#include<unistd.h>
#include<iostream>
#include<stdlib.h>
#include<unordered_map>
#include<string.h>
#include<vector>
using namespace std;

#define DICMAXSIZE 1000000000

int getSeqCode(unordered_map<string,int> rules, char* ruletmp, int curtmpSize ){
  string tmp(ruletmp, curtmpSize );
  if(rules.find(tmp)==rules.end()){
    return -1;
  }
  return rules[tmp];

}


int main(int argc, char** argv){

  unordered_map< int,string> rules;

  FILE *fp_dic, *fp_file ;
  fp_file = fopen("file.bin", "rb");
  fp_dic = fopen("dictionary.txt", "rb");
  int dicSize, totalSize;
  fread(&dicSize, sizeof(int), 1, fp_dic);
  fread(&totalSize, sizeof(int), 1, fp_dic);

  int ruleSize;
  int ret = fread(&ruleSize, sizeof(int), 1, fp_dic);


  vector<string> dic(dicSize);
  char wordStr[10000];
  for(int i=0; i<dicSize; i++){
    int id, size;
    fread(&id, sizeof(int), 1, fp_dic);
    fread(&size, sizeof(int), 1, fp_dic);
    fread(wordStr, sizeof(char), size, fp_dic);
    wordStr[size]='\0';
    dic[id-DICMAXSIZE]=string(wordStr, size);
  }

  int* input=(int*)malloc(sizeof(int)*totalSize);
  fread(input, sizeof(int), totalSize, fp_file );

  char prev[10000];
  char output[10000];
  int* p = (int*)input;

   int code =p[0];
  memcpy(output, (char*)input, sizeof(int));
  output[sizeof(int)]='\0';
  p=(int*)output;
  cout<<dic[*p-DICMAXSIZE];
  p=input;


  int preSize, ruleID=0, lastOutputSize=sizeof(int);

  for(int i=1; i<totalSize; i++){
    code=p[i];
    memcpy(prev, output,lastOutputSize);

    prev[lastOutputSize+sizeof(int)]='\0';
    string st(prev, lastOutputSize+sizeof(int));
    rules[ruleID++]=st;

    if(code>=DICMAXSIZE){ // if not rule
      memcpy(output, (char*)&p[i], sizeof(int));
      preSize = sizeof(int);
    }
    else{ // if is rule
      cout<<"ruleID-> "<<code<<" size: "<<rules[code].size() <<endl;
      memcpy(output, (char*)rules[code].c_str(), rules[code].size());
      preSize = rules[code].size();
      
    }

    for(int j=lastOutputSize; j<lastOutputSize+sizeof(int); j++){
      prev[j]=output[j-lastOutputSize];
    }
    string st2(prev, lastOutputSize+sizeof(int));
    rules[ruleID-1]=st2;

    if(code<DICMAXSIZE){
      memcpy(output, (char*)rules[code].c_str(), rules[code].size());
    }
   

    for(int j=0; j<preSize; j+=4){
      int* p=(int*)&output[j];
      cout<<dic[*p-DICMAXSIZE];
    }
    lastOutputSize=preSize;
  }


  return 0;
}
